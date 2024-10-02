#pragma once

#include "Game.h"
#include "IUIScreen.h"
#include "Font.h"

class HUD : public IUIScreen
{
public:
	HUD(class Game* game);
	~HUD();

	// インターフェースからオーバーライドした関数
	void Update(float deltaTime) override;
	void Draw() override;
	void ProcessInput(const sf::Event::KeyEvent* keyState) override;

	// 状態がアクティブかクロージングか管理
	enum UIState { EActive, EClosing };
	
	// 状態をクロージングにする
	void Close();

	// 状態を取得
	UIState GetState() const { return mState; }

	// タイトルの文字列を変更
	void SetTitle(const std::string& text, const sf::Color& color = sf::Color::White, int pointSize = 40);
private:
	// テクスチャを描画するヘルパー関数
	void DrawTexture(class sf::Texture* texture, const sf::Vector2i& offset = sf::Vector2i{ 0, 0 }, float scale = 1.0f);

	class Game* mGame;

	// UI画面のタイトル文字列用
	class Font* mFont;
	class sf::Texture mTitle;
	sf::Vector2i mTitlePos;

	// 状態
	UIState mState;
};

