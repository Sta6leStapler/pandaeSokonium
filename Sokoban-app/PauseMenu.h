#pragma once

#include "Game.h"
#include "IUIScreen.h"
#include "Font.h"

class PauseMenu : public IUIScreen
{
public:
	PauseMenu(class Game* game);
	~PauseMenu();

	// �C���^�[�t�F�[�X����I�[�o�[���C�h�����֐�
	void Update(float deltaTime) override;
	void Draw() override;
	void ProcessInput(const sf::Event::KeyEvent* keyState) override;

	// �^�C�g���̕������ύX
	void SetTitle(const std::string& text, const sf::Color& color = sf::Color::White, int pointSize = 40);

	// ��Ԃ��N���[�W���O�ɂ���
	void Close() { mState = UIState::EClosing; }

	// ��Ԃ��擾
	UIState GetState() const { return mState; }

private:
	// �e�N�X�`����`�悷��w���p�[�֐�
	void DrawTexture(class sf::Texture* texture, const sf::Vector2i& offset = sf::Vector2i{ 0, 0 }, float scale = 1.0f);

	class Game* mGame;

	// UI��ʂ̃^�C�g��������p
	class Font* mFont;
	class sf::Texture mTitle;
	sf::Vector2i mTitlePos;

	// ���
	UIState mState;
};

