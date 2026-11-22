#include "SolverRunner.h"

#include "AppPaths.h"
#include "SolutionCodec.h"

#include <algorithm>
#include <cctype>
#include <sstream>
#include <stdexcept>

#if defined(_WIN32)
#define NOMINMAX
#include <Windows.h>
#else
#include <csignal>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>
#endif

namespace
{
	std::string TrimLine(const std::string& value)
	{
		const std::size_t first = value.find_first_not_of(" \t\r\n");
		if (first == std::string::npos)
		{
			return "";
		}
		const std::size_t last = value.find_last_not_of(" \t\r\n");
		return value.substr(first, last - first + 1);
	}

	bool IsPureLurdLine(const std::string& line)
	{
		const std::string trimmed = TrimLine(line);
		if (trimmed.empty())
		{
			return false;
		}
		bool hasMove = false;
		for (char c : trimmed)
		{
			if (c == ' ' || c == '\t')
			{
				continue;
			}
			const char lower = static_cast<char>(std::tolower(static_cast<unsigned char>(c)));
			if (lower == 'r' || lower == 'l' || lower == 'u' || lower == 'd')
			{
				hasMove = true;
				continue;
			}
			return false;
		}
		return hasMove;
	}

	std::string ExtractFromSingleText(const std::string& text)
	{
		std::istringstream iss(text);
		std::string line;
		bool inSolutionSection = false;
		std::string collected;

		while (std::getline(iss, line))
		{
			const std::string trimmed = TrimLine(line);
			std::string lower = trimmed;
			std::transform(lower.begin(), lower.end(), lower.begin(), [](unsigned char c) {
				return static_cast<char>(std::tolower(c));
			});

			if (lower == "solution" || lower == "solution:")
			{
				inSolutionSection = true;
				collected.clear();
				continue;
			}

			if (inSolutionSection)
			{
				if (trimmed.empty())
				{
					if (!collected.empty())
					{
						return collected;
					}
					continue;
				}
				if (IsPureLurdLine(trimmed))
				{
					collected += trimmed;
					continue;
				}
				if (!collected.empty())
				{
					return collected;
				}
				inSolutionSection = false;
			}
		}

		return collected;
	}

#if defined(_WIN32)
	std::wstring QuoteWindowsArg(const std::wstring& arg)
	{
		if (arg.empty())
		{
			return L"\"\"";
		}
		if (arg.find_first_of(L" \t\"") == std::wstring::npos)
		{
			return arg;
		}
		std::wstring result = L"\"";
		unsigned int backslashes = 0;
		for (wchar_t c : arg)
		{
			if (c == L'\\')
			{
				++backslashes;
			}
			else if (c == L'"')
			{
				result.append(backslashes * 2 + 1, L'\\');
				result.push_back(L'"');
				backslashes = 0;
			}
			else
			{
				result.append(backslashes, L'\\');
				backslashes = 0;
				result.push_back(c);
			}
		}
		result.append(backslashes * 2, L'\\');
		result.push_back(L'"');
		return result;
	}
#endif
}

SolverRunner::SolverRunner()
	: mState(SolverState::Idle)
	, mCancelRequested(false)
	, mResult{}
	, mActiveProcessHandle(0)
{
	mResult.mState = SolverState::Idle;
	mResult.mMoveCount = 0;
	mResult.mPushCount = 0;
	mResult.mElapsedSeconds = 0.0;
}

SolverRunner::~SolverRunner()
{
	Cancel();
}

bool SolverRunner::Start(const SolverRequest& request)
{
	if (mState.load() == SolverState::Running)
	{
		return false;
	}
	if (mWorkerThread.joinable())
	{
		mWorkerThread.join();
	}

	mCancelRequested.store(false);
	mStartTime = std::chrono::steady_clock::now();
	{
		std::lock_guard<std::mutex> lock(mResultMutex);
		mResult = SolverResult{};
		mResult.mState = SolverState::Running;
	}
	mState.store(SolverState::Running);

	mWorkerThread = std::thread(&SolverRunner::RunWorker, this, request);
	return true;
}

void SolverRunner::Cancel()
{
	mCancelRequested.store(true);
	TerminateActiveProcess();
	if (mWorkerThread.joinable())
	{
		mWorkerThread.join();
	}
}

