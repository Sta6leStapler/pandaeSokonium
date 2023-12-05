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
	// アクターのリストをコンポーネントに追加
	mOwner->AddComponent(this);

	// Gameクラスで盤面のスプライトを追加
	mOwner->GetGame()->AddSprite(this);
}

BackGroundComponent::~BackGroundComponent()
{
	mOwner->GetGame()->RemoveSprite(this);
	mOwner->RemoveComponent(this);
}

void BackGroundComponent::Update(float deltaTime)
{
	// 盤面は時間経過で変化しない
}

void BackGroundComponent::ProcessInput(const sf::Event::KeyEvent* keyState)
{
	// 盤面はキーボード操作で変化しない
}

void BackGroundComponent::Draw(sf::RenderWindow* mWindow)
{

	if (mTexture)
	{
		// ウィンドウの中央に盤面のテクスチャを描画
		sf::Sprite spr;
		spr.setTexture(*mTexture);

		// スプライトの大きさと位置を調整
		// 背景の大きさを画面の大きさに揃える処理
		mOwner->SetScale(sf::Vector2f(mWindow->getSize().x / (float)mTexWidth, mWindow->getSize().y / (float)mTexHeight));

		spr.setScale(mOwner->GetScale().x, mOwner->GetScale().y);
		spr.setPosition(mOwner->GetPosition().x - spr.getGlobalBounds().width / 2.0f, mOwner->GetPosition().y - spr.getGlobalBounds().height / 2.0f);

		mWindow->draw(spr);
	}
}

void BackGroundComponent::SetTexture(sf::Texture* texture)
{
	mTexture = texture;
	// 幅と高さを設定
	mTexWidth = texture->getSize().x;
	mTexHeight = texture->getSize().y;
}