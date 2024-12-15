#pragma once

#include <functional>

#include "SFML/Window.hpp"
#include "Game.h"
#include "IUIScreen.h"
#include "Font.h"
#include "Button.h"

class PauseMenu : public IUIScreen
{
public:
	PauseMenu(class Game* game);
	~PauseMenu();

	// インターフェースからオーバーライドした関数
	void Update(float deltaTime) override;
	void Draw(sf::RenderWindow* mWindow) override;
	void ProcessInput(const sf::Event* event, const sf::Vector2i& mousePos) override;

	// タイトルの文字列を変更
	void SetTitle(const std::string& text, const sf::Color& color = sf::Color::White, int pointSize = 40);

	// 状態をクロージングにする
	void Close();

	// 縦に整列したボタンを追加するヘルパー関数
	void AddButton(const std::string& name, std::function<void()> onClick);

	// 状態を取得
	UIState GetState() override { return mState; }

private:
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

	// 各種ボタンの座標
	sf::Vector2f mTitlePos;
	sf::Vector2f mNextButtonPos; // 次に設置するボタンの位置
	sf::Vector2f mBGPos;

	// ボタンのリスト
	std::vector<Button*> mButtons;

	// 状態
	UIState mState;
};