double SolverRunner::GetElapsedSeconds() const
{
	if (mState.load() == SolverState::Running)
	{
		const auto now = std::chrono::steady_clock::now();
		return std::chrono::duration<double>(now - mStartTime).count();
	}
	std::lock_guard<std::mutex> lock(mResultMutex);
	return mResult.mElapsedSeconds;
}

SolverRunner::SolverResult SolverRunner::GetResult() const
{
	std::lock_guard<std::mutex> lock(mResultMutex);
	return mResult;
}

std::string SolverRunner::ExtractLurdFromFestivalOutput(const std::string& outFileText, const std::string& stdOutText)
{
	std::string lurd = ExtractFromSingleText(outFileText);
	if (!lurd.empty())
	{
		return lurd;
	}
	return ExtractFromSingleText(stdOutText);
}

void SolverRunner::TerminateActiveProcess()
{
	std::lock_guard<std::mutex> lock(mProcessMutex);
	if (mActiveProcessHandle == 0)
	{
		return;
	}
#if defined(_WIN32)
	HANDLE procHandle = reinterpret_cast<HANDLE>(mActiveProcessHandle);
	TerminateProcess(procHandle, 1);
#else
	pid_t pid = static_cast<pid_t>(mActiveProcessHandle);
	if (pid > 0)
	{
		kill(pid, SIGTERM);
	}
#endif
}

