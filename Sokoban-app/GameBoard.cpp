#include "GameBoard.h"

#include "Game.h"
#include "SpriteComponent.h"

#include <iostream>
#include <fstream>

GameBoard::GameBoard(Game* game)
	: mState(IActor::ActorState::EActive)
	, mPosition(0.0, 0.0)
	, mScale(sf::Vector2f(1.0f, 1.0f))
	, mRotation(0.0f)
	, mComponents(std::vector<IComponent*>{})
	, mBoardSpriteComponent(nullptr)
	, mHighlightSpriteComponent(nullptr)
	, mTextures(std::unordered_map<std::string, sf::Texture*>{})
	, mGame(game)
	, mBoardName(game->GetCurrentKey())
	, mMoveHighlightedTiles(std::vector<sf::Vector2i>{})
	, mPushHighlightedTiles(std::vector<sf::Vector2i>{})
	, mMoveHighlightTexture(nullptr)
	, mPushHighlightTexture(nullptr)
	, mHighlightingBaggage(nullptr)
{
	mGame->AddActor(this);

	// SpriteComponentを作成する
	// 床と壁のタイルを用意
	// 読み込んだ盤面データに応じたステージのテクスチャを作成する
	mBoardSpriteComponent = new SpriteComponent(this, 100, 50);
	std::string filename = "Assets/Floor.png";
	mTextures.emplace(filename, game->LoadTexture(filename));
	filename = "Assets/Wall.png";
	mTextures.emplace(filename, game->LoadTexture(filename));
	filename = "Assets/Goal.png";
	mTextures.emplace(filename, game->LoadTexture(filename));
	filename = "Assets/MoveGuideGrid.png";
	mTextures.emplace(filename, game->LoadTexture(filename));
	mMoveHighlightTexture = game->LoadTexture(filename);
	filename = "Assets/PushGuideGrid.png";
	mTextures.emplace(filename, game->LoadTexture(filename));
	mPushHighlightTexture = game->LoadTexture(filename);
	
	std::vector<std::string> lines = game->GetBoardData()[mBoardName];

	// 盤面の横幅を揃える
	size_t maxX = 0;
	for (const auto& item : lines)
	{
		if (item.length() > maxX)
		{
			maxX = item.length();
		}
	}

	// 盤面のテクスチャを作成
	sf::RenderTexture* boardTexture = new sf::RenderTexture();
	// 2024_09_05 ウィンドウサイズから描画範囲に変更
	BoundingBox viewArea = mGame->GetBoardViewArea();

	// 床と壁のスプライトを作成
	sf::Sprite tmpFloor, tmpWall, tmpGoal;
	tmpFloor.setTexture(*mTextures["Assets/Floor.png"]);
	tmpWall.setTexture(*mTextures["Assets/Wall.png"]);
	tmpGoal.setTexture(*mTextures["Assets/Goal.png"]);
	
	// スプライトをテクスチャに貼り付けていく
	boardTexture->create(static_cast<int>(tmpFloor.getGlobalBounds().width * maxX), static_cast<int>(tmpFloor.getGlobalBounds().height * lines.size()));
	for (int i = 0; i < lines.size(); i++)
	{
		for (int j = 0; j < lines[i].length(); j++)
		{
			switch (lines[i][j])
			{
			case ' ':
				tmpFloor.setPosition(tmpFloor.getGlobalBounds().width * static_cast<float>(j), tmpFloor.getGlobalBounds().height * static_cast<float>(i));
				boardTexture->draw(tmpFloor);
				break;
			case '#':
				tmpWall.setPosition(tmpWall.getGlobalBounds().width * static_cast<float>(j), tmpWall.getGlobalBounds().height * static_cast<float>(i));
				boardTexture->draw(tmpWall);
				break;
			default:
				break;
			case '$':
				tmpFloor.setPosition(tmpFloor.getGlobalBounds().width * static_cast<float>(j), tmpFloor.getGlobalBounds().height * static_cast<float>(i));
				boardTexture->draw(tmpFloor);
				break;
			case '.':
				tmpGoal.setPosition(tmpGoal.getGlobalBounds().width * static_cast<float>(j), tmpGoal.getGlobalBounds().height * static_cast<float>(i));
				boardTexture->draw(tmpGoal);
				break;
			case '*':
				tmpGoal.setPosition(tmpGoal.getGlobalBounds().width * static_cast<float>(j), tmpGoal.getGlobalBounds().height * static_cast<float>(i));
				boardTexture->draw(tmpGoal);
				break;
			case '@':
				tmpFloor.setPosition(tmpFloor.getGlobalBounds().width * static_cast<float>(j), tmpFloor.getGlobalBounds().height * static_cast<float>(i));
				boardTexture->draw(tmpFloor);
				break;
			case '+':
				tmpGoal.setPosition(tmpGoal.getGlobalBounds().width * static_cast<float>(j), tmpGoal.getGlobalBounds().height * static_cast<float>(i));
				boardTexture->draw(tmpGoal);
			}
		}
	}

	sf::Texture* tmpTexture = new sf::Texture(boardTexture->getTexture());
	delete boardTexture;

	mBoardSpriteComponent->SetTexture(tmpTexture);

	// スケーリングと位置の初期化を行う
	// 表示エリアのサイズ　/ 盤面のサイズ を求める
	// 表示エリアの方が小さければタイルは縮小すべきで、逆なら拡大するべき
	float minScale = std::min((viewArea.second.x - viewArea.first.x) / static_cast<float>(tmpTexture->getSize().x),
		(viewArea.second.y - viewArea.first.y) / static_cast<float>(tmpTexture->getSize().y));

	mScale = sf::Vector2f(minScale, -minScale);

	// 余白の分中央揃えする
	// *メモ UI等でずれる場合はオフセットを加えておく
	mPosition = sf::Vector2f
	{
		viewArea.first.x + (viewArea.second.x - viewArea.first.x - static_cast<float>(tmpTexture->getSize().x) * mScale.x) / 2.0f,
		viewArea.first.y + (viewArea.second.y - viewArea.first.y - static_cast<float>(tmpTexture->getSize().y) * mScale.y) / 2.0f
	};
}

