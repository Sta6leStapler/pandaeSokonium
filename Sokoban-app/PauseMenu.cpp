#include "PauseMenu.h"

PauseMenu::PauseMenu(class Game* game)
	: mGame(game)
{
	mGame->PushUI(this);
	// �|�[�Y���j���[�̓Q�[�����[�v�Ɋ��荞�ޏ����Ȃ̂ŁA
	// �|�[�Y���j���[�̋N�����̂݃C���X�^���X�����݂���悤�ɂ���
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
	// Esc�L�[�������ꂽ��|�[�Y���j���[�����
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