void SolverRunner::RunWorker(SolverRequest request)
{
	namespace fs = std::filesystem;

	SolverResult result{};
	result.mState = SolverState::Failed;

	// 一時作業フォルダはFestivalのfopen互換のためASCII名で作成する
	const auto uniqueId = std::chrono::steady_clock::now().time_since_epoch().count();
	const fs::path workDir = fs::temp_directory_path() / ("pandae_festival_" + std::to_string(uniqueId));
	const fs::path inputFile = workDir / "level.xsb";
	const fs::path outputFile = workDir / "solution.txt";

	auto cleanup = [&]() {
		std::error_code ec;
		fs::remove_all(workDir, ec);
	};

	try
	{
		if (request.mExecutablePath.empty() || !fs::is_regular_file(request.mExecutablePath))
		{
			throw std::runtime_error("Festival executable not found");
		}
		if (request.mStartBoard.empty())
		{
			throw std::runtime_error("Start board is empty");
		}

		fs::create_directories(workDir);
		std::string boardText;
		for (const auto& row : request.mStartBoard)
		{
			boardText += row + "\n";
		}
		AppPaths::WriteFile(inputFile, boardText);

		const int timeLimit = std::max(5, request.mTimeLimitSeconds);
		int cores = request.mCores;
		if (cores != 1 && cores != 2 && cores != 4 && cores != 8)
		{
			cores = 1;
		}
		const int hardTimeoutSec = timeLimit + 5;
		bool timedOutByHost = false;
		std::string capturedStdOut;

#if defined(_WIN32)
		SECURITY_ATTRIBUTES sa{};
		sa.nLength = sizeof(sa);
		sa.bInheritHandle = TRUE;

		HANDLE readPipe = nullptr;
		HANDLE writePipe = nullptr;
		if (!CreatePipe(&readPipe, &writePipe, &sa, 0))
		{
			throw std::runtime_error("Failed to create pipe for solver process");
		}
		SetHandleInformation(readPipe, HANDLE_FLAG_INHERIT, 0);

		STARTUPINFOW si{};
		si.cb = sizeof(si);
		si.dwFlags = STARTF_USESTDHANDLES;
		si.hStdOutput = writePipe;
		si.hStdError = writePipe;
		si.hStdInput = nullptr;

		PROCESS_INFORMATION pi{};
		const std::wstring cmdLine =
			QuoteWindowsArg(request.mExecutablePath.wstring())
			+ L" " + QuoteWindowsArg(inputFile.wstring())
			+ L" -time " + std::to_wstring(timeLimit)
			+ L" -cores " + std::to_wstring(cores)
			+ L" -out_file " + QuoteWindowsArg(outputFile.wstring());

		std::vector<wchar_t> cmdBuffer(cmdLine.begin(), cmdLine.end());
		cmdBuffer.push_back(L'\0');

		const BOOL created = CreateProcessW(
			request.mExecutablePath.c_str(),
			cmdBuffer.data(),
			nullptr,
			nullptr,
			TRUE,
			CREATE_NO_WINDOW,
			nullptr,
			workDir.c_str(),
			&si,
			&pi);

		CloseHandle(writePipe);

		if (!created)
		{
			CloseHandle(readPipe);
			throw std::runtime_error("Failed to launch Festival executable");
		}

		{
			std::lock_guard<std::mutex> lock(mProcessMutex);
			mActiveProcessHandle = reinterpret_cast<std::uintptr_t>(pi.hProcess);
		}

		// 子プロセス終了を監視しつつ標準出力を非ブロッキングで回収
		char buf[1024];
		for (;;)
		{
			DWORD available = 0;
			while (PeekNamedPipe(readPipe, nullptr, 0, nullptr, &available, nullptr) && available > 0)
			{
				DWORD bytesRead = 0;
				const DWORD toRead = std::min<DWORD>(available, static_cast<DWORD>(sizeof(buf)));
				if (ReadFile(readPipe, buf, toRead, &bytesRead, nullptr) && bytesRead > 0)
				{
					if (capturedStdOut.size() < 65536)
					{
						capturedStdOut.append(buf, bytesRead);
					}
				}
				else
				{
					break;
				}
			}

			const DWORD waitRes = WaitForSingleObject(pi.hProcess, 50);
			if (waitRes == WAIT_OBJECT_0)
			{
				break;
			}

			const double elapsed = std::chrono::duration<double>(std::chrono::steady_clock::now() - mStartTime).count();
			if (mCancelRequested.load())
			{
				TerminateProcess(pi.hProcess, 1);
				WaitForSingleObject(pi.hProcess, 1000);
				break;
			}
			if (elapsed > static_cast<double>(hardTimeoutSec))
			{
				timedOutByHost = true;
				TerminateProcess(pi.hProcess, 1);
				WaitForSingleObject(pi.hProcess, 1000);
				break;
			}
		}

		// 残りのパイプ出力を回収
		DWORD available = 0;
		while (PeekNamedPipe(readPipe, nullptr, 0, nullptr, &available, nullptr) && available > 0)
		{
			DWORD bytesRead = 0;
			const DWORD toRead = std::min<DWORD>(available, static_cast<DWORD>(sizeof(buf)));
			if (ReadFile(readPipe, buf, toRead, &bytesRead, nullptr) && bytesRead > 0)
			{
				if (capturedStdOut.size() < 65536)
				{
					capturedStdOut.append(buf, bytesRead);
				}
			}
			else
			{
				break;
			}
		}

		{
			std::lock_guard<std::mutex> lock(mProcessMutex);
			mActiveProcessHandle = 0;
		}
		CloseHandle(readPipe);
		CloseHandle(pi.hThread);
		CloseHandle(pi.hProcess);
#else
		int pipefd[2];
		if (pipe(pipefd) != 0)
		{
			throw std::runtime_error("Failed to create pipe for solver process");
		}

		const pid_t pid = fork();
		if (pid < 0)
		{
			close(pipefd[0]);
			close(pipefd[1]);
			throw std::runtime_error("Failed to fork solver process");
		}

		if (pid == 0)
		{
			// 子プロセス
			close(pipefd[0]);
			dup2(pipefd[1], STDOUT_FILENO);
			dup2(pipefd[1], STDERR_FILENO);
			close(pipefd[1]);

			if (chdir(workDir.c_str()) != 0)
			{
				_exit(127);
			}

			const std::string exeStr = request.mExecutablePath.string();
			const std::string inStr = inputFile.string();
			const std::string timeStr = std::to_string(timeLimit);
			const std::string coresStr = std::to_string(cores);
			const std::string outStr = outputFile.string();

			char* const argv[] = {
				const_cast<char*>(exeStr.c_str()),
				const_cast<char*>(inStr.c_str()),
				const_cast<char*>("-time"),
				const_cast<char*>(timeStr.c_str()),
				const_cast<char*>("-cores"),
				const_cast<char*>(coresStr.c_str()),
				const_cast<char*>("-out_file"),
				const_cast<char*>(outStr.c_str()),
				nullptr
			};
			execvp(exeStr.c_str(), argv);
			_exit(127);
		}

		// 親プロセス
		close(pipefd[1]);
		fcntl(pipefd[0], F_SETFL, O_NONBLOCK);
		{
			std::lock_guard<std::mutex> lock(mProcessMutex);
			mActiveProcessHandle = static_cast<std::uintptr_t>(pid);
		}

		char buf[1024];
		int status = 0;
		for (;;)
		{
			ssize_t n = 0;
			while ((n = read(pipefd[0], buf, sizeof(buf))) > 0)
			{
				if (capturedStdOut.size() < 65536)
				{
					capturedStdOut.append(buf, static_cast<std::size_t>(n));
				}
			}

			const pid_t w = waitpid(pid, &status, WNOHANG);
			if (w == pid)
			{
				break;
			}

			const double elapsed = std::chrono::duration<double>(std::chrono::steady_clock::now() - mStartTime).count();
			if (mCancelRequested.load())
			{
				kill(pid, SIGTERM);
				std::this_thread::sleep_for(std::chrono::milliseconds(150));
				if (waitpid(pid, &status, WNOHANG) == 0)
				{
					kill(pid, SIGKILL);
					waitpid(pid, &status, 0);
				}
				break;
			}
			if (elapsed > static_cast<double>(hardTimeoutSec))
			{
				timedOutByHost = true;
				kill(pid, SIGTERM);
				std::this_thread::sleep_for(std::chrono::milliseconds(150));
				if (waitpid(pid, &status, WNOHANG) == 0)
				{
					kill(pid, SIGKILL);
					waitpid(pid, &status, 0);
				}
				break;
			}

			std::this_thread::sleep_for(std::chrono::milliseconds(50));
		}

		ssize_t n = 0;
		while ((n = read(pipefd[0], buf, sizeof(buf))) > 0)
		{
			if (capturedStdOut.size() < 65536)
			{
				capturedStdOut.append(buf, static_cast<std::size_t>(n));
			}
		}

		{
			std::lock_guard<std::mutex> lock(mProcessMutex);
			mActiveProcessHandle = 0;
		}
		close(pipefd[0]);
#endif

		result.mElapsedSeconds = std::chrono::duration<double>(std::chrono::steady_clock::now() - mStartTime).count();
		result.mRawOutput = capturedStdOut;

		if (mCancelRequested.load())
		{
			result.mState = SolverState::Cancelled;
		}
		else if (timedOutByHost)
		{
			result.mState = SolverState::TimedOut;
			result.mErrorMessage = "Solver time limit exceeded";
		}
		else
		{
			std::string outFileText;
			if (fs::is_regular_file(outputFile))
			{
				const auto bytes = AppPaths::ReadFile(outputFile);
				outFileText.assign(bytes.begin(), bytes.end());
			}

			const std::string rawLurd = ExtractLurdFromFestivalOutput(outFileText, capturedStdOut);
			if (rawLurd.empty())
			{
				if (result.mElapsedSeconds >= static_cast<double>(timeLimit - 1))
				{
					result.mState = SolverState::TimedOut;
					result.mErrorMessage = "No solution found within time limit";
				}
				else
				{
					result.mState = SolverState::Failed;
					result.mErrorMessage = "No solution found for this board";
				}
			}
			else
			{
				// SolutionCodec で盤面に対する正当性を二重検証
				const auto sim = SolutionCodec::SimulateLurd(request.mStartBoard, rawLurd, false);
				if (!sim.mIsValid || !sim.mReachesComplete)
				{
					result.mState = SolverState::Failed;
					result.mErrorMessage = "Solver returned an invalid solution: " + sim.mErrorMessage;
				}
				else
				{
					result.mState = SolverState::Succeeded;
					result.mLurdSolution = rawLurd;
					result.mNormalizedLurd = sim.mNormalizedLurd;
					result.mMoveCount = sim.mMoveCount;
					result.mPushCount = sim.mPushCount;
				}
			}
		}
	}
	catch (const std::exception& error)
	{
		result.mElapsedSeconds = std::chrono::duration<double>(std::chrono::steady_clock::now() - mStartTime).count();
		result.mState = mCancelRequested.load() ? SolverState::Cancelled : SolverState::Failed;
		result.mErrorMessage = error.what();
	}

	cleanup();

	{
		std::lock_guard<std::mutex> lock(mResultMutex);
		mResult = result;
	}
	mState.store(result.mState);
}
