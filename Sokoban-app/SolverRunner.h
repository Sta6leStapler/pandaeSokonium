#pragma once

#include <atomic>
#include <chrono>
#include <cstdint>
#include <filesystem>
#include <mutex>
#include <string>
#include <thread>
#include <vector>

// 外部倉庫番ソルバー（Festival）をクロスプラットフォームに非同期実行するクラス
class SolverRunner
{
public:
	enum class SolverState
	{
		Idle,       // 待機中
		Running,    // 解析実行中
		Succeeded,  // 解を発見
		Failed,     // 解なし・エラー
		TimedOut,   // 時間切れ
		Cancelled   // ユーザーにより中断
	};

	struct SolverRequest
	{
		std::filesystem::path mExecutablePath;  // Festival実行ファイルパス
		std::vector<std::string> mStartBoard;   // 開始盤面（XSB行配列）
		int mTimeLimitSeconds;                  // 探索制限時間（秒）
		int mCores;                             // 使用コア数（1/2/4/8）
	};

	struct SolverResult
	{
		SolverState mState;                     // 終了状態
		std::string mLurdSolution;              // Festivalが返した生LURD文字列
		std::string mNormalizedLurd;            // 盤面検証・正規化済みrludRLUD文字列
		unsigned int mMoveCount;                // 手数
		unsigned int mPushCount;                // プッシュ数
		double mElapsedSeconds;                 // 所要時間（秒）
		std::string mErrorMessage;              // 失敗時メッセージ
		std::string mRawOutput;                 // デバッグ用出力抜粋
	};

	SolverRunner();
	~SolverRunner();

	// ソルバーをバックグラウンドスレッドで開始する
	bool Start(const SolverRequest& request);

	// 実行中のソルバー子プロセスを安全に停止し、スレッド完了を待つ
	void Cancel();

	// 現在の状態・経過時間・結果を取得する
	SolverState GetState() const { return mState.load(); }
	double GetElapsedSeconds() const;
	SolverResult GetResult() const;

	// Festivalの -out_file 出力（または標準出力）からLURD解文字列を抽出する
	static std::string ExtractLurdFromFestivalOutput(const std::string& outFileText, const std::string& stdOutText = "");

private:
	void RunWorker(SolverRequest request);
	void TerminateActiveProcess();

	std::thread mWorkerThread;
	std::atomic<SolverState> mState;
	std::atomic<bool> mCancelRequested;
	std::chrono::steady_clock::time_point mStartTime;

	mutable std::mutex mResultMutex;
	SolverResult mResult;

	mutable std::mutex mProcessMutex;
	std::uintptr_t mActiveProcessHandle; // Windows: HANDLE, POSIX: pid_t
};
