#pragma once

#include "SFML/Graphics.hpp"
#include <unordered_map>
#include <string>
#include <vector>
#include <chrono>

#include "Player.h"
#include "Baggage.h"
#include "GameBoard.h"
#include "ParamStruct.h"

#include "TGUI/TGUI.hpp"
#include "TGUI/Backend/SFML-Graphics.hpp"

class Player;
class GameBoard;

// 一動作のログ
struct Log
{
	// プレイヤーと荷物の移動前後の座標
	std::pair<sf::Vector2i, sf::Vector2i> pCoordinate, bCoordinate;
	// 荷物が移動したかどうか
	bool isBMoved;
	// プレイヤーの移動前後の向き
	Player::Direction direction1, direction2;
	// 入力時の時刻の文字列
	std::string time;
	// 現在の状態から派生した、次状態のリスト
	std::vector<std::vector<Log>> thread;
};

using BoundingBox = std::pair<sf::Vector2f, sf::Vector2f>;

class Game
{
public:
	Game();
	bool Initialize();
	void RunLoop();
	void Shutdown();

	// ゲームの状態
	enum GameState
	{
		EGamePlay,
		EPaused,
		EQuit
	};

	// スナップショット保存用の構造体
	struct GameSnapshot
	{
		// 1. 復元用データ
		sf::Vector2i playerPosition;
		Player::Direction playerDirection;
		std::vector<sf::Vector2i> baggagePositions; // 全ての荷物の座標
		unsigned int stepCount;
		std::vector<Log> logs; // その時点までのログ

		// 2. UI表示用データ
		std::string snapshotName;
		std::string snapshotComment;
		std::string timestamp; // 保存時刻 (例: "2025/10/20 19:30")
	};

	// テクスチャのロード
	sf::Texture* LoadTexture(const std::string& fileName);

	// アクターの追加と削除
	void AddActor(class IActor* actor);
	void RemoveActor(class IActor* actor);

	// スプライトの追加と削除
	void AddSprite(class SpriteComponent* sprite);
	void RemoveSprite(class SpriteComponent* sprite);

	// スプライトとUIの描画
	void DrawSprites();
	void DrawUI();

	// UI画面のスタックに関する処理
	// スタック全体を参照で返す
	const std::vector<class IUIScreen*>& GetUIStack() { return mUIStack; }
	// 指定のUIScreenをスタックにプッシュする
	void PushUI(class IUIScreen* screen) { mUIStack.emplace_back(screen); }

	// ゲーム特有のメンバ関数があれば追加
	// 盤面リストに新しい盤面を追加
	void AddBoard(const std::string& boardName, const std::vector<std::string>& boardData);
	
	// ステップを加算
	void AddStep() { mStep++; }

	// スクリーン座標をタイル座標に変換する
	sf::Vector2i ScreenToTileCoords(const sf::Vector2f& screenPos) const;
	// タイル座標をスクリーン座標に変換する
	sf::Vector2f TileToScreenCoords(const sf::Vector2i& tileCorrdsPos) const;

	// undo/redo処理
	void CallUndo();
	void CallRedo();
	void CallReset();
	void CallRedoAll();

	// 盤面をセーブ
	void CallSave(const std::vector<std::string>& boardData = {});

	// 盤面のリロード
	void CallReload();

	// 全てのリセット
	void CallRestart();

	// 先のログを削除 (プレイヤーの移動処理時に呼び出す)
	void RemoveRedo();

	// ログを追加
	void AddLog(const sf::Vector2i& playerPos1, const sf::Vector2i& playerPos2, const Player::Direction& direction1, const Player::Direction& direction2);
	void AddLog(const sf::Vector2i& playerPos1, const sf::Vector2i& playerPos2, const sf::Vector2i& baggagePos1, const sf::Vector2i& baggagePos2, const Player::Direction& direction1, const Player::Direction& direction2);

