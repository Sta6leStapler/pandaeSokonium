#include "SpriteComponent.h"
#include "Game.h"

#include <iostream>

SpriteComponent::SpriteComponent(Mediator* owner, int updateOrder, int drawOrder)
	:mOwner(owner)
	, mUpdateOrder(updateOrder)
	, mTexture(nullptr)
	, mDrawOrder(drawOrder)
{
	// アクターのリストをコンポーネントに追加
	mOwner->AccessActor()->AddComponent(this);

	// Gameクラスで盤面のスプライトを追加
	mOwner->AccessActor()->GetGame()->AddSprite(this);
}

SpriteComponent::~SpriteComponent()
{
	mOwner->AccessActor()->RemoveComponent(this);
}

void SpriteComponent::Update(float deltaTime)
{
	// 盤面は時間経過で変化しない
}

void SpriteComponent::ProcessInput(const sf::Event::KeyEvent* keyState)
{
	// 盤面はキーボード操作で変化しない
}

void SpriteComponent::Draw(sf::RenderWindow* mWindow)
{

	if (mTexture)
	{
		// ウィンドウの中央に盤面のテクスチャを描画
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