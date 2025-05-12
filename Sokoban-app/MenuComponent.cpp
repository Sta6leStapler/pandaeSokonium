#include "MenuComponent.h"
#include "Game.h"

MenuComponent::MenuComponent(Menu* owner, int updateOrder, int drawOrder)
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

	// Gameクラスでメニューのスプライトを追加
	mOwner->GetGame()->AddSprite(this);
}

MenuComponent::~MenuComponent()
{
	mOwner->GetGame()->RemoveSprite(this);
	mOwner->RemoveComponent(this);
}

void MenuComponent::Update(float deltaTime)
{

}

void MenuComponent::ProcessInput(const sf::Event::KeyEvent* keyState)
{

}

void MenuComponent::Draw(sf::RenderWindow* mWindow)
{

	if (mTexture)
	{
		// プレイヤーを盤面上の座標に配置
		sf::Sprite spr;
		spr.setTexture(*mTexture);

		// 
		spr.setScale(mOwner->GetScale().x, mOwner->GetScale().y);
		spr.setPosition(mOwner->GetPosition().x, mOwner->GetPosition().y);

		mWindow->draw(spr);
	}
}

void MenuComponent::SetTexture(sf::Texture* texture)
{
	mTexture = texture;
	// 幅と高さを設定
	mTexWidth = texture->getSize().x;
	mTexHeight = texture->getSize().y;
}