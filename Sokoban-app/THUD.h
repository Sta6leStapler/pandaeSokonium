#pragma once

#include "SFML/Graphics.hpp"
#include "TGUI/TGUI.hpp"
#include "TGUI/Backend/SFML-Graphics.hpp"

#include "IUIScreen.h"

class THUD : public IUIScreen
{
public:
	THUD(class Game* game, sf::RenderWindow* window);
	~THUD();

	enum TextIndex
	{
		EMoveCount,
		ETime,
		EBoardState,
		EMovableBaggages,
		EGoaledBaggages,
		EDeadlockedBaggages
	};

	// インターフェースからオーバーライドした関数
	void Update(float deltaTime) override;
	void Draw(sf::RenderWindow* window) override;
	void ProcessInput(const sf::Event* event, const sf::Vector2i& mousePos) override;

	// 状態をクロージングにする
	void Close() { mState = UIState::EClosing; }

	// 状態を取得
	UIState GetState() override { return mState; }
	
	// ボタンの長押しを解除するメンバ変数
	void CancelButtonHolds();

private:
	class Game* mGame;

	// tgui::Guiクラス
	std::unique_ptr<tgui::Gui> mGui;

	// テキストUI用の変数
	tgui::ListBox::Ptr mListBox;
	std::unordered_map<TextIndex, std::string> mTextInfo;

	// 各種制御用変数
	sf::Vector2i mRollBackLargeButtonSize;
	sf::Vector2i mRollBackSmallButtonSize;
	sf::Vector2i mGameSystemButtonSize;
	sf::Vector2i mButtonInitialPos;
	sf::Vector2i mButtonMergin;
	sf::Vector2i mListBoxSize;
	sf::Vector2i mListBoxPos;
	unsigned int mListBoxItemHeight;
	float mIconImageScale;

	tgui::Panel::Ptr mBlocker;

	// Undo/Redoボタンの制御
	bool mIsUndoHeld;   // Undoボタンが押され続けているか
	bool mIsRedoHeld;   // Redoボタンが押され続けているか
	float mUndoRepeatTimer; // Undoのリピートタイマー
	float mRedoRepeatTimer; // Redoのリピートタイマー

	// 状態
	UIState mState;
};

