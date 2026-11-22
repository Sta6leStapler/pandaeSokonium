#include "../SolverRunner.h"
#include "../SolutionCodec.h"

#include <chrono>
#include <filesystem>
#include <iostream>
#include <stdexcept>
#include <thread>

void Require(bool condition, const char* message)
{
	if (!condition)
	{
		throw std::runtime_error(message);
	}
}

int main(int argc, char** argv)
{
	try
	{
		// 1. Festival出力フォーマットからのLURD抽出テスト
		const std::string sampleOut =
			"Festival 3.1 solution file\n"
			"===========================\n"
			"Level 1\n\n"
			"#####\n"
			"#@$.#\n"
			"#####\n\n"
			"Solution\n"
			"rR\n\n"
			"Solver: Festival 3.1\n";
		const std::string extracted = SolverRunner::ExtractLurdFromFestivalOutput(sampleOut);
		Require(extracted == "rR", "Extract single-line LURD from Festival output");

		const std::string multiLineOut =
			"Solution:\n"
			"uulDDD\n"
			"lluRdrU\n"
			"\n"
			"Solver: Festival 3.1\n";
		Require(SolverRunner::ExtractLurdFromFestivalOutput(multiLineOut) == "uulDDDlluRdrU",
			"Extract multi-line LURD from Festival output");

		Require(SolverRunner::ExtractLurdFromFestivalOutput("Level 1\n#####\n").empty(),
			"Missing solution section should return empty");

		// 2. 存在しない実行ファイル指定時は安全にFailedになること
		{
			SolverRunner runner;
			SolverRunner::SolverRequest req{};
			req.mExecutablePath = "nonexistent_festival_binary_12345";
			req.mStartBoard = { "#####", "#@$.#", "#####" };
			req.mTimeLimitSeconds = 5;
			req.mCores = 1;
			Require(runner.Start(req), "Runner must start");
			while (runner.GetState() == SolverRunner::SolverState::Running)
			{
				std::this_thread::sleep_for(std::chrono::milliseconds(10));
			}
			Require(runner.GetState() == SolverRunner::SolverState::Failed,
				"Missing executable must transition to Failed");
		}

		// 3. Festivalバイナリパスが引数で渡された場合は実際に1問解く統合テストを実施
		if (argc >= 2 && std::filesystem::is_regular_file(argv[1]))
		{
			SolverRunner runner;
			SolverRunner::SolverRequest req{};
			req.mExecutablePath = argv[1];
			req.mStartBoard = {
				"  ####",
				"  #  #",
				"###$ #",
				"# $.@#",
				"#    #",
				"### .#",
				"  #  #",
				"  ####"
			};
			req.mTimeLimitSeconds = 10;
			req.mCores = 1;
			Require(runner.Start(req), "Festival integration solve start");
			while (runner.GetState() == SolverRunner::SolverState::Running)
			{
				std::this_thread::sleep_for(std::chrono::milliseconds(20));
			}
			const auto res = runner.GetResult();
			Require(res.mState == SolverRunner::SolverState::Succeeded,
				("Festival integration solve failed: " + res.mErrorMessage).c_str());
			Require(!res.mNormalizedLurd.empty() && res.mMoveCount > 0 && res.mPushCount > 0,
				"Festival solution metrics must be positive");
			const auto sim = SolutionCodec::SimulateLurd(req.mStartBoard, res.mNormalizedLurd, true);
			Require(sim.mIsValid && sim.mReachesComplete, "Festival solution must strictly solve board");
		}

		std::cout << "SolverRunner tests passed\n";
		return 0;
	}
	catch (const std::exception& error)
	{
		std::cerr << error.what() << "\n";
		return 1;
	}
}
