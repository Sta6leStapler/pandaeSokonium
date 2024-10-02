#include "Menu.h"
#include "Game.h"
#include <fstream>

#include <iostream>

Menu::Menu(Game* game)
	: mState(State::EActive)
	, mPosition(0.0f, 0.0f)
	, mScale(sf::Vector2f(1.0f, 1.0f))
	, mRotation(0.0f)
	, mGame(game)
	, mBoardName(game->GetFilename(0))
	, mMoveCooldown(0.0f)
	, mDetection(false)
{
	mGame->AddActor(this);

	// 描画範囲
	BoundingBox viewArea = mGame->GetBoardViewArea();

	// プレイヤーのタイルを用意
	// コンポーネントを作成
	MenuComponent* mc = new MenuComponent(this, 100, 150);

	// ファイルを読み込む
	std::string filename = "Assets/playerE.png";
	sf::Image image;
	image.loadFromFile(filename);
	mTextures.emplace(Direction::EEast, game->LoadTexture(filename));
	filename = "Assets/playerN.png";
	image.loadFromFile(filename);
	mTextures.emplace(Direction::ENorth, game->LoadTexture(filename));
	filename = "Assets/playerW.png";
	image.loadFromFile(filename);
	mTextures.emplace(Direction::EWest, game->LoadTexture(filename));
	filename = "Assets/playerS.png";
	image.loadFromFile(filename);
	mTextures.emplace(Direction::ESouth, game->LoadTexture(filename));


	// コンポーネントにテクスチャをセット
	mc->SetTexture(mTextures[Direction::ESouth]);
	mComponent = mc;

	// スケーリングと位置の初期化を行う
	float minScale = std::min((viewArea.second.x - viewArea.first.x) / static_cast<float>(mTextures[Direction::EEast]->getSize().x * maxX), (viewArea.second.y - viewArea.first.y) / static_cast<float>(mTextures[Direction::EEast]->getSize().y * lines.size()));
	mScale = sf::Vector2f(minScale, minScale);

	// 余白の分中央揃え
	mPosition = sf::Vector2f(
		((viewArea.second.x - viewArea.first.x) - (static_cast<float>(mTextures[Direction::EEast]->getSize().x * maxX) * minScale)) / 2.0f + static_cast<float>(mTextures[Direction::EEast]->getSize().x * mBoardCoordinate.x) * mScale.x,
		((viewArea.second.y - viewArea.first.y) - (static_cast<float>(mTextures[Direction::EEast]->getSize().y * lines.size()) * minScale)) / 2.0f + static_cast<float>(mTextures[Direction::EEast]->getSize().y * mBoardCoordinate.y) * mScale.y
	);
}

Menu::~Menu()
{
	mGame->RemoveActor(this);
	// コンポーネントを削除する
	delete mComponent;
}

void Menu::Update(float deltaTime)
{
	mMoveCooldown -= deltaTime;

	if (mState.GetEState() == State::EActive)
	{
		UpdateComponents(deltaTime);

		// このアクター特有の更新処理があれば書く

	}
}

void Menu::UpdateComponents(float deltaTime)
{
	mComponent->Update(deltaTime);
}

void Menu::ProcessInput(const sf::Event::KeyEvent* keyState)
{
	if (mState.GetEState() == State::EActive)
	{
		// アクターが持つ全てのComponentの入力処理を行う
		// どのComponentも特に独自の処理を実装していなければ何もしない
		mComponent->ProcessInput(keyState);

		// このアクター特有の振る舞いがあれば書く
		// 入力のクールダウンが0以下なら入力を受け付ける
		if (mMoveCooldown <= 0.0f)
		{
			mMoveCooldown = 0.13f;
			mDetection = true;
			switch (keyState->code)
			{
			case sf::Keyboard::Right:
				if (prevKeys.code != sf::Keyboard::Right)
				{
					mDirection = Direction::EEast;
					mRotation = 0.0f;
				}
			default:
				mMoveCooldown = 0.0f;
				mDetection = false;
				break;
			}
		}
	}

	prevKeys = *keyState;
}

void Menu::AddComponent(MenuComponent* component)
{
	mComponent = component;
}

void Menu::RemoveComponent(MenuComponent* component)
{
	mComponent = nullptr;
}

void Menu::Reload()
{
	// TODO 各種必要なデータをGameクラスから取得する
	

	// ウィンドウサイズ
	sf::Vector2f wSize = mGame->GetWindowSize();

	// プレイヤーのタイルを用意
	// コンポーネントを作成
	MenuComponent* mc = new MenuComponent(this, 100, 150);

	// コンポーネントにテクスチャをセット
	mc->SetTexture(mTextures[Direction::ESouth]);
	mComponent = mc;

	// スケーリングと位置の初期化を行う
	float minScale = std::min(wSize.x / static_cast<float>(mTextures[Direction::EEast]->getSize().x * maxX), wSize.y / static_cast<float>(mTextures[Direction::EEast]->getSize().y * lines.size()));
	mScale = sf::Vector2f(minScale, minScale);

	// 盤面の余白の分中央揃え
	mPosition = sf::Vector2f(
		(wSize.x - (static_cast<float>(mTextures[Direction::EEast]->getSize().x * maxX) * minScale)) / 2.0f + static_cast<float>(mTextures[Direction::EEast]->getSize().x * mBoardCoordinate.x) * mScale.x,
		(wSize.y - (static_cast<float>(mTextures[Direction::EEast]->getSize().y * lines.size()) * minScale)) / 2.0f + static_cast<float>(mTextures[Direction::EEast]->getSize().y * mBoardCoordinate.y) * mScale.y
	);
}