#pragma once

#include "SFML/Graphics.hpp"

class PlayerComponent
{
public:
	// コンストラクタ
	// updateOrderが低ければ,先に描画される(画面の後方に来る)
	PlayerComponent(class Player* owner, int updateOrder = 100, int drawOrder = 100);
	// デストラクタ
	virtual ~PlayerComponent();
	// デルタタイムでこのコンポーネントを更新する
	void Update(float deltaTime);
	// このコンポーネントのProcess Input
	void ProcessInput(const sf::Event::KeyEvent* keyState);

	// 描画関数
	void Draw(sf::RenderWindow* mWindow);
	// テクスチャの設定
	void SetTexture(sf::Texture* texture);

	// ゲッターとセッター

	int GetUpdateOrder() const { return mUpdateOrder; }

	int GetDrawOrder() const { return mDrawOrder; }
	int GetTexHeight() const { return mTexHeight; }
	int GetTexWidth() const { return mTexWidth; }
	int GetCoordinateX() const { return mCoordinateX; }
	void SetCoordinateX(const int& coordinateX) { this->mCoordinateX = coordinateX; }
	int GetCoordinateY() const { return mCoordinateY; }
	void SetCoordinateY(const int& coordinateY) { this->mCoordinateY = coordinateY; }

private:
	// 自身を保持しているアクター(Playerクラス)
	class Player* mOwner;

	// このコンポーネントのUpdate order
	int mUpdateOrder;

	// テクスチャ関連のメンバ変数
	sf::Texture* mTexture;
	int mDrawOrder;
	int mTexWidth;
	int mTexHeight;
	int mCoordinateX;
	int mCoordinateY;
};