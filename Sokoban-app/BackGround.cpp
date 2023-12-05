#include "BackGround.h"
#include "Game.h"

BackGround::BackGround(Game* game)
	: mState(State::EActive)
	, mPosition(0.0, 0.0)
	, mScale(sf::Vector2f(1.0f, 1.0f))
	, mRotation(0.0f)
	, mGame(game)
{
	mGame->AddActor(this);

	// SpriteComponentを作成する
	// 床と壁のタイルを用意
	// TODO 読み込んだ盤面データに応じたステージのテクスチャを作成する
	BackGroundComponent* gbc = new BackGroundComponent(this, 100, 150);
	gbc->SetTexture(game->LoadTexture("Assets/BackGround.png"));
}

BackGround::~BackGround()
{
	mGame->RemoveActor(this);
	// コンポーネントを削除する
	while (!mComponents.empty())
	{
		delete mComponents.back();
	}
}

void BackGround::Update(float deltaTime)
{
	if (mState.GetEState() == State::EActive)
	{
		UpdateComponents(deltaTime);

		// このアクター特有の更新処理があれば書く
	}
}

void BackGround::UpdateComponents(float deltaTime)
{
	for (auto item : mComponents)
	{
		item->Update(deltaTime);
	}
}

void BackGround::ProcessInput(const sf::Event::KeyEvent* keyState)
{
	if (mState.GetEState() == State::EActive)
	{
		// アクターが持つ全てのComponentの入力処理を行う
		// どのComponentも特に独自の処理を実装していなければ何もしない

		for (auto item : mComponents)
		{
			item->ProcessInput(keyState);
		}

		// このアクター特有の振る舞いがあれば書く

	}

}

void BackGround::AddComponent(BackGroundComponent* component)
{
	int myOrder = component->GetUpdateOrder();
	auto iter = mComponents.begin();
	for (;
		iter != mComponents.end();
		++iter)
	{
		if (myOrder < (*iter)->GetUpdateOrder())
		{
			break;
		}
	}

	mComponents.insert(iter, component);
}

void BackGround::RemoveComponent(BackGroundComponent* component)
{
	auto iter = std::find(mComponents.begin(), mComponents.end(), component);
	if (iter != mComponents.end())
	{
		mComponents.erase(iter);
	}
}