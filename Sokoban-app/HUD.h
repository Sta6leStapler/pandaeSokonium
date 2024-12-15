#pragma once

#include <functional>

#include "SFML/Window.hpp"
#include "Game.h"
#include "IUIScreen.h"
#include "Font.h"
#include "Button.h"

class HUD : public IUIScreen
{
public:
	HUD(class Game* game);
	~HUD();

	// インターフェースからオーバーライドした関数
	void Update(float deltaTime) override;
	void Draw(sf::RenderWindow* mWindow) override;
	void ProcessInput(const sf::Event* event, const sf::Vector2i& mousePos) override;

	// タイトルの文字列を変更
	void SetTitle(const std::string& text, const sf::Color& color = sf::Color::White, int pointSize = 40);

	// 状態をクロージングにする
	void Close() { mState = UIState::EClosing; }

	// 任意の座標にボタンを追加するヘルパー関数
	void AddButton(const std::string& name, const sf::Vector2f& button_pos, std::function<void()> onClick, sf::Texture* iconTexture, int pointSize = 30, const sf::Color& color = sf::Color::White);

	// 状態を取得
	UIState GetState() override { return mState; }

private:
	// テキストウィンドウにテキストを追加するヘルパー関数
	void AddText(sf::Text* text, const std::string& textStr, const sf::Vector2f& pos, const unsigned int& charSize = 20, const sf::Color& color = sf::Color::Black);

	// テクスチャを描画するヘルパー関数
	void DrawTexture(sf::RenderWindow* mWindow, class sf::Texture* texture, const sf::Vector2f& offset = sf::Vector2f{ 0.0f, 0.0f }, float scale = 1.0f);

	class Game* mGame;

	// UI画面のタイトル文字列用
	class Font* mFont;
	class sf::Texture* mTitle;

	// ボタンのテクスチャ
	class sf::Texture* mBackground;
	class sf::Texture* mButtonOn;
	class sf::Texture* mButtonOff;
	class sf::Texture* mUndoTex;
	class sf::Texture* mRedoTex;
	class sf::Texture* mResetTex;
	class sf::Texture* mRedoAllTex;
	class sf::Texture* mSaveBoardTex;
	class sf::Texture* mSaveLogTex;
	class sf::Texture* mLoadBoardTex;
	class sf::Texture* mGenerateBoardTex;
	class sf::Texture* mHelpTex;

	// 各種ボタンの座標
	sf::Vector2f mTitlePos;
	sf::Vector2f mUndoButtonPos;
	sf::Vector2f mRedoButtonPos;
	sf::Vector2f mResetButtonPos;
	sf::Vector2f mRedoAllButtonPos;
	sf::Vector2f mSaveBoardButtonPos;
	sf::Vector2f mSaveLogButtonPos;
	sf::Vector2f mHelpButtonPos;
	sf::Vector2f mLoadBoardButtonPos;
	sf::Vector2f mGenerateBoardButtonPos;
	sf::Vector2f mBGPos;

	// その他のテクスチャなど
	sf::RectangleShape* mTextInfoBox;
	sf::Text* mMoveCountText;
	sf::Text* mTimeText;
	sf::Text* mStateText;
	sf::Text* mMovableBaggagesText;
	sf::Text* mGoaledBaggagesText;
	sf::Text* mDeadlockedBaggagesText;

	// ゲームの情報とテキストのリスト
	std::vector<sf::Text*> mTextInfoes;

	// ボタンのリスト
	std::vector<std::pair<Button*, sf::Texture*>> mButtons;

	// 状態
	UIState mState;
};

