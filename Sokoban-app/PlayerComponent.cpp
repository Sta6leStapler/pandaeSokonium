#include "PlayerComponent.h"
#include "Game.h"

PlayerComponent::PlayerComponent(Player* owner, int updateOrder, int drawOrder)
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

PlayerComponent::~PlayerComponent()
{
	mOwner->GetGame()->RemoveSprite(this);
	mOwner->RemoveComponent(this);
}

void PlayerComponent::Update(float deltaTime)
{
	
}

void PlayerComponent::ProcessInput(const sf::Event::KeyEvent* keyState)
{
	
}

void PlayerComponent::Draw(sf::RenderWindow* mWindow)
{

	if (mTexture)
	{
		// �v���C���[��Ֆʏ�̍��W�ɔz�u
		sf::Sprite spr;
		spr.setTexture(*mTexture);

		// 
		spr.setScale(mOwner->GetScale().x, mOwner->GetScale().y);
		spr.setPosition(mOwner->GetPosition().x, mOwner->GetPosition().y);

		mWindow->draw(spr);
	}
}

void PlayerComponent::SetTexture(sf::Texture* texture)
{
	mTexture = texture;
	// ���ƍ�����ݒ�
	mTexWidth = texture->getSize().x;
	mTexHeight = texture->getSize().y;
}