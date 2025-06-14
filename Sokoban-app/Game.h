#pragma once

#include "SFML/Graphics.hpp"
#include <unordered_map>
#include <string>
#include <vector>
#include <chrono>

#include "Player.h"

#include "TGUI/TGUI.hpp"
#include "TGUI/Backend/SFML-Graphics.hpp"

class Player;

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

	// undo/redo処理
	void CallUndo();
	void CallRedo();
	void CallReset();
	void CallRedoAll();

	// 盤面をセーブ
	void CallSave();

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
	void ResetParameters();

	// ゲッターとセッター
	// ゲーム制御関連
	void SetState(const GameState& gameState) { mGameState = gameState; }
	sf::Vector2f GetWindowSize() const { return mWindowSize; }
	std::vector<std::string> GetFilenames() const { return mFilenames; }
	std::string GetCurrentKey() const { return mCurrentKey; }
	std::string GetFilename(unsigned int num) const { return mFilenames.at(num); }
	std::unordered_map<std::string, std::vector<std::string>> GetBoardData() const { return mBoardData; }
	sf::Vector2i GetBoardSize() const { return mBoardSize; }
	BoundingBox GetBoardViewArea() const { return mBoardViewArea; }
	unsigned int GetStep() const { return mStep; }
	double GetSecTime() const { return static_cast<double>(std::chrono::duration_cast<std::chrono::seconds>(std::chrono::system_clock::now() - mStart).count()); };
	double GetMSecTime() const { return static_cast<double>(std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::system_clock::now() - mStart).count()); };

	// テクスチャ関連
	sf::Texture* GetTexture(const std::string& fileName) const { return mTextures.at(fileName); }
	std::unordered_map<std::string, sf::Texture*> GetTextures() const { return mTextures; }

	// 盤面関連
	std::vector<class Baggage*>& GetBaggages() { return mBaggages; }
	std::vector<sf::Vector2i> GetBaggagesPos() const;
	class Player* GetPlayer() const { return mPlayer; }
	int GetBaggageNumLimit(const sf::Vector2i& size, const double& wallRate = 0.0) const;
	void SetCurrentKey(const std::string& key) { mCurrentKey = key; }
	std::vector<std::string> GetCurrentInitBoardData() const { return mInitBoardData.at(mCurrentKey); }
	std::vector<std::string> GetInitBoardData(const std::string& boardKey) const { return mInitBoardData.at(boardKey); }

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
	sf::Font mFont;
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

	// 盤面の基礎的情報
	sf::Vector2i mBoardSize;
	int mBaggageNum;
	int mRepetition01;			// 荷物とプレイヤーの配置のリセット回数
	int mRepetition02;			// 荷物の運搬回数
	double mRepetition03;		// あらかじめ設置しておく壁マスの割合
	double mRepetition04;		// 訪問済みにあらかじめ割り振る割合
	int mRepetition05;			// 評価関数のインデックス

	// ゲーム特有のメンバ変数があれば追加
	sf::Vector2f mWindowSize;
	std::string mCurrentKey;
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

	// 入力のクールダウン
	float mInputCooldown;

	// 現在のターン数
	unsigned int mStep;

	// ログを座標のリストで表す
	// (プレイヤーの初期座標から1マス動くことで、ログが初めて追加される)
	// (初期状態から遷移できる状態は基本的に複数あるので、リスト形式)
	// (つまり、このリストに格納されるログは、プレイヤーの移動前の座標が必ず初期座標になる)
	std::vector<Log> mLogs;
};