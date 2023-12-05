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
	// アクターのリストをコンポーネントに追加
	mOwner->AddComponent(this);

	// Gameクラスで盤面のスプライトを追加
	mOwner->GetGame()->AddSprite(this);
}

BaggageComponent::~BaggageComponent()
{
	mOwner->GetGame()->RemoveSprite(this);
	mOwner->RemoveComponent(this);
}

void BaggageComponent::Update(float deltaTime)
{
	// 盤面は時間経過で変化しない
}

void BaggageComponent::ProcessInput(const sf::Event::KeyEvent* keyState)
{
	// 盤面はキーボード操作で変化しない
}

void BaggageComponent::Draw(sf::RenderWindow* mWindow)
{

	if (mTexture)
	{
		// 荷物を盤面上の座標に配置
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
	// 幅と高さを設定
	mTexWidth = texture->getSize().x;
	mTexHeight = texture->getSize().y;
}