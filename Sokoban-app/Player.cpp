#include "Player.h"
#include "Game.h"
#include <fstream>

#include <iostream>

Player::Player(Game* game)
	: mState(State::EActive)
	, mPosition(0.0f, 0.0f)
	, mScale(sf::Vector2f(1.0f, 1.0f))
	, mRotation(0.0f)
	, mGame(game)
	, mBoardName(game->GetFilename(0))
	, mDirection(ESouth)
	, prevDirection(ESouth)
	, mMoveCooldown(0.0f)
	, mDetection(false)
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

	// プレイヤーの初期座標を得る
	{
		int i = 0, j = 0;
		for (const auto& line : lines)
		{
			for (const auto& item : line)
			{
				if (item == '@')
				{
					mBoardCoordinate = sf::Vector2i(j, i);
					break;
				}
				else if (item == '+')
				{
					mBoardCoordinate = sf::Vector2i(j, i);
					break;
				}
				j++;
			}
			i++;
			j = 0;
		}
	}

	// ウィンドウサイズ
	// 2024_09_05 描画範囲に修正
	BoundingBox viewArea = mGame->GetBoardViewArea();

	// プレイヤーのタイルを用意
	// コンポーネントを作成
	PlayerComponent* pc = new PlayerComponent(this, 100, 150);
	
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
	pc->SetTexture(mTextures[Direction::ESouth]);
	mComponent = pc;

	// スケーリングと位置の初期化を行う
	// 2024_09_05 盤面と同様に修正
	float minScale = std::min((viewArea.second.x - viewArea.first.x) / static_cast<float>(mTextures[Direction::EEast]->getSize().x * maxX), (viewArea.second.y - viewArea.first.y) / static_cast<float>(mTextures[Direction::EEast]->getSize().y * lines.size()));
	mScale = sf::Vector2f(minScale, minScale);

	// 盤面の余白の分中央揃え
	// 2024_09_05 盤面と同様に修正
	mPosition = sf::Vector2f(
		((viewArea.second.x - viewArea.first.x) - (static_cast<float>(mTextures[Direction::EEast]->getSize().x * maxX) * minScale)) / 2.0f + static_cast<float>(mTextures[Direction::EEast]->getSize().x * mBoardCoordinate.x) * mScale.x,
		((viewArea.second.y - viewArea.first.y) - (static_cast<float>(mTextures[Direction::EEast]->getSize().y * lines.size()) * minScale)) / 2.0f + static_cast<float>(mTextures[Direction::EEast]->getSize().y * mBoardCoordinate.y) * mScale.y
	);
}

Player::~Player()
{
	mGame->RemoveActor(this);
	// コンポーネントを削除する
	delete mComponent;
}

