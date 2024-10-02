#pragma once

#include "Game.h"
#include "IUIScreen.h"
#include "Font.h"

class HUD : public IUIScreen
{
public:
	HUD(class Game* game);
	~HUD();

	// �C���^�[�t�F�[�X����I�[�o�[���C�h�����֐�
	void Update(float deltaTime) override;
	void Draw() override;
	void ProcessInput(const sf::Event::KeyEvent* keyState) override;

	// ��Ԃ��A�N�e�B�u���N���[�W���O���Ǘ�
	enum UIState { EActive, EClosing };
	
	// ��Ԃ��N���[�W���O�ɂ���
	void Close();

	// ��Ԃ��擾
	UIState GetState() const { return mState; }

	// �^�C�g���̕������ύX
	void SetTitle(const std::string& text, const sf::Color& color = sf::Color::White, int pointSize = 40);
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

