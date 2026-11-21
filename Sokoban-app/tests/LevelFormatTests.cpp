#include "../LevelFormat.h"

#include <chrono>
#include <filesystem>
#include <iostream>
#include <stdexcept>

namespace fs = std::filesystem;

void Require(bool condition, const char* message)
{
	if (!condition)
	{
		throw std::runtime_error(message);
	}
}

template <typename Action>
void RequireFailure(Action action, const char* message)
{
	try
	{
		action();
	}
	catch (const std::exception&)
	{
		return;
	}
	throw std::runtime_error(message);
}

int main()
{
	const auto root = fs::temp_directory_path() / ("pandae-levelformat-tests-"
		+ std::to_string(std::chrono::high_resolution_clock::now().time_since_epoch().count()));
	try
	{
		fs::create_directories(root);

		// 1. 拡張子判定
		Require(LevelFormat::IsSupportedExtension("level.txt"), ".txt must be supported");
		Require(LevelFormat::IsSupportedExtension("level.XSB"), ".XSB must be supported");
		Require(LevelFormat::IsSupportedExtension("collection.sok"), ".sok must be supported");
		Require(!LevelFormat::IsSupportedExtension("image.png"), ".png must not be supported");

		// 2. シンプルな .txt 形式
		const std::string txtData =
			"#####\n"
			"#@$.#\n"
			"#####\n";
		const auto txtCol = LevelFormat::ParseCollection(txtData, "simple.txt");
		Require(txtCol.mLevels.size() == 1, "Single txt level count");
		Require(txtCol.mLevels[0].mTitle == "simple", "Fallback title from filename");
		Require(txtCol.mLevels[0].mBoard.size() == 3 && txtCol.mLevels[0].mBoard[1] == "#@$.#", "Txt board content");

		// 3. 複数レベル + メタデータ + 同梱Solution を含む .sok 形式（代替床 '-' / '_' と行幅不揃いを含む）
		const std::string sokData =
			":: Sokoban YASC Collection\n"
			"Collection: Micro Pack\n"
			"\n"
			"Title: First Stage\n"
			"Author: Tester\n"
			"#####\n"
			"#@$-.#\n"
			"######\n"
			"\n"
			"Solution:\n"
			"rR\n"
			"\n"
			"Second Stage\n"
			"######\n"
			"#_@$.#\n"
			"######\n";
		const auto sokCol = LevelFormat::ParseCollection(sokData, "micro.sok");
		Require(sokCol.mCollectionTitle == "Micro Pack", "Collection title metadata");
		Require(sokCol.mLevels.size() == 2, "Multi-level sok count");
		Require(sokCol.mLevels[0].mTitle == "First Stage", "First level title");
		Require(sokCol.mLevels[0].mAuthor == "Tester", "First level author");
		Require(sokCol.mLevels[0].mBoard[0] == "##### ", "Uniform right-padded width");
		Require(sokCol.mLevels[0].mBoard[1] == "#@$ .#", "Dash normalized to floor space");
		Require(sokCol.mLevels[0].mSolutions.size() == 1 && sokCol.mLevels[0].mSolutions[0] == "rR", "Embedded solution parsed");
		Require(sokCol.mLevels[1].mTitle == "Second Stage", "Plain title line before board");
		Require(sokCol.mLevels[1].mBoard[1] == "# @$.#", "Underscore normalized to floor space");

		// 4. RLE圧縮盤面（数値・'|'・括弧）
		const std::string rleData =
			"; Title: RLE Stage\n"
			"5#|#@$.#|2(2#)#\n";
		const auto rleCol = LevelFormat::ParseCollection(rleData, "rle.xsb");
		Require(rleCol.mLevels.size() == 1, "RLE level parsed");
		Require(rleCol.mLevels[0].mTitle == "RLE Stage", "Semicolon title metadata");
		Require(rleCol.mLevels[0].mBoard.size() == 3 && rleCol.mLevels[0].mBoard[2] == "#####", "RLE parenthesis expansion");

		// 5. 不正盤面の拒否（プレイヤー不在・荷物とゴール数不一致）
		RequireFailure([&]() {
			LevelFormat::ParseCollection("#####\n# $.#\n#####\n");
		}, "Missing player must fail");
		RequireFailure([&]() {
			LevelFormat::ParseCollection("#####\n#@$$.#\n#####\n");
		}, "Box/goal mismatch must fail");

		// 6. 3形式書き出し -> 再読込ラウンドトリップ
		LevelFormat::LevelData saveTarget = sokCol.mLevels[0];
		for (const std::string& ext : { ".txt", ".xsb", ".sok" })
		{
			const auto filePath = root / ("roundtrip" + ext);
			LevelFormat::SaveLevelToFile(filePath, saveTarget);
			const auto reloaded = LevelFormat::LoadCollectionFromFile(filePath);
			Require(reloaded.mLevels.size() == 1, "Reloaded single level");
			Require(reloaded.mLevels[0].mBoard == saveTarget.mBoard, "Roundtrip board matches");
			if (ext == ".sok")
			{
				Require(reloaded.mLevels[0].mTitle == "First Stage", "SOK title preserved");
				Require(reloaded.mLevels[0].mSolutions.size() == 1 && reloaded.mLevels[0].mSolutions[0] == "rR", "SOK solution preserved");
			}
		}

		fs::remove_all(root);
		std::cout << "LevelFormat tests passed\n";
		return 0;
	}
	catch (const std::exception& error)
	{
		fs::remove_all(root);
		std::cerr << error.what() << "\n";
		return 1;
	}
}
