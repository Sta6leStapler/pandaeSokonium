#include "SpriteComponent.h"

SpriteComponent::SpriteComponent(IActor* owner, int updateOrder, int drawOrder)
	: mOwner(owner)
	, mUpdateOrder(updateOrder)
	, mDrawOrder(drawOrder)
	, mTexture(nullptr)
	, mBoundingBox(sf::Vector2u{ 0, 0 })
	, mCoordinate(sf::Vector2i{ 0, 0 })
{
	// アクターのコンポーネントのリストに追加
	mOwner->AddComponent(this);

	// Gameクラスで盤面のスプライトを追加
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
		// テクスチャからスプライトを作成
		sf::Sprite spr;
		spr.setTexture(*mTexture);

		// アクター側に設定されているスケーリングとオフセットを調整
		spr.setScale(mOwner->GetScale().x, mOwner->GetScale().y);
		spr.setPosition(mOwner->GetPosition().x, mOwner->GetPosition().y);

		mWindow->draw(spr);
	}
}

void SpriteComponent::SetTexture(sf::Texture* texture)
{
	mTexture = texture;
	// 幅と高さを設定
	mBoundingBox = texture->getSize();
}