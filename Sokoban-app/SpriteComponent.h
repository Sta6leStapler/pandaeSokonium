#pragma once

class SpriteComponent
{
public:
	// コンストラクタ
	// updateOrderが低ければ,先に描画される(画面の後方に来る)
	SpriteComponent(class Mediator* owner, int updateOrder = 100, int drawOrder = 100);
	// デストラクタ
	virtual ~SpriteComponent();

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

private:
	// 自身を保持しているアクターを仲介するクラス
	class Mediator* mOwner;

	// このコンポーネントのUpdate order
	int mUpdateOrder;

	// テクスチャ関連のメンバ変数
	sf::Texture* mTexture;
	int mDrawOrder;
};

