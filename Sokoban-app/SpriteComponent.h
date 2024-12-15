#pragma once

#include "IComponent.h"
#include "IActor.h"
#include "Game.h"

class SpriteComponent : public IComponent
{
public:
	// �R���X�g���N�^
	// updateOrder���Ⴏ���,��ɕ`�悳���(��ʂ̌���ɗ���)
	SpriteComponent(class IActor* owner, int updateOrder = 100, int drawOrder = 100);
	// �f�X�g���N�^
	~SpriteComponent();

	// �f���^�^�C���ł��̃R���|�[�l���g���X�V����
	void Update(float deltaTime) override;

	// ���̃R���|�[�l���g���L�̓��͏���(SpriteComponent�ł͉������Ȃ�)
	void ProcessInput(const sf::Event* event) override {}

	// �`��֐�
	void Draw(sf::RenderWindow* mWindow);

	// �e�N�X�`���̐ݒ�
	void SetTexture(sf::Texture* texture);

	// �Q�b�^�[�ƃZ�b�^�[
	int GetUpdateOrder() override { return mUpdateOrder; }

	int GetDrawOrder() const { return mDrawOrder; }
	sf::Vector2u GetBoundingBox() const { return mBoundingBox; }
	sf::Vector2i GetCoordinate() const { return mCoordinate; }
	void SetCoordinateX(const int& coordinateX) { this->mCoordinate.x = coordinateX; }
	void SetCoordinateY(const int& coordinateY) { this->mCoordinate.y = coordinateY; }
	void SetCoordinate(const sf::Vector2i& coordinate) { this->mCoordinate = coordinate; }

private:
	// ���g��ێ����Ă���A�N�^�[
	class IActor* mOwner;

	// ���̃R���|�[�l���g��Update order
	int mUpdateOrder;

	// �e�N�X�`���֘A�̃����o�ϐ�
	// SpriteComponent�͒P��̃e�N�X�`���݂̂������A
	// �A�N�^�[����̌Ăяo���ɂ���ăe�N�X�`����K�X�ύX����
	// �܂�A�Q�[�����ɕ`�悷��X�v���C�g�̐�����SpriteComponent�͐錾�����
	sf::Texture* mTexture;
	int mDrawOrder;
	sf::Vector2u mBoundingBox;
	sf::Vector2i mCoordinate;
};

