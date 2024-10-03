#include "HUD.h"

HUD::HUD(class Game* game)
	: mGame(game)
{
	mGame->PushUI(this);
}

HUD::~HUD()
{
	mGame = nullptr;
	delete mGame;
}

void HUD::Update(float deltaTime)
{

}

void HUD::Draw()
{

}

void HUD::ProcessInput(const sf::Event::KeyEvent* keyState)
{

}

void HUD::SetTitle(const std::string& text, const sf::Color& color = sf::Color::White, int pointSize = 40)
{

}

void HUD::DrawTexture(class sf::Texture* texture, const sf::Vector2i& offset = sf::Vector2i{ 0, 0 }, float scale = 1.0f)
{

}