GameBoard::~GameBoard()
{
	mGame->RemoveActor(this);
	// コンポーネントを削除する
	while (!mComponents.empty())
	{
		delete mComponents.back();
	}
}

void GameBoard::Update(float deltaTime)
{
	if (mState == IActor::ActorState::EActive)
	{
		UpdateComponents(deltaTime);

		// このアクター特有の更新処理があれば書く
	}
}

void GameBoard::UpdateComponents(float deltaTime)
{
	for (auto item : mComponents)
	{
		item->Update(deltaTime);
	}
}

void GameBoard::ProcessInput(const sf::Event* event, const std::map<sf::Keyboard::Key, float>& key_held_duration, const std::map<sf::Keyboard::Key, float>& auto_repeat_timer)
{
	if (mState == IActor::ActorState::EActive)
	{
		// アクターが持つ全てのComponentの入力処理を行う
		// どのComponentも特に独自の処理を実装していなければ何もしない
		ProcessInputComponents(event, key_held_duration, auto_repeat_timer);

		// このアクター特有の振る舞いがあれば書く
		
	}

}

void GameBoard::ProcessInputComponents(const sf::Event* event, const std::map<sf::Keyboard::Key, float>& key_held_duration, const std::map<sf::Keyboard::Key, float>& auto_repeat_timer)
{
	for (auto& component : mComponents)
	{
		component->ProcessInput(event, key_held_duration, auto_repeat_timer);
	}
}

void GameBoard::AddComponent(IComponent* component)
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

void GameBoard::RemoveComponent(IComponent* component)
{
	auto iter = std::find(mComponents.begin(), mComponents.end(), component);
	if (iter != mComponents.end())
	{
		mComponents.erase(iter);
	}
}

