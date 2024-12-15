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

	// �C���^�[�t�F�[�X����I�[�o�[���C�h�����֐�
	void Update(float deltaTime) override;
	void Draw(sf::RenderWindow* mWindow) override;
	void ProcessInput(const sf::Event* event, const sf::Vector2i& mousePos) override;

	// �^�C�g���̕������ύX
	void SetTitle(const std::string& text, const sf::Color& color = sf::Color::White, int pointSize = 40);

	// ��Ԃ��N���[�W���O�ɂ���
	void Close() { mState = UIState::EClosing; }

	// �C�ӂ̍��W�Ƀ{�^����ǉ�����w���p�[�֐�
	void AddButton(const std::string& name, const sf::Vector2f& button_pos, std::function<void()> onClick, sf::Texture* iconTexture, int pointSize = 30, const sf::Color& color = sf::Color::White);

	// ��Ԃ��擾
	UIState GetState() override { return mState; }

private:
	// �e�L�X�g�E�B���h�E�Ƀe�L�X�g��ǉ�����w���p�[�֐�
	void AddText(sf::Text* text, const std::string& textStr, const sf::Vector2f& pos, const unsigned int& charSize = 20, const sf::Color& color = sf::Color::Black);

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
	class sf::Texture* mUndoTex;
	class sf::Texture* mRedoTex;
	class sf::Texture* mResetTex;
	class sf::Texture* mRedoAllTex;
	class sf::Texture* mSaveBoardTex;
	class sf::Texture* mSaveLogTex;
	class sf::Texture* mLoadBoardTex;
	class sf::Texture* mGenerateBoardTex;
	class sf::Texture* mHelpTex;

	// �e��{�^���̍��W
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

	// ���̑��̃e�N�X�`���Ȃ�
	sf::RectangleShape* mTextInfoBox;
	sf::Text* mMoveCountText;
	sf::Text* mTimeText;
	sf::Text* mStateText;
	sf::Text* mMovableBaggagesText;
	sf::Text* mGoaledBaggagesText;
	sf::Text* mDeadlockedBaggagesText;

	// �Q�[���̏��ƃe�L�X�g�̃��X�g
	std::vector<sf::Text*> mTextInfoes;

	// �{�^���̃��X�g
	std::vector<std::pair<Button*, sf::Texture*>> mButtons;

	// ���
	UIState mState;
};

