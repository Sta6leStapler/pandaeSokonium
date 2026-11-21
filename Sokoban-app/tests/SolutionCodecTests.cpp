#include "../SolutionCodec.h"

#include <iostream>
#include <stdexcept>
#include <string>
#include <vector>

void Require(bool condition, const char* message)
{
	if (!condition)
	{
		throw std::runtime_error(message);
	}
}

int main()
{
	try
	{
		// 1. 1手エンコード
		Require(SolutionCodec::EncodeMove(1, 0, false) == 'r', "Encode r");
		Require(SolutionCodec::EncodeMove(1, 0, true) == 'R', "Encode R");
		Require(SolutionCodec::EncodeMove(-1, 0, false) == 'l', "Encode l");
		Require(SolutionCodec::EncodeMove(-1, 0, true) == 'L', "Encode L");
		Require(SolutionCodec::EncodeMove(0, -1, false) == 'u', "Encode u");
		Require(SolutionCodec::EncodeMove(0, -1, true) == 'U', "Encode U");
		Require(SolutionCodec::EncodeMove(0, 1, false) == 'd', "Encode d");
		Require(SolutionCodec::EncodeMove(0, 1, true) == 'D', "Encode D");
		Require(SolutionCodec::EncodeMove(1, 1, false) == '\0', "Diagonal move must be invalid");

		// 2. 空白・改行・RLE展開
		std::string normalized;
		std::string error;
		Require(SolutionCodec::NormalizeLurdString(" r 2R\n3(ul) ", normalized, error), "Normalize with whitespace and RLE");
		Require(normalized == "rRRululul", "Normalized RLE string matches");
		Require(!SolutionCodec::NormalizeLurdString("rrXll", normalized, error), "Invalid character must fail");

		// 3. 盤面シミュレーション（正常クリア）
		const std::vector<std::string> board = {
			"#######",
			"#@ $ .#",
			"#######"
		};
		const auto ok = SolutionCodec::SimulateLurd(board, "rRR");
		Require(ok.mIsValid, "Valid solution must succeed");
		Require(ok.mReachesComplete, "Solution must reach completion");
		Require(!ok.mHasCaseMismatchWarning, "Exact case should have no warning");
		Require(ok.mMoveCount == 3 && ok.mPushCount == 2, "Move and push counts");
		Require(ok.mNormalizedLurd == "rRR", "Normalized LURD matches");

		// 4. 寛容モード（小文字押し入力を自動補正して受け入れ）
		const auto lenient = SolutionCodec::SimulateLurd(board, "rrr", false);
		Require(lenient.mIsValid && lenient.mReachesComplete, "Lenient mode must accept lowercase push");
		Require(lenient.mHasCaseMismatchWarning, "Lenient mode should set case mismatch warning");
		Require(lenient.mNormalizedLurd == "rRR", "Lenient mode should normalize to uppercase push");

		// 5. 厳密モード（大文字小文字不一致を拒否）
		const auto strict = SolutionCodec::SimulateLurd(board, "rrr", true);
		Require(!strict.mIsValid && strict.mErrorIndex == 1, "Strict mode must reject lowercase push");

		// 6. 違法手検出（壁衝突・二重荷物押し）
		const auto wallHit = SolutionCodec::SimulateLurd(board, "u");
		Require(!wallHit.mIsValid && wallHit.mErrorIndex == 0, "Wall collision must fail");

		const std::vector<std::string> doubleBoxBoard = {
			"#######",
			"#@$$..#",
			"#######"
		};
		const auto doublePush = SolutionCodec::SimulateLurd(doubleBoxBoard, "R");
		Require(!doublePush.mIsValid && doublePush.mErrorIndex == 0, "Double box push must fail");

		std::cout << "SolutionCodec tests passed\n";
		return 0;
	}
	catch (const std::exception& error)
	{
		std::cerr << error.what() << "\n";
		return 1;
	}
}
