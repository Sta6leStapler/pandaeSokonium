#pragma once

#include "SFML/Graphics.hpp"
#include "TGUI/TGUI.hpp"
#include "TGUI/Backend/SFML-Graphics.hpp"

#include "IUIScreen.h"

class THUD : public IUIScreen
{
public:
	THUD(class Game* game, sf::RenderWindow* window);
	~THUD();

	enum TextIndex
	{
		EMoveCount,
		ETime,
		EBoardState,
		EMovableBaggages,
		EGoaledBaggages,
		EDeadlockedBaggages
	};

	// �C���^�[�t�F�[�X����I�[�o�[���C�h�����֐�
	void Update(float deltaTime) override;
	void Draw(sf::RenderWindow* window) override;
	void ProcessInput(const sf::Event* event, const sf::Vector2i& mousePos) override;

	// ��Ԃ��N���[�W���O�ɂ���
	void Close() { mState = UIState::EClosing; }

	// ��Ԃ��擾
	UIState GetState() override { return mState; }

private:
	class Game* mGame;

	// tgui::Gui�N���X
	std::unique_ptr<tgui::Gui> mGui;

	// UI�̃e�[�}
	std::unique_ptr<tgui::Theme> mTheme;

	// �e�L�X�gUI�p�̕ϐ�
	tgui::ListBox::Ptr mListBox;
	std::unordered_map<TextIndex, std::string> mTextInfo;

	// �e�퐧��p�ϐ�
	sf::Vector2i mButtonSize;
	sf::Vector2i mButtonInitialPos;
	sf::Vector2i mButtonMergin;
	sf::Vector2i mListBoxSize;
	sf::Vector2i mListBoxPos;
	int mListBoxItemHeight;

	// ���
	UIState mState;
};