	// ログをテキストに変換
	std::string ConvertLogToStr(const std::vector<Log>& logs, const unsigned long long& current = 0);

	// ログの出力
	void OutputLogs();

	// スナップショット関連の関数
	// 現在のゲーム状態をスナップショットとして保存
	void AddSnapshot(const std::string& name, const std::string& comment);
	// スナップショット一覧UIを表示
	void DisplaySnapshot();
	// スナップショットの名前・コメント入力UIを表示
	void AddSnapshotDialog();
	// 指定されたインデックスのスナップショットを復元
	void RestoreSnapshot(int snapshotIndex);
	// 保持しているスナップショットをすべて消去
	void ClearSnapshots();
	// スナップショットのリスト（データのみ）を取得
	const std::vector<GameSnapshot>& GetSnapshots() const { return mSnapshots; }
	// スナップショットデータからサムネイルテクスチャを動的に生成
	sf::Texture* GenerateThumbnail(const GameSnapshot& snapshotData);
	// 現在の盤面状況からサムネイルを動的に生成
	sf::Texture* GenerateCurrentThumbnail();
	// サムネイルのサイズを取得
	sf::Vector2u GetThumbnailSize() const;
	// スナップショットのコメントを更新
	void SetSnapshotComment(const int index, const std::string& comment = "") { mSnapshots[index].snapshotComment = comment; }
	void SetSnapshotComment(const std::string& title, const std::string& comment = "")
	{
		for (auto& snapshot : mSnapshots)
		{
			if (snapshot.snapshotName == title)
			{
				snapshot.snapshotComment = comment;
				return;
			}
		}
	}
	// 指定されたスナップショットを削除
	void RemoveSnapshot(const int index) { if (index >= 0 && index < mSnapshots.size()) mSnapshots.erase(mSnapshots.begin() + index); }
	void RemoveSnapshot(const std::string& title)
	{
		for (auto iter = mSnapshots.begin(); iter != mSnapshots.end(); iter++)
		{
			if (iter->snapshotName == title)
			{
				mSnapshots.erase(iter);
				return;
			}
		}
	}
	// スナップショット関連の関数ここまで

	// エディットモード関連の関数
	// レベルエディタの初期設定を行うウィンドウを呼び出す。
	// レベルエディタウィンドウのDisplayEditorScreen関数の呼び出しまでやる。
	void CallEditorSetup();
	// レベルエディタUIを表示する。editCurrentBoard trueならば現在の盤面をロードして編集
	void DisplayEditorScreen(const int mode = 0);
	// エディタで編集した盤面を適用してリロード
	void ApplyEditedBoard(const std::vector<std::string>& newBoardData);
	
	// エディットモード関連の関数ここまで

	// 現在日時を文字列で出力
	std::string GetDateTime();

	// 終了判定を行い、ウィンドウで通知
	void HasComplete();
	void DisplayResult();

	// 盤面の規模の入力
	bool InputBoardData();

	// ヘルプウィンドウの表示
	void DisplayHelpWindow();

	// 盤面リストを表示し、選択した盤面に変更する
	void SelectBoards();
	void ChangeBoard();

	// プレイ履歴の再生
	// リザルト画面から呼び出すときは引数に盤面のキーを入れ、
	// そうでないときは引数無しで呼び出す
	void DisplayPlayLogs(const std::string& boardKey = "");

	// パラメータのリセットを行う処理
	// Resetは同じ盤面でパラメータをリセットする場合
	// Clearは別の盤面でパラメータをリセットする場合
	// ClearParametersの後にパラメータを設定せず盤面読み込みに入らないこと
	void ResetParameters();
	void ClearParameters();

