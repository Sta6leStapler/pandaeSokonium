#include "Baggage.h"

#include "Game.h"
#include "SpriteComponent.h"
#include "HUDHelper.h"

#include <fstream>

Baggage::Baggage(Game* game, sf::Vector2i bCoordinate)
	: mState(IActor::ActorState::EActive)
	, mPosition(0.0, 0.0)
	, mScale(sf::Vector2f(1.0f, 1.0f))
	, mRotation(0.0f)
	, mComponents(std::vector<IComponent*>{})
	, mSpriteComponent(nullptr)
	, mTextures(std::unordered_map<BState, sf::Texture*>{})
	, mGame(game)
	, mBoardName(game->GetCurrentKey())
	, mBoardCoordinate(bCoordinate)
	, bState(BState::OnFloor)
{
	mGame->AddActor(this);

	// 盤面データをGameクラスから取得する
	std::vector<std::string> lines = game->GetBoardData()[mBoardName];

	// 盤面の横幅を得る
	int maxX = 0;
	for (const auto& item : lines)
	{
		if (item.length() > maxX)
		{
			maxX = (int)item.length();
		}
	}

	// ウィンドウサイズ
	// 2024_09_05 描画範囲に修正
	BoundingBox viewArea = mGame->GetBoardViewArea();

	// プレイヤーのタイルを用意
	// コンポーネントを作成
	mSpriteComponent = new SpriteComponent(this, 100, 100);
	// 初期位置がゴール上にあるかどうか
	bState = BState::OnFloor;
	if (mGame->GetBoardState()[mBoardCoordinate.y][mBoardCoordinate.x] == '.')
	{
		bState = BState::OnGoal;
	}

	// ファイルを読み込む
	std::string filename = "Assets/Box.png";
	sf::Image image;
	image.loadFromFile(filename);
	mTextures.emplace(BState::OnFloor, game->LoadTexture(filename));
	filename = "Assets/ShinyBox.png";
	image.loadFromFile(filename);
	mTextures.emplace(BState::OnGoal, game->LoadTexture(filename));
	filename = "Assets/DeadlockedBox.png";
	image.loadFromFile(filename);
	mTextures.emplace(BState::Deadlock, game->LoadTexture(filename));

	// コンポーネントにテクスチャをセット
	mSpriteComponent->SetTexture(mTextures[bState]);

	// スケーリングと位置の初期化を行う
	// 表示エリアのサイズ　/ 盤面のサイズ を求める
	// 表示エリアの方が小さければタイルは縮小すべきで、逆なら拡大するべき
	float minScale = std::min((viewArea.second.x - viewArea.first.x) / static_cast<float>(mTextures[bState]->getSize().x * mGame->GetBoardSize().x),
		(viewArea.second.y - viewArea.first.y) / static_cast<float>(mTextures[bState]->getSize().y * mGame->GetBoardSize().y));

	mScale = sf::Vector2f(minScale, minScale);

	// 余白の分中央揃えする
	// *メモ UI等でずれる場合はオフセットを加えておく
	mPosition = sf::Vector2f
	{
		viewArea.first.x + (viewArea.second.x - viewArea.first.x - static_cast<float>(mTextures[bState]->getSize().x * mGame->GetBoardSize().x) * mScale.x) / 2.0f + static_cast<float>(mTextures[bState]->getSize().x * mBoardCoordinate.x) * mScale.x,
		viewArea.first.y + (viewArea.second.y - viewArea.first.y - static_cast<float>(mTextures[bState]->getSize().x * mGame->GetBoardSize().y) * mScale.y) / 2.0f + static_cast<float>(mTextures[bState]->getSize().y * mBoardCoordinate.y) * mScale.y
	};
}

Baggage::~Baggage()
{
	mGame->RemoveActor(this);
	// コンポーネントを削除する
	while (!mComponents.empty())
	{
		delete mComponents.back();
	}
}

void Baggage::Update(float deltaTime)
{
	if (mState == IActor::ActorState::EActive)
	{
		UpdateComponents(deltaTime);

		// このアクター特有の更新処理があれば書く
		// このアクターの位置に応じてテクスチャを変える
		if (mGame->GetBoardState()[mBoardCoordinate.y][mBoardCoordinate.x] == '.')
		{
			mSpriteComponent->SetTexture(mTextures[BState::OnGoal]);
		}
		else if (mGame->GetHUDHelper()->isDeadlockedBaggage(mBoardCoordinate))
		{
			mSpriteComponent->SetTexture(mTextures[BState::Deadlock]);
		}
		else
		{
			mSpriteComponent->SetTexture(mTextures[BState::OnFloor]);
		}
	}
}

void Baggage::UpdateComponents(float deltaTime)
{
	for (auto item : mComponents)
	{
		item->Update(deltaTime);
	}
}

void Baggage::ProcessInput(const sf::Event* event)
{
	if (mState == IActor::ActorState::EActive)
	{
		// アクターが持つ全てのComponentの入力処理を行う
		// どのComponentも特に独自の処理を実装していなければ何もしない
		ProcessInputComponents(event);

		// このアクター特有の振る舞いがあれば書く

	}

}

void Baggage::ProcessInputComponents(const sf::Event* event)
{
	for (auto& component : mComponents)
	{
		component->ProcessInput(event);
	}
}

void Baggage::AddComponent(IComponent* component)
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

void Baggage::RemoveComponent(IComponent* component)
{
	auto iter = std::find(mComponents.begin(), mComponents.end(), component);
	if (iter != mComponents.end())
	{
		mComponents.erase(iter);
	}
}

void Baggage::SetBoardCoordinate(const sf::Vector2i boardCoordinate)
{
	mPosition = sf::Vector2f(
		mPosition.x + static_cast<float>(mTextures[bState]->getSize().x) * mScale.x * static_cast<float>(boardCoordinate.x - mBoardCoordinate.x),
		mPosition.y + static_cast<float>(mTextures[bState]->getSize().y) * mScale.y * static_cast<float>(boardCoordinate.y - mBoardCoordinate.y)
	);
	mBoardCoordinate = boardCoordinate;
}