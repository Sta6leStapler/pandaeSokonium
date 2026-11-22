#include "AppPaths.h"

#include <algorithm>
#include <cctype>
#include <cstdint>
#include <fstream>
#include <iterator>
#include <stdexcept>
#include <system_error>

#if defined(_WIN32)
#define NOMINMAX
#include <Windows.h>
#elif defined(__APPLE__)
#include <mach-o/dyld.h>
#else
#include <unistd.h>
#include <cerrno>
#endif

namespace
{
	std::filesystem::path ExecutablePath()
	{
#if defined(_WIN32)
		std::vector<wchar_t> buffer(1024);
		for (;;)
		{
			const DWORD length = GetModuleFileNameW(nullptr, buffer.data(), static_cast<DWORD>(buffer.size()));
			if (length == 0)
				throw std::system_error(GetLastError(), std::system_category(), "GetModuleFileNameW");
			if (length < buffer.size())
				return std::filesystem::path(std::wstring(buffer.data(), length));
			buffer.resize(buffer.size() * 2);
		}
#elif defined(__APPLE__)
		uint32_t size = 0;
		_NSGetExecutablePath(nullptr, &size);
		std::vector<char> buffer(size);
		if (_NSGetExecutablePath(buffer.data(), &size) != 0)
			throw std::runtime_error("Unable to resolve executable path");
		return std::filesystem::canonical(buffer.data());
#else
		std::vector<char> buffer(1024);
		for (;;)
		{
			const auto length = readlink("/proc/self/exe", buffer.data(), buffer.size());
			if (length < 0)
				throw std::system_error(errno, std::generic_category(), "readlink /proc/self/exe");
			if (static_cast<std::size_t>(length) < buffer.size())
				return std::filesystem::path(std::string(buffer.data(), length));
			buffer.resize(buffer.size() * 2);
		}
#endif
	}
}

void AppPaths::Initialize()
{
	Initialize(ExecutablePath());
}

void AppPaths::Initialize(const std::filesystem::path& executable)
{
	const auto directory = std::filesystem::canonical(executable).parent_path();
	auto portableDirectory = directory;
	mExecutableDirectory = directory;
	mAssetDirectory = directory / "Assets";
	mToolsDirectory = directory / "Tools";
	if (directory.filename() == "MacOS" && directory.parent_path().filename() == "Contents"
		&& directory.parent_path().parent_path().extension() == ".app")
	{
		mAssetDirectory = directory.parent_path() / "Resources" / "Assets";
		mToolsDirectory = directory.parent_path() / "Resources" / "Tools";
		portableDirectory = directory.parent_path().parent_path().parent_path();
	}

	if (!std::filesystem::is_directory(mAssetDirectory))
		throw std::runtime_error("Asset directory not found: " + mAssetDirectory.u8string());

	mBoardsDirectory = portableDirectory / "UserData" / "Boards";
	mLogsDirectory = portableDirectory / "UserData" / "Logs";
	mUserToolsDirectory = portableDirectory / "UserData" / "Tools";
	std::filesystem::create_directories(mBoardsDirectory);
	std::filesystem::create_directories(mLogsDirectory);
}

std::filesystem::path AppPaths::FindFestivalExecutable(const std::filesystem::path& customPath) const
{
#if defined(_WIN32)
	const std::string exeName = "festival.exe";
#else
	const std::string exeName = "festival";
#endif
	if (!customPath.empty() && std::filesystem::is_regular_file(customPath))
	{
		return customPath;
	}

	const std::vector<std::filesystem::path> candidates = {
		mToolsDirectory / exeName,
		mExecutableDirectory / exeName,
		mUserToolsDirectory / exeName
	};
	for (const auto& candidate : candidates)
	{
		std::error_code ec;
		if (std::filesystem::is_regular_file(candidate, ec))
		{
			return candidate;
		}
	}
	return {};
}

std::filesystem::path AppPaths::Asset(const std::string& name) const
{
	auto relative = std::filesystem::u8path(name).lexically_normal();
	if (relative.empty() || relative.is_absolute() || relative.has_root_path())
		throw std::invalid_argument("Asset name must be relative: " + name);
	if (*relative.begin() == "Assets")
		relative = relative.lexically_relative("Assets");
	for (const auto& part : relative)
	{
		if (part == "..")
			throw std::invalid_argument("Asset name escapes asset directory: " + name);
	}
	return mAssetDirectory / relative;
}

std::vector<AppPaths::BoardFile> AppPaths::GetBoardFiles() const
{
	auto isSupportedBoardExt = [](const std::filesystem::path& path) {
		std::string ext = path.extension().u8string();
		std::transform(ext.begin(), ext.end(), ext.begin(), [](unsigned char c) {
			return static_cast<char>(std::tolower(c));
		});
		return ext == ".txt" || ext == ".xsb" || ext == ".sok";
	};

	std::vector<BoardFile> files;
	for (bool saved : { false, true })
	{
		const auto directory = saved ? mBoardsDirectory : Asset("Boards");
		for (const auto& entry : std::filesystem::directory_iterator(directory))
		{
			if (entry.is_regular_file() && isSupportedBoardExt(entry.path()))
				files.push_back({ entry.path(),
					(saved ? "saved:" : "bundled:") + entry.path().filename().u8string(), saved });
		}
	}
	std::sort(files.begin(), files.end(), [](const BoardFile& a, const BoardFile& b) {
		return a.key < b.key;
	});
	return files;
}

std::filesystem::path AppPaths::NewLogPath(const std::string& timestamp) const
{
	auto path = mLogsDirectory / std::filesystem::u8path(timestamp + ".txt");
	for (unsigned int suffix = 1; std::filesystem::exists(path); ++suffix)
		path = mLogsDirectory / std::filesystem::u8path(timestamp + "_" + std::to_string(suffix) + ".txt");
	return path;
}

std::vector<char> AppPaths::ReadFile(const std::filesystem::path& path)
{
	std::ifstream file(path, std::ios::binary);
	if (!file)
		throw std::runtime_error("Unable to read file: " + path.u8string());
	std::vector<char> contents{ std::istreambuf_iterator<char>(file), std::istreambuf_iterator<char>() };
	if (file.bad())
		throw std::runtime_error("Failed while reading file: " + path.u8string());
	return contents;
}

void AppPaths::WriteFile(const std::filesystem::path& path, const std::string& contents)
{
	std::ofstream file(path, std::ios::binary);
	if (!file)
		throw std::runtime_error("Unable to open file for writing: " + path.u8string());
	file.write(contents.data(), static_cast<std::streamsize>(contents.size()));
	file.close();
	if (!file)
		throw std::runtime_error("Failed while writing file: " + path.u8string());
}