	// ゲッターとセッター
	// ゲーム制御関連
	void SetState(const GameState& gameState) { mGameState = gameState; }
	sf::Vector2f GetWindowSize() const { return mWindowSize; }
	std::vector<std::string> GetFilenames() const { return mFilenames; }
	std::string GetCurrentKey() const { return mCurrentKey; }
	std::string GetFilename(unsigned int num) const { return mFilenames.at(num); }
	std::unordered_map<std::string, std::vector<std::string>> GetBoardData() const { return mBoardData; }
	sf::Vector2i GetBoardSize() const { return mGenParams.boardSize; }
	BoundingBox GetBoardViewArea() const { return mBoardViewArea; }
	unsigned int GetStep() const { return mStep; }
	double GetSecTime() const { return static_cast<double>(std::chrono::duration_cast<std::chrono::seconds>(std::chrono::system_clock::now() - mStart).count()); }
	double GetMSecTime() const { return static_cast<double>(std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::system_clock::now() - mStart).count()); }
	float GetTapThresHold() const { return TAP_THRESHOLD; }
	float GetHoldThresHold() const { return HOLD_THRESHOLD; }
	float GetAutoRepeatInterval() const { return AUTO_REPEAT_INTERVAL; }
	sf::Font GetFontSFML() const { return mFontSFML; }
	tgui::Font GetFontTGUI() const { return *mFontTGUI; }
	tgui::Theme GetTheme() const { return *mTheme; };

	// テクスチャ関連
	sf::Texture* GetTexture(const std::string& fileName) const { return mTextures.at(fileName); }
	std::unordered_map<std::string, sf::Texture*> GetTextures() const { return mTextures; }
	sf::Vector2f GetTileSize() const;

	// 盤面関連
	std::vector<class Baggage*>& GetBaggages() { return mBaggages; }
	class Baggage* GetBaggageFromPos(const sf::Vector2i& pos);
	std::vector<sf::Vector2i> GetBaggagesPos() const;
	std::vector<sf::Vector2i> GetGoalPos() const { return mGoalPos; }
	class Player* GetPlayer() const { return mPlayer; }
	int GetBaggageNumLimit(const sf::Vector2i& size, const double& wallRate = 0.0) const;
	void SetCurrentKey(const std::string& key) { mCurrentKey = key; }
	std::vector<std::string> GetCurrentInitBoardData() const { return mInitBoardData.at(mCurrentKey); }
	std::vector<std::string> GetInitBoardData(const std::string& boardKey) const { return mInitBoardData.at(boardKey); }
	// 現在の盤面（動いている状態）を文字列配列で取得
	std::vector<std::string> GetBoardStateWithObjects() const;


	// アクターからハイライトの更新依頼を受け取り、更新が必要アクターに指示を出す関数
	void SetMoveHighlights(const std::vector<sf::Vector2i>& tiles);
	void ClearMoveHighlights();
	std::vector<sf::Vector2i> GetMoveHighlightedTiles() const { return mGameBoard->GetMoveHighlightedTiles(); }
	void SetPushHighlights(const std::vector<sf::Vector2i>& tiles, class Baggage* baggage);
	void ClearPushHighlights();
	std::vector<sf::Vector2i> GetPushHighlightedTiles() const { return mGameBoard->GetPushHighlightedTiles(); }
	void SetPushDirections(const std::vector<int>& indexes, class Baggage* baggage);
	void ClearPushDirections();
	void SetBaggagesIdleState();
	bool ExistsBaggagesHighlightingState();

	class HUDHelper* GetHUDHelper() const { return mHUDHelper; }
	std::vector<std::string> GetBoardState() const { return mBoardState; }

