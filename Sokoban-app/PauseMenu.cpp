#include "PauseMenu.h"

PauseMenu::PauseMenu(class Game* game)
	: mGame(game)
{
	mGame->PushUI(this);
	// ポーズメニューはゲームループに割り込む処理なので、
	// ポーズメニューの起動中のみインスタンスが存在するようにする
	mGame->SetState(Game::GameState::EPaused);
	SetTitle("PAUSED");
}

PauseMenu::~PauseMenu()
{
	mGame->SetState(Game::GameState::EGamePlay);
}

void PauseMenu::Update(float deltaTime)
{

}

void PauseMenu::Draw()
{

}

void PauseMenu::ProcessInput(const sf::Event::KeyEvent* keyState)
{
	// Escキーが押されたらポーズメニューを閉じる
	switch (keyState->code)
	{
	case sf::Keyboard::Escape:
		Close();
		break;
	default:
		break;
	}
}

void PauseMenu::SetTitle(const std::string& text, const sf::Color& color = sf::Color::White, int pointSize = 40)
{

}

void PauseMenu::DrawTexture(class sf::Texture* texture, const sf::Vector2i& offset = sf::Vector2i{ 0, 0 }, float scale = 1.0f)
{

}