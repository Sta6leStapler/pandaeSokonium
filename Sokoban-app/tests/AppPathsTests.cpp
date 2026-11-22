#include "../AppPaths.h"

#include <chrono>
#include <iostream>
#include <stdexcept>

namespace fs = std::filesystem;

void Require(bool condition, const char* message)
{
	if (!condition) throw std::runtime_error(message);
}

template <typename Action>
void RequireFailure(Action action, const char* message)
{
	try { action(); }
	catch (const std::exception&) { return; }
	throw std::runtime_error(message);
}

int main()
{
	const auto root = fs::temp_directory_path() / ("pandae-path-tests-"
		+ std::to_string(std::chrono::high_resolution_clock::now().time_since_epoch().count()));
	const auto previousDirectory = fs::current_path();
	try
	{
		const auto portable = root / fs::u8path(u8"portable \u65e5\u672c\u8a9e");
		fs::create_directories(portable / "Assets" / "Boards");
		AppPaths::WriteFile(portable / "game", "");
		AppPaths::WriteFile(portable / "Assets" / "Boards" / "same.txt", "bundled\n");
		fs::current_path(fs::temp_directory_path());
		AppPaths paths;
		paths.Initialize(portable / "game");
		Require(paths.Asset("Assets/Boards/same.txt") == portable / "Assets" / "Boards" / "same.txt",
			"Asset path must not depend on working directory");
		Require(paths.Asset("Boards/same.txt") == paths.Asset("Assets/Boards/same.txt"), "Logical asset prefix");
		RequireFailure([&]() { paths.Asset("../outside"); }, "Reject escaping asset paths");
		RequireFailure([&]() { paths.Asset((portable / "absolute").u8string()); }, "Reject absolute asset names");
		Require(fs::is_directory(paths.GetBoardsDirectory()), "Create saved boards directory");
		Require(fs::is_directory(paths.GetLogsDirectory()), "Create logs directory");
		AppPaths::WriteFile(paths.GetBoardsDirectory() / "same.txt", "saved\n");
		AppPaths::WriteFile(paths.GetBoardsDirectory() / "pack.sok", "Title: Pack\n#####\n#@$.#\n#####\n");
		AppPaths::WriteFile(paths.GetBoardsDirectory() / "stage.XSB", "#####\n#@$.#\n#####\n");
		const auto unicodeFile = paths.GetBoardsDirectory() / fs::u8path(u8"\u76e4\u9762.txt");
		AppPaths::WriteFile(unicodeFile, "unicode filename\n");
		const auto bytes = AppPaths::ReadFile(unicodeFile);
		Require(std::string(bytes.begin(), bytes.end()) == "unicode filename\n", "Unicode path roundtrip");
		const auto boards = paths.GetBoardFiles();
		Require(boards.size() == 5, "List bundled and saved boards across .txt/.xsb/.sok");
		Require(boards[0].key == "bundled:same.txt" && boards[1].key == "saved:pack.sok"
			&& boards[2].key == "saved:same.txt" && boards[3].key == "saved:stage.XSB",
			"Same names and extensions must have distinct stable keys");
		const auto log = paths.NewLogPath("2026-01-01-00-00-00");
		AppPaths::WriteFile(log, "first log");
		const auto secondLog = paths.NewLogPath("2026-01-01-00-00-00");
		Require(log != secondLog, "Automatic logs must not overwrite same-second logs");
		AppPaths::WriteFile(secondLog, "second log");
		RequireFailure([&]() { AppPaths::ReadFile(portable / "missing"); }, "Missing files must fail");
		RequireFailure([&]() { AppPaths::WriteFile(portable / "missing" / "file", "data"); }, "Write errors must fail");
		RequireFailure([&]() { AppPaths::WriteFile(paths.GetBoardsDirectory(), "data"); }, "Directory writes must fail");
		RequireFailure([&]() {
			AppPaths missing;
			missing.Initialize(portable / "Assets" / "Boards" / "same.txt");
		}, "Missing assets must fail, without working-directory fallback");
		AppPaths::WriteFile(paths.GetBoardsDirectory() / "same.txt", "updated\n");
		Require(paths.GetBoardFiles().size() == 5, "Repeated saves must not duplicate board identities");
		const auto original = AppPaths::ReadFile(portable / "Assets" / "Boards" / "same.txt");
		Require(std::string(original.begin(), original.end()) == "bundled\n", "Bundled data must remain untouched");

		const auto mac = root / "mac";
		const auto executable = mac / "Game.app" / "Contents" / "MacOS" / "game";
		fs::create_directories(executable.parent_path());
		fs::create_directories(mac / "Game.app" / "Contents" / "Resources" / "Assets" / "Boards");
		AppPaths::WriteFile(executable, "");
		AppPaths bundlePaths;
		bundlePaths.Initialize(executable);
		Require(bundlePaths.Asset("Floor.png") == mac / "Game.app" / "Contents" / "Resources" / "Assets" / "Floor.png",
			"App bundle resources");
		Require(bundlePaths.GetBoardsDirectory() == mac / "UserData" / "Boards", "Save outside app bundle");

		// Festival実行ファイル探索テスト
#if defined(_WIN32)
		const std::string festivalName = "festival.exe";
#else
		const std::string festivalName = "festival";
#endif
		Require(paths.FindFestivalExecutable().empty(), "Missing festival executable returns empty path");
		fs::create_directories(portable / "Tools");
		AppPaths::WriteFile(portable / "Tools" / festivalName, "dummy");
		Require(paths.FindFestivalExecutable() == portable / "Tools" / festivalName, "Find bundled festival in Tools/");

		const auto blocked = root / "blocked";
		fs::create_directories(blocked / "Assets");
		AppPaths::WriteFile(blocked / "game", "");
		AppPaths::WriteFile(blocked / "UserData", "not a directory");
		RequireFailure([&]() {
			AppPaths blockedPaths;
			blockedPaths.Initialize(blocked / "game");
		}, "Storage creation errors must fail");
		fs::current_path(previousDirectory);
		fs::remove_all(root);
		std::cout << "AppPaths tests passed\n";
		return 0;
	}
	catch (const std::exception& error)
	{
		fs::current_path(previousDirectory);
		fs::remove_all(root);
		std::cerr << error.what() << "\n";
		return 1;
	}
}
