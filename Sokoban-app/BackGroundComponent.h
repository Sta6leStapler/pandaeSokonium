#pragma once

#include "SFML/Graphics.hpp"

class BackGroundComponent
{
public:
	// コンストラクタ
	// updateOrderが低ければ,先に描画される(画面の後方に来る)
	BackGroundComponent(class BackGround* owner, int updateOrder = 100, int drawOrder = 100);
	// デストラクタ
	virtual ~BackGroundComponent();
	// デルタタイムでこのコンポーネントを更新する
	void Update(float deltaTime);
	// このコンポーネントのProcess Input
	void ProcessInput(const sf::Event::KeyEvent* keyState);

	void Draw(sf::RenderWindow* mWindow);
	void SetTexture(sf::Texture* texture);

	// ゲッターとセッター

	int GetUpdateOrder() const { return mUpdateOrder; }

	int GetDrawOrder() const { return mDrawOrder; }
	int GetTexHeight() const { return mTexHeight; }
	int GetTexWidth() const { return mTexWidth; }
private:
	// 自身を保持しているアクター(BackGroundクラス)
	class BackGround* mOwner;

	// このコンポーネントのUpdate order
	int mUpdateOrder;

	// テクスチャ関連のメンバ変数
	sf::Texture* mTexture;
	int mDrawOrder;
	int mTexWidth;
	int mTexHeight;
};