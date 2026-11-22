#pragma once

#include <cstddef>
#include <string>
#include <vector>

// rludRLUD（LURD）形式の解手順エンコード・デコード・盤面検証を行うモジュール
namespace SolutionCodec
{
	// 1手分の移動・押し情報
	struct SolutionStep
	{
		int mFromX;            // プレイヤー移動前X
		int mFromY;            // プレイヤー移動前Y
		int mToX;              // プレイヤー移動後X
		int mToY;              // プレイヤー移動後Y
		bool mIsPush;          // 荷物を押したか
		int mBaggageFromX;     // 荷物移動前X（押していない時は0）
		int mBaggageFromY;     // 荷物移動前Y（押していない時は0）
		int mBaggageToX;       // 荷物移動後X（押していない時は0）
		int mBaggageToY;       // 荷物移動後Y（押していない時は0）
		char mNormalizedChar;  // 正規化された1文字（r,l,u,d,R,L,U,D）
	};

	// LURD手順シミュレーションの検証結果
	struct SimulationResult
	{
		bool mIsValid;                 // 違法手がなく全手順を適用できたか
		bool mReachesComplete;         // 適用後に全ての荷物がゴール上にあるか
		bool mHasCaseMismatchWarning;  // 寛容モードで大文字/小文字の押し判定を自動補正したか
		std::size_t mErrorIndex;       // エラー発生位置（0始まり）
		std::string mErrorMessage;     // エラー詳細
		std::string mNormalizedLurd;   // 盤面の荷物配置に合わせて正規化したrludRLUD文字列
		unsigned int mMoveCount;       // 総手数
		unsigned int mPushCount;       // 総プッシュ数
		std::vector<SolutionStep> mSteps;
	};

	// 1ステップの変位と押しフラグを rludRLUD の1文字に変換する（無効な変位なら '\0'）
	char EncodeMove(int dx, int dy, bool isPush);

	// 入力文字列から空白・改行を除き、RLE圧縮（例: "3r2U"）を展開して rludRLUD 文字列へ正規化する
	bool NormalizeLurdString(const std::string& rawInput, std::string& outMoves, std::string& outError);

	// 開始盤面に対してLURD手順を1手ずつ検証・シミュレーションする
	// strictCase == false（標準の寛容モード）では盤面上の荷物有無に合わせて押し/移動を自動判定し、
	// 壁衝突や二重荷物押し等の違法手のみをエラーとする。
	SimulationResult SimulateLurd(const std::vector<std::string>& startBoard, const std::string& rawInput, bool strictCase = false);
}
