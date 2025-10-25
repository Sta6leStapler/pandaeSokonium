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
	, mIdleArrowTextures(std::map<Direction, sf::Texture*>{})
	, mOnCursorArrowTextures(std::map<Direction, sf::Texture*>{})
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
	
	// ハイライト用のテクスチャを読み込む
	filename = "Assets/MoveGuideGrid.png";
	mTextures.emplace(filename, game->LoadTexture(filename));
	mMoveHighlightTexture = game->LoadTexture(filename);
	filename = "Assets/PushGuideGrid.png";
	mTextures.emplace(filename, game->LoadTexture(filename));
	mPushHighlightTexture = game->LoadTexture(filename);
	filename = "Assets/PushGuideArrowIdleFromE.png";
	mTextures.emplace(filename, game->LoadTexture(filename));
	mIdleArrowTextures.emplace(Direction::EEast, mTextures[filename]);
	filename = "Assets/PushGuideArrowIdleFromS.png";
	mTextures.emplace(filename, game->LoadTexture(filename));
	mIdleArrowTextures.emplace(Direction::ESouth, mTextures[filename]);
	filename = "Assets/PushGuideArrowIdleFromW.png";
	mTextures.emplace(filename, game->LoadTexture(filename));
	mIdleArrowTextures.emplace(Direction::EWest, mTextures[filename]);
	filename = "Assets/PushGuideArrowIdleFromN.png";
	mTextures.emplace(filename, game->LoadTexture(filename));
	mIdleArrowTextures.emplace(Direction::ENorth, mTextures[filename]);
	filename = "Assets/PushGuideArrowOnCursorFromE.png";
	mTextures.emplace(filename, game->LoadTexture(filename));
	mOnCursorArrowTextures.emplace(Direction::EEast, mTextures[filename]);
	filename = "Assets/PushGuideArrowOnCursorFromS.png";
	mTextures.emplace(filename, game->LoadTexture(filename));
	mOnCursorArrowTextures.emplace(Direction::ESouth, mTextures[filename]);
	filename = "Assets/PushGuideArrowOnCursorFromW.png";
	mTextures.emplace(filename, game->LoadTexture(filename));
	mOnCursorArrowTextures.emplace(Direction::EWest, mTextures[filename]);
	filename = "Assets/PushGuideArrowOnCursorFromN.png";
	mTextures.emplace(filename, game->LoadTexture(filename));
	mOnCursorArrowTextures.emplace(Direction::ENorth, mTextures[filename]);
	
	// 盤面の読み込みを行う
	// テキストデータを取得
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
	// 荷物の移動可能マスと運搬可能な方向のハイライトを先に消す
	mGame->ClearPushHighlights();
	mGame->ClearPushDirections();

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
	// プレイヤーの移動可能マスのハイライトと荷物の運搬可能な方向の表示を先に消す
	mGame->ClearMoveHighlights();
	mGame->ClearPushDirections();

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

void GameBoard::SetPushDirections(const std::vector<int>& indexes, class Baggage* baggage)
{
	// プレイヤーの移動可能マスのハイライトと荷物のハイライトを先に消す
	mGame->ClearMoveHighlights();
	mGame->ClearPushHighlights();

	// 表示対象の荷物のポインタを一応登録しておく(変わらないはずではあるが)
	mHighlightingBaggage = baggage;

	// 荷物の移動先の座標
	sf::Vector2i destination = baggage->GetDestination();

	// ハイライトのテクスチャを作成
	sf::RenderTexture* highlightTexture = new sf::RenderTexture();

	// 各マスに貼り付けるハイライトのスプライトを作成
	sf::Sprite tmpIdleEast, tmpIdleSouth, tmpIdleWest, tmpIdleNorth, tmpHighlight;
	tmpIdleEast.setTexture(*mIdleArrowTextures[Direction::EEast]);
	tmpIdleSouth.setTexture(*mIdleArrowTextures[Direction::ESouth]);
	tmpIdleWest.setTexture(*mIdleArrowTextures[Direction::EWest]);
	tmpIdleNorth.setTexture(*mIdleArrowTextures[Direction::ENorth]);
	tmpHighlight.setTexture(*mPushHighlightTexture);

	// スプライトをテクスチャに貼り付けていく
	highlightTexture->create(static_cast<int>(tmpIdleEast.getGlobalBounds().width * mGame->GetBoardSize().x), static_cast<int>(tmpIdleEast.getGlobalBounds().height * mGame->GetBoardSize().y));

	tmpHighlight.setPosition(tmpHighlight.getGlobalBounds().width * static_cast<float>(destination.x), tmpHighlight.getGlobalBounds().height * static_cast<float>(destination.y));
	highlightTexture->draw(tmpHighlight);
	for (const auto& index : indexes)
	{
		switch (index)
		{
		case 0:
			tmpIdleEast.setPosition(tmpIdleEast.getGlobalBounds().width * static_cast<float>(destination.x), tmpIdleEast.getGlobalBounds().height * static_cast<float>(destination.y));
			highlightTexture->draw(tmpIdleEast);
			break;
		case 1:
			tmpIdleSouth.setPosition(tmpIdleSouth.getGlobalBounds().width * static_cast<float>(destination.x), tmpIdleSouth.getGlobalBounds().height * static_cast<float>(destination.y));
			highlightTexture->draw(tmpIdleSouth);
			break;
		case 2:
			tmpIdleWest.setPosition(tmpIdleWest.getGlobalBounds().width * static_cast<float>(destination.x), tmpIdleWest.getGlobalBounds().height * static_cast<float>(destination.y));
			highlightTexture->draw(tmpIdleWest);
			break;
		case 3:
			tmpIdleNorth.setPosition(tmpIdleNorth.getGlobalBounds().width * static_cast<float>(destination.x), tmpIdleNorth.getGlobalBounds().height * static_cast<float>(destination.y));
			highlightTexture->draw(tmpIdleNorth);
			break;
		default:
			break;
		}
	}

	sf::Texture* tmpTexture = new sf::Texture(highlightTexture->getTexture());
	delete highlightTexture;

	mHighlightSpriteComponent = new SpriteComponent(this, 100, 150);
	mHighlightSpriteComponent->SetTexture(tmpTexture);
}

void GameBoard::ClearPushDirections()
{
	mHighlightingBaggage = nullptr;

	if (mHighlightSpriteComponent != nullptr)
	{
		delete mHighlightSpriteComponent;
		mHighlightSpriteComponent = nullptr;
	}
}

sf::Texture* GameBoard::GetBoardTexture() const
{
	return mBoardSpriteComponent->GetTexture();
}