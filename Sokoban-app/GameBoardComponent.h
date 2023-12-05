#pragma once

#include "SFML/Graphics.hpp"

class GameBoardComponent
{
public:
	// �R���X�g���N�^
	// updateOrder���Ⴏ���,��ɕ`�悳���(��ʂ̌���ɗ���)
	GameBoardComponent(class GameBoard* owner, int updateOrder = 100, int drawOrder = 100);
	// �f�X�g���N�^
	virtual ~GameBoardComponent();
	// �f���^�^�C���ł��̃R���|�[�l���g���X�V����
	void Update(float deltaTime);
	// ���̃R���|�[�l���g��Process Input
	void ProcessInput(const sf::Event::KeyEvent* keyState);

	// �`��֐�
	void Draw(sf::RenderWindow* mWindow);
	// �e�N�X�`���̐ݒ�
	void SetTexture(sf::Texture* texture);

	// �Q�b�^�[�ƃZ�b�^�[

	int GetUpdateOrder() const { return mUpdateOrder; }
	
	int GetDrawOrder() const { return mDrawOrder; }

private:
	// ���g��ێ����Ă���A�N�^�[(GameBoard�N���X)
	class GameBoard* mOwner;
	
	// ���̃R���|�[�l���g��Update order
	int mUpdateOrder;

	// �e�N�X�`���֘A�̃����o�ϐ�
	sf::Texture* mTexture;
	int mDrawOrder;
};