private:
	void ProcessInput();
	void UpdateGame();
	void GenerateOutput();
	void LoadData();
	void UnloadData();

	// ヘルパー関数は以下に定義
	void SyncSliderWithEditBox(tgui::Slider::Ptr slider, tgui::EditBox::Ptr editBox, const bool& isInteger);
	// 長押し処理のあるボタンの長押し状態を解除する関数
	void CancelAllUIHolds();

	// ロードされたテクスチャのマップ
	std::unordered_map<std::string, sf::Texture*> mTextures;

	// アクティブなアクターと待機中のアクター
	std::vector<IActor*> mActiveActors;
	std::vector<IActor*> mPendingActors;
	// ゲームクラスからアクセス可能なアクター
	// (入力や更新の処理でこれらを用いることはない)
	class Player* mPlayer;
	class GameBoard* mGameBoard;
	std::vector<class Baggage*> mBaggages;
	
	// 描画を行うコンポーネント
	std::vector<class SpriteComponent*> mSprites;

	sf::RenderWindow* mWindow;
	sf::Clock mClock;
	sf::Time mTicksCount;
	sf::Font mFontSFML;
	sf::Text mInfoTxt;

	// ゲーム状態を格納する変数
	GameState mGameState;
	bool mIsComplete;

	// アクターが更新中かどうかの追跡 (true : あり / false : なし)
	bool mUpdatingActors;

	// UIのスプライト処理を行うためのスタック
	std::vector<class IUIScreen*> mUIStack;

	// HUDの補助を行うHUDHelperクラス
	HUDHelper* mHUDHelper;

	// ベースとなるgui
	std::unique_ptr<tgui::Gui> mGui;
	std::unique_ptr<tgui::Theme> mTheme;
	std::unique_ptr<tgui::Font> mFontTGUI;

	// ゲーム特有のメンバ変数があれば追加
	sf::Vector2f mWindowSize;
	std::string mCurrentKey;	// 現在の盤面のインデックスを表す文字列
	std::vector<std::string> mFilenames;
	std::unordered_map<std::string, std::vector<std::string>> mBoardData;		// プレイヤーと荷物を含めた盤面情報
	std::unordered_map<std::string, std::vector<std::string>> mInitBoardData;	// 初期盤面
	std::vector<std::string> mBoardState;	// プレイヤーと荷物を除いた盤面情報
	std::vector<sf::Vector2i> mGoalPos;
	const sf::Vector2i mSizeMax{ 48, 48 };
	const sf::Vector2i mSizeMin{ 5, 5 };
	int mBaggageLimit;
	const double mWallRateLimit = 0.5;
	const double mVisitedRateLimit = 1.0;
	const std::pair<int, int> mEvaluateFancIndexRange = std::pair<int, int>{ 0, 6 };
	std::chrono::system_clock::time_point mStart;

	// プレイヤーと荷物の初期位置
	sf::Vector2i mInitialPlayerPos;
	std::unordered_map<Baggage*, sf::Vector2i> mInitialBaggagePos;

	// 盤面の描画範囲
	BoundingBox mBoardViewArea;

	// 盤面の基礎的情報
	// レベル生成時のパラメータ管理もする
	GenerationParameters mGenParams;

	// 各キーの押下時間を記録するマップ
	// 長押し中の次の入力までの時間を計るタイマー
	std::map<sf::Keyboard::Key, float> mKeyHeldDuration, mAutoRepeatTimer;

	// 単押しとみなす押下時間
	static constexpr float TAP_THRESHOLD = 0.05f;
	// 長押しと判定するまでの時間（秒）
	static constexpr float HOLD_THRESHOLD = 0.5f;
	// 長押し中の連続入力の間隔（秒）
	static constexpr float AUTO_REPEAT_INTERVAL = 0.1f;

	// 現在のターン数
	unsigned int mStep;

	// ログを座標のリストで表す
	// (プレイヤーの初期座標から1マス動くことで、ログが初めて追加される)
	// (初期状態から遷移できる状態は基本的に複数あるので、リスト形式)
	// (つまり、このリストに格納されるログは、プレイヤーの移動前の座標が必ず初期座標になる)
	std::vector<Log> mLogs;

	// 現在の盤面のスナップショットをリストで保持
	// 盤面が切り替わった場合は破棄
	std::vector<GameSnapshot> mSnapshots;
};