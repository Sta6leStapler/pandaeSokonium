#pragma once

#include <filesystem>
#include <string>
#include <vector>

class AppPaths
{
public:
	struct BoardFile
	{
		std::filesystem::path path;
		std::string key;
		bool saved;
	};

	void Initialize();
	void Initialize(const std::filesystem::path& executable);
	std::filesystem::path Asset(const std::string& name) const;
	const std::filesystem::path& GetBoardsDirectory() const { return mBoardsDirectory; }
	const std::filesystem::path& GetLogsDirectory() const { return mLogsDirectory; }
	std::vector<BoardFile> GetBoardFiles() const;
	std::filesystem::path NewLogPath(const std::string& timestamp) const;

	static std::vector<char> ReadFile(const std::filesystem::path& path);
	static void WriteFile(const std::filesystem::path& path, const std::string& contents);

private:
	std::filesystem::path mAssetDirectory;
	std::filesystem::path mBoardsDirectory;
	std::filesystem::path mLogsDirectory;
};
