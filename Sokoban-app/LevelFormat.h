#pragma once

#include <filesystem>
#include <string>
#include <vector>

// 倉庫番レベルデータ（.sok / .xsb / .txt）の読み込み・書き出しを扱うモジュール
namespace LevelFormat
{
	// 1盤面分のデータとメタ情報
	struct LevelData
	{
		std::string mTitle;                   // レベル名（未指定時は空）
		std::string mAuthor;                  // 作者名（未指定時は空）
		std::string mComment;                 // 備考・コメント
		std::vector<std::string> mBoard;      // 正規化済みXSB盤面（全行同幅・床は' '）
		std::vector<std::string> mSolutions;  // 同梱LURD解手順（あれば保持）
	};

	// 1ファイルに収録されたレベルコレクション
	struct LevelCollection
	{
		std::filesystem::path mFilePath;      // 読み込み元ファイルパス
		std::string mCollectionTitle;         // コレクション名
		std::vector<LevelData> mLevels;       // 収録レベル一覧
	};

	// 対応拡張子（.txt / .xsb / .sok、大文字小文字不問）か判定する
	bool IsSupportedExtension(const std::filesystem::path& path);

	// 拡張子からフォーマット名（"sok" / "xsb" / "txt"）を判定する
	std::string DetectFormatFromPath(const std::filesystem::path& path);

	// RLE圧縮された盤面文字列（例: "5#|#@$.#|5#" や "2(2#) "）を展開する
	bool ExpandBoardRle(const std::string& input, std::vector<std::string>& outRows, std::string& outError);

	// 盤面の行文字（'-', '_' -> ' '）と矩形幅を正規化する
	std::vector<std::string> NormalizeBoard(const std::vector<std::string>& rawRows);

	// 盤面が倉庫番として妥当か検証する（プレイヤー1人、荷物数==ゴール数>=1、壁存在）
	bool ValidateBoard(const std::vector<std::string>& board, std::string& outError);

	// テキスト内容から .sok / .xsb / .txt のレベルコレクションをパースする
	LevelCollection ParseCollection(const std::string& text, const std::filesystem::path& sourcePath = {});

	// ファイルを読み込んでレベルコレクションを返す
	LevelCollection LoadCollectionFromFile(const std::filesystem::path& path);

	// 1盤面を指定フォーマット（"txt" / "xsb" / "sok"）のテキストへ整形する
	std::string FormatLevel(const LevelData& level, const std::string& format = "txt");

	// 1盤面をファイルへ書き出す（formatが空なら拡張子から自動判定）
	void SaveLevelToFile(const std::filesystem::path& path, const LevelData& level, const std::string& format = "");
}