void Player::Update(float deltaTime)
{
	// TODO 更新後も判定が更新前の盤面になっている

	mMoveCooldown -= deltaTime;

	if (mState.GetEState() == State::EActive)
	{
		UpdateComponents(deltaTime);

		// このアクター特有の更新処理があれば書く
		// 向きの更新が入ったらコンポーネントのスプライトを切り替える
		mComponent->SetTexture(mTextures[mDirection]);

		// プレイヤーや荷物を抜いた盤面の情報
		std::vector<std::string> boardState = mGame->GetBoardState();

		if (mDetection)
		{
			// 移動先の座標
			sf::Vector2i destination = sf::Vector2i(mBoardCoordinate.x + static_cast<int>(std::cos(mRotation)), mBoardCoordinate.y + static_cast<int>(std::sin(mRotation)));

			mDetection = false;

			// まず移動先が壁でないか確認する
			if (boardState[destination.y][destination.x] != '#')
			{
				// 移動先に荷物があるかチェック
				const auto boxes = mGame->GetBaggages();
				std::vector<sf::Vector2i> boxesPos;
				for (const auto& item : boxes)
				{
					boxesPos.push_back(item->GetBoardCoordinate());
				}
				auto iter = boxesPos.end();
				for (auto it = boxesPos.begin(); it != boxesPos.end(); ++it)
				{
					if (*it == destination)
					{
						iter = it;
						break;
					}
				}

				if (iter == boxesPos.end())
				{
					// 荷物がなかった場合
					// 移動前の座標
					sf::Vector2i lastPos = mBoardCoordinate;

					// 移動処理
					mBoardCoordinate = destination;
					mPosition = sf::Vector2f(
						mPosition.x + static_cast<float>(mTextures[mDirection]->getSize().x) * mScale.x * std::cos(mRotation),
						mPosition.y + static_cast<float>(mTextures[mDirection]->getSize().y) * mScale.y * std::sin(mRotation)
					);

					// ゲームクラスからログを書き込む関数を呼び出す
					mGame->RemoveRedo();
					mGame->AddStep();
					mGame->AddLog(lastPos, destination, prevDirection, mDirection);
					prevDirection = mDirection;
				}
				else
				{
					// 荷物があった場合
					// 更にもう一マス先に荷物があるかチェック
					iter = boxesPos.end();
					for (auto it = boxesPos.begin(); it != boxesPos.end(); ++it)
					{
						if (*it == sf::Vector2i(destination.x + static_cast<int>(std::cos(mRotation)), destination.y + static_cast<int>(std::sin(mRotation))))
						{
							iter = it;
							break;
						}
					}
					// 荷物が2個並んでおらず、壁に突き当たらなければ荷物ごと移動
					if (iter == boxesPos.end() && boardState[destination.y + static_cast<int>(std::sin(mRotation))][destination.x + static_cast<int>(std::cos(mRotation))] != '#')
					{
						// 移動前の座標
						sf::Vector2i lastPos = mBoardCoordinate;

						// 移動処理
						mBoardCoordinate = destination;
						mPosition = sf::Vector2f(
							mPosition.x + static_cast<float>(mTextures[mDirection]->getSize().x) * mScale.x * std::cos(mRotation),
							mPosition.y + static_cast<float>(mTextures[mDirection]->getSize().y) * mScale.y * std::sin(mRotation)
						);

						sf::Vector2i baggageLastPos, baggageDestination;

						for (auto& item : mGame->GetBaggages())
						{
							if (item->GetBoardCoordinate() == destination)
							{
								baggageLastPos = sf::Vector2i(item->GetBoardCoordinate().x, item->GetBoardCoordinate().y);
								baggageDestination = sf::Vector2i(item->GetBoardCoordinate().x + static_cast<int>(std::cos(mRotation)), item->GetBoardCoordinate().y + static_cast<int>(std::sin(mRotation)));
								item->SetBoardCoordinate(baggageDestination);
								break;
							}
						}

						// ゲームクラスからログを書き込む関数を呼び出す
						mGame->RemoveRedo();
						mGame->AddStep();
						mGame->AddLog(lastPos, destination, baggageLastPos, baggageDestination, prevDirection, mDirection);
						prevDirection = mDirection;
					}
				}
			}
		}
	}
}

void Player::UpdateComponents(float deltaTime)
{
	mComponent->Update(deltaTime);
}

void Player::ProcessInput(const sf::Event::KeyEvent* keyState)
{
	if (mState.GetEState() == State::EActive)
	{
		// アクターが持つ全てのComponentの入力処理を行う
		// どのComponentも特に独自の処理を実装していなければ何もしない
		mComponent->ProcessInput(keyState);

		// このアクター特有の振る舞いがあれば書く
		// 移動入力のクールダウンが0以下なら入力を受け付ける
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
				break;
			case sf::Keyboard::Up:
				if (prevKeys.code != sf::Keyboard::Up)
				{
					mDirection = Direction::ENorth;
					mRotation = static_cast<float>(3.0 * std::acos(-1) / 2.0);
				}
				break;
			case sf::Keyboard::Left:
				if (prevKeys.code != sf::Keyboard::Left)
				{
					mDirection = Direction::EWest;
					mRotation = static_cast<float>(std::acos(-1));
				}
				break;
			case sf::Keyboard::Down:
				if (prevKeys.code != sf::Keyboard::Down)
				{
					mDirection = Direction::ESouth;
					mRotation = static_cast<float>(std::acos(-1) / 2.0);
				}
				break;
			default:
				mMoveCooldown = 0.0f;
				mDetection = false;
				break;
			}
		}
	}

	prevKeys = *keyState;
}

