#include "GameBoardComponent.h"
#include "GameBoard.h"
#include "Game.h"

#include <iostream>

GameBoardComponent::GameBoardComponent(GameBoard* owner, int updateOrder, int drawOrder)
	:mOwner(owner)
	, mUpdateOrder(updateOrder)
	, mTexture(nullptr)
	, mDrawOrder(drawOrder)
{
	// �A�N�^�[�̃��X�g���R���|�[�l���g�ɒǉ�
	mOwner->AddComponent(this);

	// Game�N���X�ŔՖʂ̃X�v���C�g��ǉ�
	mOwner->GetGame()->AddSprite(this);
}

GameBoardComponent::~GameBoardComponent()
{
	mOwner->GetGame()->RemoveSprite(this);
	mOwner->RemoveComponent(this);
}

void GameBoardComponent::Update(float deltaTime)
{
	// �Ֆʂ͎��Ԍo�߂ŕω����Ȃ�
}

void GameBoardComponent::ProcessInput(const sf::Event::KeyEvent* keyState)
{
	// �Ֆʂ̓L�[�{�[�h����ŕω����Ȃ�
}

void GameBoardComponent::Draw(sf::RenderWindow* mWindow)
{

	if (mTexture)
	{
		// �E�B���h�E�̒����ɔՖʂ̃e�N�X�`����`��
		sf::Sprite spr;
		spr.setTexture(*mTexture);

		// 
		spr.setScale(mOwner->GetScale().x, mOwner->GetScale().y);
		spr.setPosition(mOwner->GetPosition().x, mOwner->GetPosition().y);
		
		mWindow->draw(spr);
	}
}

void GameBoardComponent::SetTexture(sf::Texture* texture)
{
	mTexture = texture;
}