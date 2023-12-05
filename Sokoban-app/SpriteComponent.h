#pragma once

class SpriteComponent
{
public:
	// �R���X�g���N�^
	// updateOrder���Ⴏ���,��ɕ`�悳���(��ʂ̌���ɗ���)
	SpriteComponent(class Mediator* owner, int updateOrder = 100, int drawOrder = 100);
	// �f�X�g���N�^
	virtual ~SpriteComponent();

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
	// ���g��ێ����Ă���A�N�^�[�𒇉��N���X
	class Mediator* mOwner;

	// ���̃R���|�[�l���g��Update order
	int mUpdateOrder;

	// �e�N�X�`���֘A�̃����o�ϐ�
	sf::Texture* mTexture;
	int mDrawOrder;
};

