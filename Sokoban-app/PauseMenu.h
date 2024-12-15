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

	// �C���^�[�t�F�[�X����I�[�o�[���C�h�����֐�
	void Update(float deltaTime) override;
	void Draw(sf::RenderWindow* mWindow) override;
	void ProcessInput(const sf::Event* event, const sf::Vector2i& mousePos) override;

	// �^�C�g���̕������ύX
	void SetTitle(const std::string& text, const sf::Color& color = sf::Color::White, int pointSize = 40);

	// ��Ԃ��N���[�W���O�ɂ���
	void Close();

	// �c�ɐ��񂵂��{�^����ǉ�����w���p�[�֐�
	void AddButton(const std::string& name, std::function<void()> onClick);

	// ��Ԃ��擾
	UIState GetState() override { return mState; }

private:
	// �e�N�X�`����`�悷��w���p�[�֐�
	void DrawTexture(sf::RenderWindow* mWindow, class sf::Texture* texture, const sf::Vector2f& offset = sf::Vector2f{ 0.0f, 0.0f }, float scale = 1.0f);

	class Game* mGame;

	// UI��ʂ̃^�C�g��������p
	class Font* mFont;
	class sf::Texture* mTitle;

	// �{�^���̃e�N�X�`��
	class sf::Texture* mBackground;
	class sf::Texture* mButtonOn;
	class sf::Texture* mButtonOff;

	// �e��{�^���̍��W
	sf::Vector2f mTitlePos;
	sf::Vector2f mNextButtonPos; // ���ɐݒu����{�^���̈ʒu
	sf::Vector2f mBGPos;

	// �{�^���̃��X�g
	std::vector<Button*> mButtons;

	// ���
	UIState mState;
};