void GameBoard::Reload()
{
	// ハイライト中のタイルを消去
	ClearMoveHighlights();

	mBoardName = mGame->GetCurrentKey();
	std::vector<std::string> lines = mGame->GetInitBoardData(mBoardName);

	// 盤面の横幅を揃える
	size_t maxX = 0;
	for (const auto& item : lines)
	{
		if (item.length() > maxX)
		{
			maxX = item.length();
		}
	}

	// 盤面のテクスチャを作成
	sf::RenderTexture* boardTexture = new sf::RenderTexture();

	// 床と壁のスプライトを作成
	sf::Sprite tmpFloor, tmpWall, tmpGoal;
	tmpFloor.setTexture(*mTextures["Assets/Floor.png"]);
	tmpWall.setTexture(*mTextures["Assets/Wall.png"]);
	tmpGoal.setTexture(*mTextures["Assets/Goal.png"]);

	// スプライトをテクスチャに貼り付けていく
	boardTexture->create(static_cast<int>(tmpFloor.getGlobalBounds().width * maxX), static_cast<int>(tmpFloor.getGlobalBounds().height * lines.size()));
	for (int i = 0; i < lines.size(); i++)
	{
		for (int j = 0; j < lines[i].length(); j++)
		{
			switch (lines[i][j])
			{
			case ' ':
				tmpFloor.setPosition(tmpFloor.getGlobalBounds().width * static_cast<float>(j), tmpFloor.getGlobalBounds().height * static_cast<float>(i));
				boardTexture->draw(tmpFloor);
				break;
			case '#':
				tmpWall.setPosition(tmpWall.getGlobalBounds().width * static_cast<float>(j), tmpWall.getGlobalBounds().height * static_cast<float>(i));
				boardTexture->draw(tmpWall);
				break;
			default:
				break;
			case '$':
				tmpFloor.setPosition(tmpFloor.getGlobalBounds().width * static_cast<float>(j), tmpFloor.getGlobalBounds().height * static_cast<float>(i));
				boardTexture->draw(tmpFloor);
				break;
			case '.':
				tmpGoal.setPosition(tmpGoal.getGlobalBounds().width * static_cast<float>(j), tmpGoal.getGlobalBounds().height * static_cast<float>(i));
				boardTexture->draw(tmpGoal);
				break;
			case '*':
				tmpGoal.setPosition(tmpGoal.getGlobalBounds().width * static_cast<float>(j), tmpGoal.getGlobalBounds().height * static_cast<float>(i));
				boardTexture->draw(tmpGoal);
				break;
			case '@':
				tmpFloor.setPosition(tmpFloor.getGlobalBounds().width * static_cast<float>(j), tmpFloor.getGlobalBounds().height * static_cast<float>(i));
				boardTexture->draw(tmpFloor);
				break;
			case '+':
				tmpGoal.setPosition(tmpGoal.getGlobalBounds().width * static_cast<float>(j), tmpGoal.getGlobalBounds().height * static_cast<float>(i));
				boardTexture->draw(tmpGoal);
			}
		}
	}

	sf::Texture* tmpTexture = new sf::Texture(boardTexture->getTexture());
	delete boardTexture;

	mBoardSpriteComponent->SetTexture(tmpTexture);

	// 2024_09_05 ウィンドウサイズから描画範囲に変更
	BoundingBox viewArea = mGame->GetBoardViewArea();

	// スケーリングと位置の初期化を行う
	// 表示エリアのサイズ　/ 盤面のサイズ を求める
	// 表示エリアの方が小さければタイルは縮小すべきで、逆なら拡大するべき
	float minScale = std::min((viewArea.second.x - viewArea.first.x) / static_cast<float>(tmpTexture->getSize().x),
		(viewArea.second.y - viewArea.first.y) / static_cast<float>(tmpTexture->getSize().y));

	mScale = sf::Vector2f(minScale, -minScale);

	// 余白の分中央揃えする
	// *メモ UI等でずれる場合はオフセットを加えておく
	mPosition = sf::Vector2f
	{
		viewArea.first.x + (viewArea.second.x - viewArea.first.x - static_cast<float>(tmpTexture->getSize().x) * mScale.x) / 2.0f,
		viewArea.first.y + (viewArea.second.y - viewArea.first.y - static_cast<float>(tmpTexture->getSize().y) * mScale.y) / 2.0f
	};

}

