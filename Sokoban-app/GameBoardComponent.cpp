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
	// アクターのリストをコンポーネントに追加
	mOwner->AddComponent(this);

	// Gameクラスで盤面のスプライトを追加
	mOwner->GetGame()->AddSprite(this);
}

GameBoardComponent::~GameBoardComponent()
{
	mOwner->GetGame()->RemoveSprite(this);
	mOwner->RemoveComponent(this);
}

void GameBoardComponent::Update(float deltaTime)
{
	// 盤面は時間経過で変化しない
}

void GameBoardComponent::ProcessInput(const sf::Event::KeyEvent* keyState)
{
	// 盤面はキーボード操作で変化しない
}

void GameBoardComponent::Draw(sf::RenderWindow* mWindow)
{

	if (mTexture)
	{
		// ウィンドウの中央に盤面のテクスチャを描画
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