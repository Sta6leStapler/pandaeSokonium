#include "Baggage.h"

#include "Game.h"
#include "SpriteComponent.h"
#include "MoveAnimationComponent.h"
#include "HUDHelper.h"

#include <iostream>
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
	, mBState(BState::OnFloor)
	, mCurrentHighlightState(HighlightState::Idle)
	, mDestination(sf::Vector2i{ -1, -1 })
	, mTransportingPathes(std::map<int, std::vector<sf::Vector2i>>{})
	, mDetection(false)
{
	mGame->AddActor(this);

	// MoveAnimationComponent を生成して追加
	mMoveAnimation = new MoveAnimationComponent(this, 10);

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
	mBState = BState::OnFloor;
	if (mGame->GetBoardState()[mBoardCoordinate.y][mBoardCoordinate.x] == '.')
	{
		mBState = BState::OnGoal;
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
	mSpriteComponent->SetTexture(mTextures[mBState]);

	// スケーリングと位置の初期化を行う
	// 表示エリアのサイズ　/ 盤面のサイズ を求める
	// 表示エリアの方が小さければタイルは縮小すべきで、逆なら拡大するべき
	float minScale = std::min((viewArea.second.x - viewArea.first.x) / static_cast<float>(mTextures[mBState]->getSize().x * mGame->GetBoardSize().x),
		(viewArea.second.y - viewArea.first.y) / static_cast<float>(mTextures[mBState]->getSize().y * mGame->GetBoardSize().y));

	mScale = sf::Vector2f(minScale, minScale);

	// 余白の分中央揃えする
	// *メモ UI等でずれる場合はオフセットを加えておく
	mPosition = sf::Vector2f
	{
		viewArea.first.x + (viewArea.second.x - viewArea.first.x - static_cast<float>(mTextures[mBState]->getSize().x * mGame->GetBoardSize().x) * mScale.x) / 2.0f + static_cast<float>(mTextures[mBState]->getSize().x * mBoardCoordinate.x) * mScale.x,
		viewArea.first.y + (viewArea.second.y - viewArea.first.y - static_cast<float>(mTextures[mBState]->getSize().x * mGame->GetBoardSize().y) * mScale.y) / 2.0f + static_cast<float>(mTextures[mBState]->getSize().y * mBoardCoordinate.y) * mScale.y
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
			mBState = BState::OnGoal;
			mSpriteComponent->SetTexture(mTextures[BState::OnGoal]);
		}
		else if (mGame->GetHUDHelper()->isDeadlockedBaggage(mBoardCoordinate))
		{
			mBState = BState::Deadlock;
			mSpriteComponent->SetTexture(mTextures[BState::Deadlock]);
		}
		else
		{
			mBState = BState::OnFloor;
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

void Baggage::ProcessInput(const sf::Event* event, const std::map<sf::Keyboard::Key, float>& key_held_duration, const std::map<sf::Keyboard::Key, float>& auto_repeat_timer)
{
	if (mState == IActor::ActorState::EActive)
	{
		// アクターが持つ全てのComponentの入力処理を行う
		// どのComponentも特に独自の処理を実装していなければ何もしない
		ProcessInputComponents(event, key_held_duration, auto_repeat_timer);

		// このアクター特有の振る舞いがあれば書く
		if (mCurrentHighlightState == HighlightState::Idle ||
			mCurrentHighlightState == HighlightState::Highlighting ||
			mCurrentHighlightState == HighlightState::DisplayDirection)
		{
			if (mDetection)
			{
				// もしハイライト表示中なら、移動の前にハイライトを消す
				if (mCurrentHighlightState == HighlightState::Highlighting ||
					mCurrentHighlightState == HighlightState::DisplayDirection)
				{
					mGame->ClearMoveHighlights();
					mCurrentHighlightState = HighlightState::Idle;
				}
			}

			// マウスクリックの処理を追加
			if (event && event->type == sf::Event::MouseButtonPressed)
			{
				// アニメーション中のマウスガード
				// 荷物自身が移動中、またはプレイヤーが移動中の場合は、クリック処理を完全に無視する
				if (mMoveAnimation->IsAnimating() || mGame->GetPlayer()->GetMoveAnimation()->IsAnimating())
				{
					return;
				}

				if (event->mouseButton.button == sf::Mouse::Left)
				{
					sf::Vector2f mousePos(static_cast<float>(event->mouseButton.x), static_cast<float>(event->mouseButton.y));
					sf::Vector2i clickedTile = mGame->ScreenToTileCoords(mousePos);

					if (clickedTile != sf::Vector2i{ -1, -1 })
					{
						// 荷物の現在の状態で処理を分岐
						switch (mCurrentHighlightState)
						{
						case HighlightState::Idle:
							HandleInputIdle(clickedTile);
							break;
						case HighlightState::Highlighting:
							HandleInputHighlighting(clickedTile);
							break;
						case HighlightState::DisplayDirection:
							HandleInputDirection(clickedTile, mousePos);
							break;
						case HighlightState::MovingOnPath:
							// 経路移動中は入力を無視
							break;
						default:
							break;
						}
					}
				}
			}
		}
	}

}

void Baggage::ProcessInputComponents(const sf::Event* event, const std::map<sf::Keyboard::Key, float>& key_held_duration, const std::map<sf::Keyboard::Key, float>& auto_repeat_timer)
{
	for (auto& component : mComponents)
	{
		component->ProcessInput(event, key_held_duration, auto_repeat_timer);
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
		mPosition.x + static_cast<float>(mTextures[mBState]->getSize().x) * mScale.x * static_cast<float>(boardCoordinate.x - mBoardCoordinate.x),
		mPosition.y + static_cast<float>(mTextures[mBState]->getSize().y) * mScale.y * static_cast<float>(boardCoordinate.y - mBoardCoordinate.y)
	);
	mBoardCoordinate = boardCoordinate;
}

void Baggage::HandleInputIdle(const sf::Vector2i& clickedTile)
{
	if (clickedTile == mBoardCoordinate) // 荷物自身がクリックされた
	{
		// 1. 移動可能なマスをすべて計算
		auto transportable = Pathfinder::FindAllTransportable(mBoardCoordinate, mGame->GetBoardState(), mGame->GetPlayer()->GetBoardCoordinate(), mGame->GetBaggagesPos(), mGame->GetBoardSize());
		
		// 移動可能なマスがあれば、ハイライト処理を行い、ハイライト状態に移行する
		if (!transportable.empty())
		{
			// 2. Gameクラスにハイライト描画を依頼
			mGame->SetPushHighlights(transportable, this);

			// 3. 状態を更新
			mCurrentHighlightState = HighlightState::Highlighting;
		}
	}
}

void Baggage::HandleInputHighlighting(const sf::Vector2i& clickedTile)
{
	// Gameクラスからハイライト中のマスリストを取得
	const auto& highlightedTiles = mGame->GetPushHighlightedTiles();

	auto iter = std::find(highlightedTiles.begin(), highlightedTiles.end(), clickedTile);
	// ハイライトされているマスがクリックされたかチェック
	if (iter == highlightedTiles.end()) 
	{
		// ハイライトされていないマスがクリックされた場合
		mGame->ClearPushHighlights(); // ハイライトを消去依頼
		mCurrentHighlightState = HighlightState::Idle;
	}
	else if (iter != highlightedTiles.end())
	{
		mGame->ClearPushHighlights();

		// ハイライトされているマスがクリックされた場合
		// もし移動先の候補が複数ある場合は方向を指定する状態に遷移させる
		// 移動先の候補を探索
		mDestination = sf::Vector2i{ -1, -1 };
		mTransportingPathes.clear();
		mTransportingPathes = Pathfinder::FindAllDirectionTransportPath(mBoardCoordinate, *iter, mGame->GetPlayer()->GetBoardCoordinate(), mGame->GetBoardState(), mGame->GetBaggagesPos(), mGame->GetBoardSize());
		if (mTransportingPathes.size() > 1)
		{
			std::cout << "Baggage : Find " << mTransportingPathes.size() << "'s path!" << std::endl;
			// 運搬先の座標を登録
			mDestination = *iter;
			// Gameクラスに運搬可能な方向の表示を依頼
			std::vector<int> indexes{};
			for (const auto& path : mTransportingPathes)
			{
				indexes.emplace_back(path.first);
			}
			mGame->SetPushDirections(indexes, this);

			// 状態を更新
			mCurrentHighlightState = HighlightState::DisplayDirection;
		}
		else
		{
			// 経路を計算し、移動経路をプレイヤーに送る
			std::vector<sf::Vector2i> path = Pathfinder::FindTransportPath(mBoardCoordinate, *iter, mGame->GetPlayer()->GetBoardCoordinate(), mGame->GetBoardState(), mGame->GetBaggagesPos(), mGame->GetBoardSize());
			mGame->GetPlayer()->InputMovePath(path);
		}
	}
}

void Baggage::HandleInputDirection(const sf::Vector2i& clickedTile, const sf::Vector2f& mousePos)
{
	// クリックされた座標が移動先の座標か？
	if (clickedTile == mDestination)
	{
		// マスの上下左右度の部分がクリックされたか判定
		Direction tmpDir = Direction::EEast;
		sf::Vector2f topLeft = mGame->TileToScreenCoords(clickedTile);
		sf::Vector2f topRight = topLeft + sf::Vector2f{ mGame->GetTileSize().x, 0.0f };
		sf::Vector2f bottomLeft = topLeft + sf::Vector2f{ 0.0f, mGame->GetTileSize().y };
		sf::Vector2f bottomRight = topLeft + mGame->GetTileSize();

		std::cout << "Baggage::HandleInputDirection -> Mouse pos is: (" << mousePos.x << ", " << mousePos.y << ")" << std::endl;
		std::cout << "Baggage::HandleInputDirection -> Bounding box is: (" << 
			topLeft.x << ", " << topLeft.y << "), (" <<
			topRight.x << ", " << topRight.y << "), (" <<
			bottomLeft.x << ", " << bottomLeft.y << "), (" <<
			bottomRight.x << ", " << bottomRight.y << ")" << std::endl;

		// 対角線1 (左上 -> 右下) に対する位置を計算
		float val1 = (bottomRight.x - topLeft.x) * (mousePos.y - topLeft.y) - (bottomRight.y - topLeft.y) * (mousePos.x - topLeft.x);
		// 対角線2 (左下 -> 右上) に対する位置を計算
		float val2 = (topRight.x - bottomLeft.x) * (mousePos.y - bottomLeft.y) - (topRight.y - bottomLeft.y) * (mousePos.x - bottomLeft.x);

		if (val1 >= 0)
		{
			// val1が正: クリック位置は対角線(左上->右下)の下側
			if (val2 >= 0)
			{
				// val2が正: クリック位置は対角線(左下->右上)の上側
				tmpDir = Direction::ESouth; // よって「下の三角形」
				std::cout << "Baggage::HandleInputDirection -> Input ESouth!" << std::endl;
			}
			else
			{
				// val2が負: クリック位置は対角線(左下->右上)の下側
				tmpDir = Direction::EWest; // よって「左の三角形」
				std::cout << "Baggage::HandleInputDirection -> Input EWest!" << std::endl;
			}
		}
		else
		{
			// val1が負: クリック位置は対角線(左上->右下)の上側
			if (val2 >= 0)
			{
				// val2が正: クリック位置は対角線(左下->右上)の上側
				tmpDir = Direction::EEast; // よって「右の三角形」
				std::cout << "Baggage::HandleInputDirection -> Input EEast!" << std::endl;
			}
			else
			{
				// val2が負: クリック位置は対角線(左下->右上)の下側
				tmpDir = Direction::ENorth; // よって「上の三角形」
				std::cout << "Baggage::HandleInputDirection -> Input ENorth!" << std::endl;
			}
		}

		// クリックされた方向から運搬できるなら、
		// その方向から押し込むように運搬の支持をプレイヤーに送る
		if (mTransportingPathes.count(tmpDir))
		{
			mGame->ClearPushHighlights();
			mGame->ClearPushDirections();
			// 経路は既に保持してある
			mGame->GetPlayer()->InputMovePath(mTransportingPathes[tmpDir]);
		}
	}
	else
	{
		// ハイライトされていないマスがクリックされた場合
		mGame->ClearPushDirections(); // ハイライトを消去依頼
		mCurrentHighlightState = HighlightState::Idle;
	}
}