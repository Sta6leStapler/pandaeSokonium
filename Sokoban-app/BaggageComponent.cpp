#include "BaggageComponent.h"
#include "Game.h"

BaggageComponent::BaggageComponent(Baggage* owner, int updateOrder, int drawOrder)
	:mOwner(owner)
	, mUpdateOrder(updateOrder)
	, mTexture(nullptr)
	, mDrawOrder(drawOrder)
	, mTexWidth(0)
	, mTexHeight(0)
	, mCoordinateX(0)
	, mCoordinateY(0)
{
	// �A�N�^�[�̃��X�g���R���|�[�l���g�ɒǉ�
	mOwner->AddComponent(this);

	// Game�N���X�ŔՖʂ̃X�v���C�g��ǉ�
	mOwner->GetGame()->AddSprite(this);
}

BaggageComponent::~BaggageComponent()
{
	mOwner->GetGame()->RemoveSprite(this);
	mOwner->RemoveComponent(this);
}

void BaggageComponent::Update(float deltaTime)
{
	// �Ֆʂ͎��Ԍo�߂ŕω����Ȃ�
}

void BaggageComponent::ProcessInput(const sf::Event::KeyEvent* keyState)
{
	// �Ֆʂ̓L�[�{�[�h����ŕω����Ȃ�
}

void BaggageComponent::Draw(sf::RenderWindow* mWindow)
{

	if (mTexture)
	{
		// �ו���Ֆʏ�̍��W�ɔz�u
		sf::Sprite spr;
		spr.setTexture(*mTexture);

		// 
		spr.setScale(mOwner->GetScale().x, mOwner->GetScale().y);
		spr.setPosition(mOwner->GetPosition().x, mOwner->GetPosition().y);

		mWindow->draw(spr);
	}
}

void BaggageComponent::SetTexture(sf::Texture* texture)
{
	mTexture = texture;
	// ���ƍ�����ݒ�
	mTexWidth = texture->getSize().x;
	mTexHeight = texture->getSize().y;
}