void Player::AddComponent(PlayerComponent* component)
{
	mComponent = component;
}

void Player::RemoveComponent(PlayerComponent* component)
{
	mComponent = nullptr;
}

void Player::SetBoardCoordinate(const sf::Vector2i boardCoordinate)
{
	mPosition = sf::Vector2f(
		mPosition.x + static_cast<float>(mTextures[mDirection]->getSize().x) * mScale.x * static_cast<float>(boardCoordinate.x - mBoardCoordinate.x),
		mPosition.y + static_cast<float>(mTextures[mDirection]->getSize().y) * mScale.y * static_cast<float>(boardCoordinate.y - mBoardCoordinate.y)
	);
	mBoardCoordinate = boardCoordinate;
}

void Player::Reload()
{
	// 盤面データをGameクラスから取得する
	mBoardName = mGame->GetFilename(0);
	std::vector<std::string> lines = mGame->GetBoardData()[mBoardName];

	// 盤面の横幅を得る
	int maxX = 0;
	for (const auto& item : lines)
	{
		if (item.length() > maxX)
		{
			maxX = (int)item.length();
		}
	}

	// プレイヤーの初期座標を得る
	{
		int i = 0, j = 0;
		for (const auto& line : lines)
		{
			for (const auto& item : line)
			{
				if (item == '@')
				{
					mBoardCoordinate = sf::Vector2i(j, i);
					break;
				}
				else if (item == '+')
				{
					mBoardCoordinate = sf::Vector2i(j, i);
					break;
				}
				j++;
			}
			i++;
			j = 0;
		}
	}

	// ウィンドウサイズ
	sf::Vector2f wSize = mGame->GetWindowSize();

	// プレイヤーのタイルを用意
	// コンポーネントを作成
	PlayerComponent* pc = new PlayerComponent(this, 100, 150);

	// コンポーネントにテクスチャをセット
	pc->SetTexture(mTextures[Direction::ESouth]);
	mComponent = pc;

	// 2024_09_05 描画範囲に修正
	BoundingBox viewArea = mGame->GetBoardViewArea();

	// スケーリングと位置の初期化を行う
	// 2024_09_05 盤面と同様に修正
	float minScale = std::min((viewArea.second.x - viewArea.first.x) / static_cast<float>(mTextures[Direction::EEast]->getSize().x * maxX), (viewArea.second.y - viewArea.first.y) / static_cast<float>(mTextures[Direction::EEast]->getSize().y * lines.size()));
	mScale = sf::Vector2f(minScale, minScale);

	// 盤面の余白の分中央揃え
	// 2024_09_05 盤面と同様に修正
	mPosition = sf::Vector2f(
		((viewArea.second.x - viewArea.first.x) - (static_cast<float>(mTextures[Direction::EEast]->getSize().x * maxX) * minScale)) / 2.0f + static_cast<float>(mTextures[Direction::EEast]->getSize().x * mBoardCoordinate.x) * mScale.x,
		((viewArea.second.y - viewArea.first.y) - (static_cast<float>(mTextures[Direction::EEast]->getSize().y * lines.size()) * minScale)) / 2.0f + static_cast<float>(mTextures[Direction::EEast]->getSize().y * mBoardCoordinate.y) * mScale.y
	);
}