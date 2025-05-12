#include "SpriteComponent.h"

SpriteComponent::SpriteComponent(IActor* owner, int updateOrder, int drawOrder)
	: mOwner(owner)
	, mUpdateOrder(updateOrder)
	, mDrawOrder(drawOrder)
	, mTexture(nullptr)
	, mBoundingBox(sf::Vector2u{ 0, 0 })
	, mCoordinate(sf::Vector2i{ 0, 0 })
{
	// �A�N�^�[�̃R���|�[�l���g�̃��X�g�ɒǉ�
	mOwner->AddComponent(this);

	// Game�N���X�ŔՖʂ̃X�v���C�g��ǉ�
	mOwner->GetGame()->AddSprite(this);
}

SpriteComponent::~SpriteComponent()
{
	mOwner->GetGame()->RemoveSprite(this);
	mOwner->RemoveComponent(this);
}

void SpriteComponent::Update(float deltaTime)
{

}

void SpriteComponent::Draw(sf::RenderWindow* mWindow)
{
	if (mTexture)
	{
		// �e�N�X�`������X�v���C�g���쐬
		sf::Sprite spr;
		spr.setTexture(*mTexture);

		// �A�N�^�[���ɐݒ肳��Ă���X�P�[�����O�ƃI�t�Z�b�g�𒲐�
		spr.setScale(mOwner->GetScale().x, mOwner->GetScale().y);
		spr.setPosition(mOwner->GetPosition().x, mOwner->GetPosition().y);

		mWindow->draw(spr);
	}
}

void SpriteComponent::SetTexture(sf::Texture* texture)
{
	mTexture = texture;
	// ���ƍ�����ݒ�
	mBoundingBox = texture->getSize();
}