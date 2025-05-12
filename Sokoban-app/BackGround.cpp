#include "BackGround.h"

#include "Game.h"
#include "SpriteComponent.h"

BackGround::BackGround(Game* game)
	: mState(IActor::ActorState::EActive)
	, mPosition(0.0, 0.0)
	, mScale(sf::Vector2f(1.0f, 1.0f))
	, mRotation(0.0f)
	, mComponents(std::vector<IComponent*>{})
	, mSpriteComponent(nullptr)
	, mTextures(std::unordered_map<std::string, sf::Texture*>{})
	, mGame(game)
{
	mGame->AddActor(this);

	// SpriteComponentを作成する
	// 床と壁のタイルを用意
	// TODO 読み込んだ盤面データに応じたステージのテクスチャを作成する
	std::string fileName = "Assets/BackGround.png";
	sf::Texture* backGround = game->LoadTexture(fileName);
	mSpriteComponent = new SpriteComponent(this, 100, 0);
	mSpriteComponent->SetTexture(backGround);
	mTextures.emplace(fileName, backGround);
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
	if (mState == IActor::ActorState::EActive)
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

void BackGround::ProcessInput(const sf::Event* event)
{
	if (mState == IActor::ActorState::EActive)
	{
		// アクターが持つ全てのComponentの入力処理を行う
		// どのComponentも特に独自の処理を実装していなければ何もしない
		ProcessInputComponents(event);

		// このアクター特有の振る舞いがあれば書く

	}

}

void BackGround::ProcessInputComponents(const sf::Event* event)
{
	for (auto& component : mComponents)
	{
		component->ProcessInput(event);
	}
}

void BackGround::AddComponent(IComponent* component)
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

void BackGround::RemoveComponent(IComponent* component)
{
	auto iter = std::find(mComponents.begin(), mComponents.end(), component);
	if (iter != mComponents.end())
	{
		mComponents.erase(iter);
	}
}