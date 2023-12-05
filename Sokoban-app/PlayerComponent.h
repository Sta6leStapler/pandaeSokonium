#pragma once

#include "SFML/Graphics.hpp"

class PlayerComponent
{
public:
	// �R���X�g���N�^
	// updateOrder���Ⴏ���,��ɕ`�悳���(��ʂ̌���ɗ���)
	PlayerComponent(class Player* owner, int updateOrder = 100, int drawOrder = 100);
	// �f�X�g���N�^
	virtual ~PlayerComponent();
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
	int GetTexHeight() const { return mTexHeight; }
	int GetTexWidth() const { return mTexWidth; }
	int GetCoordinateX() const { return mCoordinateX; }
	void SetCoordinateX(const int& coordinateX) { this->mCoordinateX = coordinateX; }
	int GetCoordinateY() const { return mCoordinateY; }
	void SetCoordinateY(const int& coordinateY) { this->mCoordinateY = coordinateY; }

private:
	// ���g��ێ����Ă���A�N�^�[(Player�N���X)
	class Player* mOwner;

	// ���̃R���|�[�l���g��Update order
	int mUpdateOrder;

	// �e�N�X�`���֘A�̃����o�ϐ�
	sf::Texture* mTexture;
	int mDrawOrder;
	int mTexWidth;
	int mTexHeight;
	int mCoordinateX;
	int mCoordinateY;
};