void GameBoard::SetMoveHighlightedTiles(const std::vector<sf::Vector2i>& tiles)
{
	// 荷物の移動可能マスのハイライトを先に消す
	mGame->ClearPushHighlights();

	// ハイライトされるタイルのリストを読み取る
	mMoveHighlightedTiles = tiles;

	// ハイライトのテクスチャを作成
	sf::RenderTexture* highlightTexture = new sf::RenderTexture();

	// 各マスに貼り付けるハイライトのスプライトを作成
	sf::Sprite tmpHighlight;
	tmpHighlight.setTexture(*mMoveHighlightTexture);

	// スプライトをテクスチャに貼り付けていく
	highlightTexture->create(static_cast<int>(tmpHighlight.getGlobalBounds().width * mGame->GetBoardSize().x), static_cast<int>(tmpHighlight.getGlobalBounds().height * mGame->GetBoardSize().y));

	for (const auto& tile : mMoveHighlightedTiles)
	{
		tmpHighlight.setPosition(tmpHighlight.getGlobalBounds().width * static_cast<float>(tile.x), tmpHighlight.getGlobalBounds().height * static_cast<float>(tile.y));
		highlightTexture->draw(tmpHighlight);
	}

	sf::Texture* tmpTexture = new sf::Texture(highlightTexture->getTexture());
	delete highlightTexture;

	mHighlightSpriteComponent = new SpriteComponent(this, 100, 150);
	mHighlightSpriteComponent->SetTexture(tmpTexture);
}

void GameBoard::ClearMoveHighlights()
{
	mMoveHighlightedTiles.clear();
	
	if (mHighlightSpriteComponent != nullptr)
	{
		delete mHighlightSpriteComponent;
		mHighlightSpriteComponent = nullptr;
	}
}

void GameBoard::SetPushHighlightedTiles(const std::vector<sf::Vector2i>& tiles, Baggage* baggage)
{
	// プレイヤーの移動可能マスのハイライトを先に消す
	mGame->ClearMoveHighlights();

	// 表示対象の荷物のポインタを登録
	mHighlightingBaggage = baggage;

	// ハイライトされるタイルのリストを読み取る
	mPushHighlightedTiles = tiles;

	// ハイライトのテクスチャを作成
	sf::RenderTexture* highlightTexture = new sf::RenderTexture();

	// 各マスに貼り付けるハイライトのスプライトを作成
	sf::Sprite tmpHighlight;
	tmpHighlight.setTexture(*mPushHighlightTexture);

	// スプライトをテクスチャに貼り付けていく
	highlightTexture->create(static_cast<int>(tmpHighlight.getGlobalBounds().width * mGame->GetBoardSize().x), static_cast<int>(tmpHighlight.getGlobalBounds().height * mGame->GetBoardSize().y));

	for (const auto& tile : mPushHighlightedTiles)
	{
		tmpHighlight.setPosition(tmpHighlight.getGlobalBounds().width * static_cast<float>(tile.x), tmpHighlight.getGlobalBounds().height * static_cast<float>(tile.y));
		highlightTexture->draw(tmpHighlight);
	}

	sf::Texture* tmpTexture = new sf::Texture(highlightTexture->getTexture());
	delete highlightTexture;

	mHighlightSpriteComponent = new SpriteComponent(this, 100, 150);
	mHighlightSpriteComponent->SetTexture(tmpTexture);
}

void GameBoard::ClearPushHighlights()
{
	mPushHighlightedTiles.clear();
	mHighlightingBaggage = nullptr;

	if (mHighlightSpriteComponent != nullptr)
	{
		delete mHighlightSpriteComponent;
		mHighlightSpriteComponent = nullptr;
	}
}