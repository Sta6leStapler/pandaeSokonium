#include "SpriteComponent.h"
#include "Game.h"

#include <iostream>

SpriteComponent::SpriteComponent(Mediator* owner, int updateOrder, int drawOrder)
	:mOwner(owner)
	, mUpdateOrder(updateOrder)
	, mTexture(nullptr)
	, mDrawOrder(drawOrder)
{
	// �A�N�^�[�̃��X�g���R���|�[�l���g�ɒǉ�
	mOwner->AccessActor()->AddComponent(this);

	// Game�N���X�ŔՖʂ̃X�v���C�g��ǉ�
	mOwner->AccessActor()->GetGame()->AddSprite(this);
}

SpriteComponent::~SpriteComponent()
{
	mOwner->AccessActor()->RemoveComponent(this);
}

void SpriteComponent::Update(float deltaTime)
{
	// �Ֆʂ͎��Ԍo�߂ŕω����Ȃ�
}

void SpriteComponent::ProcessInput(const sf::Event::KeyEvent* keyState)
{
	// �Ֆʂ̓L�[�{�[�h����ŕω����Ȃ�
}

void SpriteComponent::Draw(sf::RenderWindow* mWindow)
{

	if (mTexture)
	{
		// �E�B���h�E�̒����ɔՖʂ̃e�N�X�`����`��
		sf::Sprite spr;
		spr.setTexture(*mTexture);

		// 
		spr.setScale(mOwner->AccessActor()->GetScale().x, mOwner->AccessActor()->GetScale().y);
		spr.setPosition(mOwner->AccessActor()->GetPosition().x, mOwner->AccessActor()->GetPosition().y);

		mWindow->draw(spr);
	}
}

void SpriteComponent::SetTexture(sf::Texture* texture)
{
	mTexture = texture;
}