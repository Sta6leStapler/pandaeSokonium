#include "GameBoard.h"
#include "Game.h"
#include <iostream>
#include <fstream>

GameBoard::GameBoard(Game* game)
	: mState(State::EActive)
	, mPosition(0.0, 0.0)
	, mScale(sf::Vector2f(1.0f, 1.0f))
	, mRotation(0.0f)
	, mGame(game)
	, mBoardName(game->GetFilename(0))
{
	mGame->AddActor(this);

	// SpriteComponentを作成する
	// 床と壁のタイルを用意
	// 読み込んだ盤面データに応じたステージのテクスチャを作成する
	GameBoardComponent* gbc = new GameBoardComponent(this, 100, 150);
	std::string filename = "Assets/Floor.png";
	mTextures.emplace(filename, game->LoadTexture(filename));
	filename = "Assets/Wall.png";
	mTextures.emplace(filename, game->LoadTexture(filename));
	filename = "Assets/Goal.png";
	mTextures.emplace(filename, game->LoadTexture(filename));
	
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

	gbc->SetTexture(tmpTexture);
	mGameBoardComponent = gbc;

	// スケーリングと位置の初期化を行う
	// ウィンドウのサイズ　/ 盤面のサイズ を求める
	// *メモ ... UI等を追加する場合、描画範囲をウィンドウのサイズと置き換える
	// ウィンドウの方が小さければタイルは縮小すべきで、逆なら拡大するべき
	// まずは盤面をウィンドウいっぱいにスケーリングするための比率を求め、幅に余裕がある方に合わせる
	// 横に長ければ横方向に少し縮小でき、縦に長ければ縦方向に縮小できる
	float minScale = std::min((viewArea.second.x - viewArea.first.x) / static_cast<float>(tmpTexture->getSize().x), (viewArea.second.y - viewArea.first.y) / static_cast<float>(tmpTexture->getSize().y));
	mScale = sf::Vector2f(minScale, -minScale);

	// 余白の分中央揃えする
	// *メモ UI等でずれる場合はオフセットを加えておく
	mPosition = sf::Vector2f(((viewArea.second.x - viewArea.first.x) - (static_cast<float>(tmpTexture->getSize().x) * minScale)) / 2.0f, ((viewArea.second.y - viewArea.first.y) - ((viewArea.second.y - viewArea.first.y) - (static_cast<float>(tmpTexture->getSize().y) * minScale)) / 2.0f));
}

GameBoard::~GameBoard()
{
	mGame->RemoveActor(this);
	// コンポーネントを削除する
	delete mGameBoardComponent;
}

void GameBoard::Update(float deltaTime)
{
	if (mState.GetEState() == State::EActive)
	{
		UpdateComponents(deltaTime);

		// このアクター特有の更新処理があれば書く
	}
}

void GameBoard::UpdateComponents(float deltaTime)
{
	mGameBoardComponent->Update(deltaTime);
}

void GameBoard::ProcessInput(const sf::Event::KeyEvent* keyState)
{
	if (mState.GetEState() == State::EActive)
	{
		// アクターが持つ全てのComponentの入力処理を行う
		// どのComponentも特に独自の処理を実装していなければ何もしない
		mGameBoardComponent->ProcessInput(keyState);

		// このアクター特有の振る舞いがあれば書く
		
	}

}

void GameBoard::AddComponent(GameBoardComponent* component)
{
	mGameBoardComponent = component;
}

void GameBoard::RemoveComponent(GameBoardComponent* component)
{
	mGameBoardComponent = nullptr;
}

void GameBoard::Reload()
{
	mBoardName = mGame->GetFilename(0);
	std::vector<std::string> lines = mGame->GetBoardData()[mBoardName];

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
	sf::Vector2f wSize = mGame->GetWindowSize();

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

	mGameBoardComponent->SetTexture(tmpTexture);

	// 2024_09_05 ウィンドウサイズから描画範囲に変更
	BoundingBox viewArea = mGame->GetBoardViewArea();

	// スケーリングと位置の初期化を行う
	// ウィンドウのサイズ　/ 盤面のサイズ を求める
	// ウィンドウの方が小さければタイルは縮小すべきで、逆なら拡大するべき
	// まずは盤面をウィンドウいっぱいにスケーリングするための比率を求め、幅に余裕がある方に合わせる
	// 横に長ければ横方向に少し縮小でき、縦に長ければ縦方向に縮小できる
	float minScale = std::min((viewArea.second.x - viewArea.first.x) / static_cast<float>(tmpTexture->getSize().x), (viewArea.second.y - viewArea.first.y) / static_cast<float>(tmpTexture->getSize().y));
	mScale = sf::Vector2f(minScale, -minScale);

	// 余白の分中央揃えする
	// *メモ UI等でずれる場合はオフセットを加えておく
	mPosition = sf::Vector2f(((viewArea.second.x - viewArea.first.x) - (static_cast<float>(tmpTexture->getSize().x) * minScale)) / 2.0f, ((viewArea.second.y - viewArea.first.y) - ((viewArea.second.y - viewArea.first.y) - (static_cast<float>(tmpTexture->getSize().y) * minScale)) / 2.0f));

}