#include "Baggage.h"
#include "Game.h"
#include <fstream>

Baggage::Baggage(Game* game, sf::Vector2i bCoordinate)
	: mState(State::EActive)
	, mPosition(0.0, 0.0)
	, mScale(sf::Vector2f(1.0f, 1.0f))
	, mRotation(0.0f)
	, mGame(game)
	, mBoardName(game->GetFilename(0))
	, mBoardCoordinate(bCoordinate)
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
	sf::Vector2f wSize = mGame->GetWindowSize();

	// プレイヤーのタイルを用意
	// コンポーネントを作成
	BaggageComponent* pc = new BaggageComponent(this, 100, 150);
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

	// コンポーネントにテクスチャをセット
	pc->SetTexture(mTextures[bState]);
	mComponent = pc;

	// スケーリングと位置の初期化を行う
	float minScale = std::min(wSize.x / static_cast<float>(mTextures[bState]->getSize().x * maxX), wSize.y / static_cast<float>(mTextures[bState]->getSize().y * lines.size()));
	mScale = sf::Vector2f(minScale, minScale);

	// 盤面の余白の分中央揃え
	mPosition = sf::Vector2f(
		(wSize.x - (static_cast<float>(mTextures[bState]->getSize().x * maxX) * minScale)) / 2.0f + static_cast<float>(mTextures[bState]->getSize().x * mBoardCoordinate.x) * mScale.x,
		(wSize.y - (static_cast<float>(mTextures[bState]->getSize().y * lines.size()) * minScale)) / 2.0f + static_cast<float>(mTextures[bState]->getSize().y * mBoardCoordinate.y) * mScale.y
	);
}

Baggage::~Baggage()
{
	mGame->RemoveActor(this);
	// コンポーネントを削除する
	delete mComponent;
}

void Baggage::Update(float deltaTime)
{
	if (mState.GetEState() == State::EActive)
	{
		UpdateComponents(deltaTime);

		// このアクター特有の更新処理があれば書く
		// このアクターがある床に合わせてテクスチャを変える
		switch (mGame->GetBoardState()[mBoardCoordinate.y][mBoardCoordinate.x])
		{
		case ' ':
			mComponent->SetTexture(mTextures[BState::OnFloor]);
			break;
		case '.':
			mComponent->SetTexture(mTextures[BState::OnGoal]);
			break;
		default:
			break;
		}
	}
}

void Baggage::UpdateComponents(float deltaTime)
{
	mComponent->Update(deltaTime);
}

void Baggage::ProcessInput(const sf::Event::KeyEvent* keyState)
{
	if (mState.GetEState() == State::EActive)
	{
		// アクターが持つ全てのComponentの入力処理を行う
		// どのComponentも特に独自の処理を実装していなければ何もしない
		mComponent->ProcessInput(keyState);

		// このアクター特有の振る舞いがあれば書く
		// 荷物はキーボード入力によって自発的には動かない

	}

}

void Baggage::AddComponent(BaggageComponent* component)
{
	mComponent = component;
}

void Baggage::RemoveComponent(BaggageComponent* component)
{
	mComponent = nullptr;
}

void Baggage::SetBoardCoordinate(const sf::Vector2i boardCoordinate)
{
	mPosition = sf::Vector2f(
		mPosition.x + static_cast<float>(mTextures[bState]->getSize().x) * mScale.x * static_cast<float>(boardCoordinate.x - mBoardCoordinate.x),
		mPosition.y + static_cast<float>(mTextures[bState]->getSize().y) * mScale.y * static_cast<float>(boardCoordinate.y - mBoardCoordinate.y)
	);
	mBoardCoordinate = boardCoordinate;
}