#pragma once

#include "IComponent.h"
#include "IActor.h"
#include "Game.h"

class SpriteComponent : public IComponent
{
public:
	// コンストラクタ
	// updateOrderが低ければ,先に描画される(画面の後方に来る)
	SpriteComponent(class IActor* owner, int updateOrder = 100, int drawOrder = 100);
	// デストラクタ
	~SpriteComponent();

	// デルタタイムでこのコンポーネントを更新する
	void Update(float deltaTime) override;

	// このコンポーネント特有の入力処理(SpriteComponentでは何もしない)
	void ProcessInput(const sf::Event* event) override {}

	// 描画関数
	void Draw(sf::RenderWindow* mWindow);

	// テクスチャの設定
	void SetTexture(sf::Texture* texture);

	// ゲッターとセッター
	int GetUpdateOrder() override { return mUpdateOrder; }

	int GetDrawOrder() const { return mDrawOrder; }
	sf::Vector2u GetBoundingBox() const { return mBoundingBox; }
	sf::Vector2i GetCoordinate() const { return mCoordinate; }
	void SetCoordinateX(const int& coordinateX) { this->mCoordinate.x = coordinateX; }
	void SetCoordinateY(const int& coordinateY) { this->mCoordinate.y = coordinateY; }
	void SetCoordinate(const sf::Vector2i& coordinate) { this->mCoordinate = coordinate; }

private:
	// 自身を保持しているアクター
	class IActor* mOwner;

	// このコンポーネントのUpdate order
	int mUpdateOrder;

	// テクスチャ関連のメンバ変数
	// SpriteComponentは単一のテクスチャのみを持ち、
	// アクターからの呼び出しによってテクスチャを適宜変更する
	// つまり、ゲーム内に描画するスプライトの数だけSpriteComponentは宣言される
	sf::Texture* mTexture;
	int mDrawOrder;
	sf::Vector2u mBoundingBox;
	sf::Vector2i mCoordinate;
};

