#pragma once

#include "SFML/Graphics.hpp"

class BackGroundComponent
{
public:
	// �R���X�g���N�^
	// updateOrder���Ⴏ���,��ɕ`�悳���(��ʂ̌���ɗ���)
	BackGroundComponent(class BackGround* owner, int updateOrder = 100, int drawOrder = 100);
	// �f�X�g���N�^
	virtual ~BackGroundComponent();
	// �f���^�^�C���ł��̃R���|�[�l���g���X�V����
	void Update(float deltaTime);
	// ���̃R���|�[�l���g��Process Input
	void ProcessInput(const sf::Event::KeyEvent* keyState);

	void Draw(sf::RenderWindow* mWindow);
	void SetTexture(sf::Texture* texture);

	// �Q�b�^�[�ƃZ�b�^�[

	int GetUpdateOrder() const { return mUpdateOrder; }

	int GetDrawOrder() const { return mDrawOrder; }
	int GetTexHeight() const { return mTexHeight; }
	int GetTexWidth() const { return mTexWidth; }
private:
	// ���g��ێ����Ă���A�N�^�[(BackGround�N���X)
	class BackGround* mOwner;

	// ���̃R���|�[�l���g��Update order
	int mUpdateOrder;

	// �e�N�X�`���֘A�̃����o�ϐ�
	sf::Texture* mTexture;
	int mDrawOrder;
	int mTexWidth;
	int mTexHeight;
};