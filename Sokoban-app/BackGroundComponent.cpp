#include "BackGroundComponent.h"
#include "BackGround.h"
#include "Game.h"

BackGroundComponent::BackGroundComponent(BackGround* owner, int updateOrder, int drawOrder)
	:mOwner(owner)
	, mUpdateOrder(updateOrder)
	, mTexture(nullptr)
	, mDrawOrder(drawOrder)
	, mTexWidth(0)
	, mTexHeight(0)
{
	// �A�N�^�[�̃��X�g���R���|�[�l���g�ɒǉ�
	mOwner->AddComponent(this);

	// Game�N���X�ŔՖʂ̃X�v���C�g��ǉ�
	mOwner->GetGame()->AddSprite(this);
}

BackGroundComponent::~BackGroundComponent()
{
	mOwner->GetGame()->RemoveSprite(this);
	mOwner->RemoveComponent(this);
}

void BackGroundComponent::Update(float deltaTime)
{
	// �Ֆʂ͎��Ԍo�߂ŕω����Ȃ�
}

void BackGroundComponent::ProcessInput(const sf::Event::KeyEvent* keyState)
{
	// �Ֆʂ̓L�[�{�[�h����ŕω����Ȃ�
}

void BackGroundComponent::Draw(sf::RenderWindow* mWindow)
{

	if (mTexture)
	{
		// �E�B���h�E�̒����ɔՖʂ̃e�N�X�`����`��
		sf::Sprite spr;
		spr.setTexture(*mTexture);

		// �X�v���C�g�̑傫���ƈʒu�𒲐�
		// �w�i�̑傫������ʂ̑傫���ɑ����鏈��
		mOwner->SetScale(sf::Vector2f(mWindow->getSize().x / (float)mTexWidth, mWindow->getSize().y / (float)mTexHeight));

		spr.setScale(mOwner->GetScale().x, mOwner->GetScale().y);
		spr.setPosition(mOwner->GetPosition().x - spr.getGlobalBounds().width / 2.0f, mOwner->GetPosition().y - spr.getGlobalBounds().height / 2.0f);

		mWindow->draw(spr);
	}
}

void BackGroundComponent::SetTexture(sf::Texture* texture)
{
	mTexture = texture;
	// ���ƍ�����ݒ�
	mTexWidth = texture->getSize().x;
	mTexHeight = texture->getSize().y;
}