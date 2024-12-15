#pragma once

#include "SFML/Graphics.hpp"
#include <unordered_map>
#include <string>
#include <vector>
#include <chrono>

#include "IActor.h"
#include "IComponent.h"
#include "SpriteComponent.h"

#include "GameBoard.h"
#include "GameBoardComponent.h"
#include "BackGround.h"
#include "BackGroundComponent.h"
#include "Player.h"
#include "PlayerComponent.h"
#include "Baggage.h"
#include "BaggageComponent.h"
#include "IUIScreen.h"
#include "PauseMenu.h"
#include "HUD.h"
#include "THUD.h"
#include "HUDHelper.h"
#include "TGUI/TGUI.hpp"
#include "TGUI/Backend/SFML-Graphics.hpp"
#include "MySolution.h"

// 一動作のログ
struct Log
{
	std::pair<sf::Vector2i, sf::Vector2i> pCoordinate, bCoordinate;
	bool isBMoved;
	Player::Direction direction1, direction2;
	std::string time;
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
	
	void AddActor(class GameBoard* gameboard);
	void RemoveActor(class GameBoard* gameboard);
	void AddActor(class BackGround* gameboard);
	void RemoveActor(class BackGround* gameboard);
	void AddActor(class Player* player);
	void RemoveActor(class Player* player);
	void AddActor(class Baggage* baggage);
	void RemoveActor(class Baggage* baggage);

	// スプライトの追加と削除
	void AddSprite(class GameBoardComponent* GBComp);
	void RemoveSprite(class GameBoardComponent* GBcomp);
	void AddSprite(class BackGroundComponent* GBComp);
	void RemoveSprite(class BackGroundComponent* GBcomp);
	void AddSprite(class PlayerComponent* PComp);
	void RemoveSprite(class PlayerComponent* PComp);
	void AddSprite(class BaggageComponent* BComp);
	void RemoveSprite(class BaggageComponent* BComp);

	// UI画面のスタックに関する処理
	// スタック全体を参照で返す
	const std::vector<class IUIScreen*>& GetUIStack() { return mUIStack; }
	// 指定のUIScreenをスタックにプッシュする
	void PushUI(class IUIScreen* screen) { mUIStack.emplace_back(screen); }

	// ゲーム特有のメンバ関数があれば追加
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
	std::string ConvertLogToStr(const std::vector<Log>& logs, const unsigned long long& current);

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
	double GetSecTime() const { return std::chrono::duration_cast<std::chrono::seconds>(std::chrono::system_clock::now() - mStart).count(); };
	double GetMSecTime() const { return std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::system_clock::now() - mStart).count(); };

	// 盤面関連
	std::vector<class Baggage*>& GetBaggages() { return mBaggages; }
	std::vector<sf::Vector2i> GetBaggagesPos() const;
	class Player* GetPlayer() const { return mPlayer; }
	int GetBaggageNumLimit(const sf::Vector2i& size, const double& wallRate = 0.0) const;

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

	// ゲームのすべてのアクター
	class GameBoard* mGameBoard;
	class BackGround* mBackGround;
	class Player* mPlayer;
	std::vector<class Baggage*> mBaggages;
	// 待機中のアクター
	class GameBoard* mPendingGameBoard;
	class BackGround* mPendingBackGround;
	class Player* mPendingPlayer;
	std::vector<class Baggage*> mPendingBaggages;

	// 描画を行うコンポーネント
	GameBoardComponent* mGameBoardComponent;
	BackGroundComponent* mBackGroundComponent;
	PlayerComponent* mPlayerComponent;
	std::vector<BaggageComponent*> mBaggageComponents;

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
	std::vector<Log> mLogs;
};