#include "Game.h"

#include "IActor.h"
#include "IComponent.h"
#include "SpriteComponent.h"

#include "GameBoard.h"
#include "BackGround.h"
#include "Baggage.h"
#include "IUIScreen.h"
#include "PauseMenu.h"
#include "MySolution.h"
#include "THUD.h"
#include "HUDHelper.h"

#include <iostream>
#include <filesystem>
#include <fstream>
#include <chrono>
#include <ctime>

Game::Game()
	: mWindow(nullptr)
	, mGameState(GameState::EGamePlay)
	, mPlayer(nullptr)
	, mGameBoard(nullptr)
	, mBaggages(std::vector<Baggage*>{})
	, mHUDHelper(nullptr)
	, mUpdatingActors(false)
	, mIsComplete(false)
	, mWindowSize(1600.0, 900.0)
	, mBoardViewArea(BoundingBox{ sf::Vector2f{ mWindowSize.x - mWindowSize.y, 0.0 }, sf::Vector2f{ mWindowSize.x, mWindowSize.y } })
	, mInputCooldown(0.0f)
	, mBoardSize(sf::Vector2i{ 0, 0 })
	, mBaggageNum(5)
	, mBaggageLimit(1)
	, mRepetition01(1)
	, mRepetition02(10)
	, mRepetition03(0.0)
	, mRepetition04(0.0)
	, mRepetition05(0)
	, mStep(0)
{
	// 盤面をロードする度に更新するのは以下のメンバ変数
	// mBoardSize
	// 現在の盤面サイズ
	// mBaggageNum
	// 現在の盤面の荷物の数
	// mRepetition01
	// mRepetition02
	// mRepetition03
	// mRepetition04
	// mRepetition05
	// 盤面の自動生成を行う際のパラメータ
	// mCurrentKey
	// 現在の盤面のキー (ゲーム起動時からロードされた盤面を読み込むのに使う)
	// mFilenames
	// ゲーム起動時からロードされた盤面のキーのリスト
	// mBoardData
	// ゲーム起動時からロードされた盤面のリスト (進行状態を反映)
	// mInitBoardData
	// ゲーム起動時からロードされた盤面の初期状態リスト
	// mBoardState
	// 現在の盤面のプレイヤーと荷物を除いた地形情報
	// mGoalPos
	// 現在の盤面のゴール位置のリスト
	// mBaggageLimit
	// 現在の盤面の広さと初期壁マスの生成割合から計算される、設置できる荷物の数
	// mInitialPlayerPos
	// 現在の盤面のプレイヤーの初期位置
	// mInitialBaggagePos
	// 現在の盤面の荷物の初期位置
	// mStep
	// 現在の盤面の手数
	// mLogs
	// 現在の盤面の入力のログ
}

bool Game::Initialize()
{
	// ウィンドウの作成
	// 最後2つの引数は画面サイズ固定のためのおまじない
	sf::RenderWindow* window = new sf::RenderWindow(sf::VideoMode(static_cast<unsigned int>(mWindowSize.x), static_cast<unsigned int>(mWindowSize.y)), "SFML Window", sf::Style::Titlebar | sf::Style::Close);
	mWindow = window;
	
	// ファイルの読み込み
	LoadData();

	mClock.restart();

	mTicksCount = mClock.getElapsedTime();

	mStart = std::chrono::system_clock::now();

	return true;
}

void Game::RunLoop()
{
	while (mGameState != GameState::EQuit)
	{
		ProcessInput();
		UpdateGame();
		GenerateOutput();
	}
}

void Game::LoadData()
{
	// guiのテーマをロード
	mGui = std::make_unique<tgui::Gui>(*mWindow);
	mTheme = std::make_unique<tgui::Theme>("Assets/themes/Black.txt");

	// フォントをロードし、初期化
	if (!mFont.loadFromFile("Assets/fonts/arial.ttf"))
	{
		std::cout << "error : failed loading font." << std::endl;
	}
	mInfoTxt.setFont(mFont);
	mInfoTxt.setCharacterSize(mWindow->getSize().y);
	mInfoTxt.setScale(20.0f / static_cast<float>(mWindow->getSize().y), 20.0f / static_cast<float>(mWindow->getSize().y));

	// 盤面データを読み取る
	//*
	std::string filename = "Assets/Boards/default.txt", name = "default";
	mCurrentKey = name;
	mFilenames.push_back(name);
	std::ifstream file(filename);
	if (!file.is_open())
	{
		std::cerr << "Failed to open file" << std::endl;
		return;
	}

	std::vector<std::string> lines;
	std::string line;
	while (std::getline(file, line))
	{
		lines.push_back(line);
	}

	mBoardData.emplace(name, lines);
	mInitBoardData.emplace(name, lines);
	file.close();
	//*/

	// 自動生成の場合
	// MySolution
	/*
	InputBoardData();
	MySolution* gen = new MySolution(mBoardSize, mBaggageNum, mRepetition01, mRepetition02, mRepetition03, mRepetition04, mRepetition05);
	std::vector<std::string> lines = gen->GetBoard();
	delete(gen);
	mCurrentKey = GetDateTime();
	mFilenames.push_back(mCurrentKey);
	mBoardData.emplace(mCurrentKey, lines);
	mInitBoardData.emplace(mCurrentKey, lines);
	//*/

	// 盤面の初期状態をセット
	std::vector<sf::Vector2i> mBoxesPos;
	{
		int i = 0, j = 0;
		std::string tmpstr = "";
		for (const auto& line : lines)
		{
			
			for (const auto& item : line)
			{
				switch (item)
				{
				case ' ':
					tmpstr += ' ';
					break;
				case '#':
					tmpstr += '#';
					break;
				case '$':
					tmpstr += ' ';
					mBoxesPos.push_back(sf::Vector2i(j, i));
					break;
				case '.':
					tmpstr += '.';
					mGoalPos.push_back(sf::Vector2i(j, i));
					break;
				case '*':
					tmpstr += '.';
					mBoxesPos.push_back(sf::Vector2i(j, i));
					mGoalPos.push_back(sf::Vector2i(j, i));
					break;
				case '@':
					tmpstr += ' ';
					mInitialPlayerPos = sf::Vector2i(j, i);
					break;
				case '+':
					tmpstr += '.';
					mInitialPlayerPos = sf::Vector2i(j, i);
					mGoalPos.push_back(sf::Vector2i(j, i));
					break;
				default:
					break;
				}
				j++;
			}
			mBoardState.push_back(tmpstr);
			tmpstr = "";
			i++;
			j = 0;
		}
	}

	// 背景の作成
	// 背景は画面の中央に配置
	new BackGround(this);

	// ボードを作成
	mGameBoard = new GameBoard(this);
	mBoardSize = sf::Vector2i{ static_cast<int>(lines.front().size()), static_cast<int>(lines.size()) };

	// プレイヤーの作成
	mPlayer = new Player(this);

	// 荷物を作成
	// 荷物の初期座標を得る
	for (const auto& item : mBoxesPos)
	{
		mBaggages.emplace_back(new Baggage(this, item));
		mInitialBaggagePos.emplace(mBaggages.back(), item);
	}

	// HUD補助のクラスを宣言
	mHUDHelper = new HUDHelper(this);

	// TODO UI関連のクラスのインスタンス作成の処理を書く
	// HUDのインスタンス作成
	// 盤面の情報を得てから作成する
	// tguiを用いる場合は以下に書く
	// ゲームループ部分の処理は未実装
	new THUD(this, mWindow);

	// 自作のHUDクラスを用いる場合は以下のコメントアウトを戻す
	// new HUD(this);
}

void Game::UnloadData()
{
	// アクターを削除
	while (!mActiveActors.empty())
	{
		delete mActiveActors.back();
	}

	// HUDヘルパーを削除
	delete mHUDHelper;

	// UIスタックをすべて削除
	while (!mUIStack.empty())
	{
		delete mUIStack.back();
		mUIStack.pop_back();
	}

	// テクスチャを削除
	for (auto& item : mTextures)
	{
		delete item.second;
	}
	mTextures.clear();
}

void Game::ProcessInput()
{
	sf::Event event;
	while (mWindow->pollEvent(event))
	{
		switch (event.type)
		{
		case sf::Event::Closed:
			mGameState = GameState::EQuit;
			break;
		}

		// ゲーム全体に関する各種入力処理
		if (mInputCooldown <= 0.0f)
		{
			// Escキーでポーズメニューを開く
			if (sf::Keyboard::isKeyPressed(sf::Keyboard::Escape) && mGameState == GameState::EGamePlay)
			{
				mInputCooldown = 0.13f;
				new PauseMenu(this);
				mGameState = GameState::EPaused;
			}

			// z でundo処理
			if (sf::Keyboard::isKeyPressed(sf::Keyboard::Z))
			{
				mInputCooldown = 0.13f;
				CallUndo();
			}

			// y でredo処理
			if (sf::Keyboard::isKeyPressed(sf::Keyboard::Y))
			{
				mInputCooldown = 0.13f;
				CallRedo();
			}

			// PGUPで最新の状態にする
			if (sf::Keyboard::isKeyPressed(sf::Keyboard::PageUp))
			{
				CallRedoAll();
			}

			// PGDNで初期状態にする
			if (sf::Keyboard::isKeyPressed(sf::Keyboard::PageDown))
			{
				CallReset();
			}

			// Ctrl + rで全てリセット
			if (sf::Keyboard::isKeyPressed(sf::Keyboard::LControl) && sf::Keyboard::isKeyPressed(sf::Keyboard::R))
			{
				mInputCooldown = 0.13f;
				CallRestart();
			}

			// Ctrl + s で現在の盤面をセーブ
			if (sf::Keyboard::isKeyPressed(sf::Keyboard::LControl) && sf::Keyboard::isKeyPressed(sf::Keyboard::S))
			{
				mInputCooldown = 0.13f;
				CallSave();
			}

			// H でヘルプ画面の表示
			if (sf::Keyboard::isKeyPressed(sf::Keyboard::H))
			{
				mInputCooldown = 0.13f;
				DisplayHelpWindow();
			}

			// F5 で盤面のリロード（自動生成の盤面なら新たな盤面の生成）
			if (sf::Keyboard::isKeyPressed(sf::Keyboard::F5))
			{
				mInputCooldown = 0.13f;
				CallReload();
			}
		}

		// ゲームプレイ状態ならアクターの入力処理を行う
		if (mGameState == GameState::EGamePlay)
		{
			// 全てのActorの入力処理を行う
			mUpdatingActors = true;

			for (auto& actor : mActiveActors)
			{
				actor->ProcessInput(&event);
			}

			mUpdatingActors = false;
			// アクターの入力処理はここまで
		}

		// その他の状態ではUIの入力処理を行う
		// ゲームプレイ中に使えるUIは未実装
		if (!mUIStack.empty())
		{
			// 一番手前のレイヤのUIの入力処理のみ行う
			mUIStack.back()->ProcessInput(&event, sf::Mouse::getPosition(*mWindow));
		}
	}
}

void Game::UpdateGame()
{
	// デルタタイムを計算する
	// 最期のフレームから16msだけ待つ
	while (mClock.getElapsedTime().asMilliseconds() - mTicksCount.asMilliseconds() < 16);

	float deltaTime = static_cast<float>(mClock.getElapsedTime().asMilliseconds() - mTicksCount.asMilliseconds()) / 1000.0f;
	if (deltaTime > 0.05f)
	{
		deltaTime = 0.05f;
	}
	mTicksCount = mClock.getElapsedTime();
	mInputCooldown -= deltaTime;

	// 全てのアクターを更新
	mUpdatingActors = true;

	for (auto& actor : mActiveActors)
	{
		actor->Update(deltaTime);
	}
	//mPlayer->Update(deltaTime);
	for (auto& item : mBaggages)
	{
		item->Update(deltaTime);
	}

	// 盤面の更新を行ったので、HUDHelperの内容も更新
	mHUDHelper->Update();

	mUpdatingActors = false;
	// アクターの更新はここまで

	// スタックのUI画面を更新する
	for (auto& ui : mUIStack)
	{
		if (ui->GetState() == IUIScreen::EActive)
		{
			ui->Update(deltaTime);
		}
	}

	// クロージング状態のUI画面をすべて破棄する
	// スタックと同等の処理をするため、リストの末尾から削除をしていく
	for (int i = static_cast<int>(mUIStack.size()) - 1; i >= 0; --i)
	{
		if (mUIStack[i]->GetState() == IUIScreen::EClosing)
		{
			delete mUIStack[i];
			mUIStack.erase(mUIStack.begin() + i);
		}
	}

	// 待機中のアクターを実行可能状態にする
	for (auto& pendingActor : mPendingActors)
	{
		mActiveActors.emplace_back(pendingActor);
	}

	// 死んだアクターを削除する
	std::vector<IActor*> deadActors;
	for (auto actor : mActiveActors)
	{
		if (actor->GetState() == IActor::ActorState::EClosing)
		{
			deadActors.emplace_back(actor);
		}
	}

	for (auto actor : deadActors)
	{
		delete actor;
	}

	// ゲームのクリア判定を呼び出す
	HasComplete();
}

void Game::GenerateOutput()
{
	// 全てのスプライトを持つコンポーネントを描画
	mWindow->clear();

	// 背面から描画
	DrawSprites();

	// UIはゲームオブジェクトの上に描画するのでここに処理を書く
	DrawUI();
	
	mWindow->display();

	// ゲームクリア状態ならリザルト画面を出力
	if (mIsComplete)
	{
		DisplayResult();
	}
}

sf::Texture* Game::LoadTexture(const std::string& fileName)
{
	sf::Texture* tex = new sf::Texture();
	// 同じ名前のテクスチャが既に作成されているか調べる
	auto iter = mTextures.find(fileName);
	if (iter != mTextures.end())
	{
		tex = iter->second;
	}
	else
	{
		// ファイルをロード
		sf::Image* img = new sf::Image();

		if (!img->loadFromFile(fileName.c_str()))
		{
			std::cout << "Failed to load texture file " << fileName.c_str() << std::endl;
			return nullptr;
		}

		// 画像データからテクスチャを作成
		if (!tex->loadFromImage(*img))
		{
			std::cout << "Failed to convert surface to texture for " << fileName.c_str() << std::endl;
			delete img;
			return nullptr;
		}
		delete img;

		mTextures.emplace(fileName.c_str(), tex);
	}
	return tex;
}

void Game::Shutdown()
{
	mWindow->close();
	UnloadData();
	delete mWindow;
}

// アクターの追加と削除
void Game::AddActor(IActor* actor)
{
	// 更新中のアクターがあれば、待機中に追加する
	if (mUpdatingActors)
	{
		mPendingActors.emplace_back(actor);
	}
	else
	{
		mActiveActors.emplace_back(actor);
	}
}

void Game::RemoveActor(IActor* actor)
{
	// 引数のアクターが存在しなければ何もしない
	// 待機中か？
	auto iter = std::find(mPendingActors.begin(), mPendingActors.end(), actor);
	if (iter != mPendingActors.end())
	{
		std::iter_swap(iter, mPendingActors.end() - 1);
		mPendingActors.pop_back();
	}

	// アクターにあるか？
	iter = std::find(mActiveActors.begin(), mActiveActors.end(), actor);
	if (iter != mActiveActors.end())
	{
		std::iter_swap(iter, mActiveActors.end() - 1);
		mActiveActors.pop_back();
	}
}

// スプライトの追加と削除
void Game::AddSprite(SpriteComponent* sprite)
{
	int myDrawOrder = sprite->GetDrawOrder();
	auto iter = mSprites.begin();
	for (;
		iter != mSprites.end();
		++iter)
	{
		if (myDrawOrder < (*iter)->GetDrawOrder())
		{
			break;
		}
	}

	mSprites.insert(iter, sprite);
}

void Game::RemoveSprite(SpriteComponent* sprite)
{
	auto iter = std::find(mSprites.begin(), mSprites.end(), sprite);
	if (iter != mSprites.end())
	{
		mSprites.erase(iter);
	}
}

// スプライトの描画
void Game::DrawSprites()
{
	for (auto& sprite : mSprites)
	{
		sprite->Draw(mWindow);
	}
}

// UIの描画
void Game::DrawUI()
{
	for (const auto& ui : mUIStack)
	{
		ui->Draw(mWindow);
	}
}

// 盤面リストに新しい盤面を追加
void Game::AddBoard(const std::string& key, const std::vector<std::string>& lines)
{
	if (!mInitBoardData.count(key))
	{
		mInitBoardData.emplace(key, lines);
		mBoardData.emplace(key, lines);
		mFilenames.emplace_back(key);
	}
}

void Game::CallUndo()
{
	// 移動のログがあり、ステップ数は1以上か
	if (mLogs.size() > 0 && mStep > 0)
	{
		// 荷物が移動していたか
		if (!mLogs[mStep - 1].isBMoved)
		{
			// プレイヤーの位置を戻す
			if (mStep == 1)
			{
				mPlayer->SetBoardCoordinate(mInitialPlayerPos);
				mPlayer->SetDirection(Player::ESouth);
			}
			else if (mStep > 1)
			{
				mPlayer->SetBoardCoordinate(mLogs[mStep - 1].pCoordinate.first);
				mPlayer->SetDirection(mLogs[mStep - 1].direction2);
			}
		}
		else if (mLogs[mStep - 1].isBMoved)
		{
			// 最後のステップに該当する荷物を探す
			Baggage* ptrBaggage = nullptr;
			for (const auto& item : mBaggages)
			{
				if (item->GetBoardCoordinate() == mLogs[mStep - 1].bCoordinate.second)
				{
					ptrBaggage = item;
					break;
				}
			}

			// プレイヤーと荷物の位置を戻す
			if (mStep == 1)
			{
				mPlayer->SetBoardCoordinate(mInitialPlayerPos);
				mPlayer->SetDirection(Player::ESouth);
				ptrBaggage->SetBoardCoordinate(mInitialBaggagePos[ptrBaggage]);
			}
			else if (mStep > 1)
			{
				mPlayer->SetBoardCoordinate(mLogs[mStep - 1].pCoordinate.first);
				mPlayer->SetDirection(mLogs[mStep - 1].direction2);
				ptrBaggage->SetBoardCoordinate(mLogs[mStep - 1].bCoordinate.first);
			}
		}
		mStep--;
		
	}
}

void Game::CallRedo()
{
	// ログのサイズは現在のステップ数より大きいか
	if (mLogs.size() > mStep)
	{
		// 荷物が移動していたか
		if (!mLogs[mStep].isBMoved)
		{
			// プレイヤーの位置を進める
			mPlayer->SetBoardCoordinate(mLogs[mStep].pCoordinate.second);
			mPlayer->SetDirection(mLogs[mStep].direction2);
		}
		else if (mLogs[mStep].isBMoved)
		{
			// 次ステップに該当する荷物を探す
			Baggage* ptrBaggage = nullptr;
			for (const auto& item : mBaggages)
			{
				if (item->GetBoardCoordinate() == mLogs[mStep].bCoordinate.first)
				{
					ptrBaggage = item;
					break;
				}
			}

			// プレイヤーと荷物の位置を進める
			mPlayer->SetBoardCoordinate(mLogs[mStep].pCoordinate.second);
			mPlayer->SetDirection(mLogs[mStep].direction2);
			ptrBaggage->SetBoardCoordinate(mLogs[mStep].bCoordinate.second);
		}
		mStep++;
	}
}

void Game::CallReset()
{
	while (mStep > 0)
	{
		CallUndo();
	}
}

void Game::CallRedoAll()
{
	while (mLogs.size() > mStep)
	{
		CallRedo();
	}
}

void Game::CallSave()
{
	auto now = std::chrono::system_clock::now();
	std::time_t currentTime = std::chrono::system_clock::to_time_t(now);

	// 文字列に変換
	std::tm localTime;
	localtime_s(&localTime, &currentTime);
	char buffer[80];
	std::strftime(buffer, sizeof(buffer), "%Y%m%d %H%M%S", &localTime);
	std::string timeStr = buffer;

	std::string fileName = "Save/" + timeStr + ".txt";
	std::ofstream outputFile(fileName);

	if (outputFile.is_open())
	{
		for (const auto& str : mInitBoardData[mCurrentKey])
		{
			outputFile << str << std::endl;
		}

		outputFile.close();
	}
	else
	{
		std::cerr << "Error: Unable to open file '" << fileName << "' for writing." << std::endl;
	}
}

void Game::CallReload()
{
	// プレイヤーと荷物と盤面を更新
	// パラメータ設定際に、closeが押されなければ、続きの更新処理を行う
	if (InputBoardData())
	{
		// ログをファイル出力してから全て消す
		OutputLogs();
		ResetParameters();

		// 更新されたGameクラスのメンバ変数を参照して盤面を生成
		MySolution* gen = new MySolution(mBoardSize, mBaggageNum, mRepetition01, mRepetition02, mRepetition03, mRepetition04, mRepetition05);
		std::vector<std::string> lines = gen->GetBoard();
		delete(gen);

		// 生成された盤面のキーは時刻を文字列に変換したものにする
		mCurrentKey = GetDateTime();
		mFilenames.emplace_back(mCurrentKey);
		mBoardData.emplace(mCurrentKey, lines);
		mInitBoardData.emplace(mCurrentKey, lines);
		mBaggageLimit = GetBaggageNumLimit(mBoardSize, mRepetition03);

		// 盤面の初期状態をセット
		std::vector<sf::Vector2i> mBoxesPos;
		{
			int i = 0, j = 0;
			std::string tmpstr = "";
			for (const auto& line : lines)
			{

				for (const auto& item : line)
				{
					switch (item)
					{
					case ' ':
						tmpstr += ' ';
						break;
					case '#':
						tmpstr += '#';
						break;
					case '$':
						tmpstr += ' ';
						mBoxesPos.push_back(sf::Vector2i(j, i));
						break;
					case '.':
						tmpstr += '.';
						mGoalPos.push_back(sf::Vector2i(j, i));
						break;
					case '*':
						tmpstr += '.';
						mBoxesPos.push_back(sf::Vector2i(j, i));
						mGoalPos.push_back(sf::Vector2i(j, i));
						break;
					case '@':
						tmpstr += ' ';
						mInitialPlayerPos = sf::Vector2i(j, i);
						break;
					case '+':
						tmpstr += '.';
						mInitialPlayerPos = sf::Vector2i(j, i);
						mGoalPos.push_back(sf::Vector2i(j, i));
						break;
					default:
						break;
					}
					j++;
				}
				mBoardState.push_back(tmpstr);
				tmpstr = "";
				i++;
				j = 0;
			}
		}

		// ボードを再構築
		mGameBoard->Reload();

		// プレイヤーを再構築
		mPlayer->Reload();

		// 荷物を再構築
		while (!mBaggages.empty())
		{
			delete mBaggages.back();
		}
		mBaggages.clear();

		for (const auto& item : mBoxesPos)
		{
			new Baggage(this, item);
			mInitialBaggagePos.emplace(mBaggages.back(), item);
		}

		// HUDHelperを再構築
		mHUDHelper = new HUDHelper(this);

		mStart = std::chrono::system_clock::now();
	}
}

void Game::CallRestart()
{
	// ログをファイル出力してから全て消す
	OutputLogs();
	ResetParameters();

	mBoardData[mCurrentKey] = mInitBoardData[mCurrentKey];

	// ボードを再構築
	mGameBoard->Reload();

	// プレイヤーを再構築
	mPlayer->Reload();

	// 荷物を再構築
	for (const auto& baggage : mInitialBaggagePos)
	{
		baggage.first->SetBoardCoordinate(baggage.second);
	}

	mStart = std::chrono::system_clock::now();
}

void Game::RemoveRedo()
{
	// 現在のステップ数より大きいステップのログをすべて削除
	std::vector<Log> subLogs(mLogs.begin() + mStep, mLogs.end());
	auto it = mLogs.begin();
	while (it != mLogs.end()) {
		if (it - mLogs.begin() >= mStep)
		{
			it = mLogs.erase(it);
		}
		else 
		{
			++it;
		}
	}

	// 手をやり直す場合、これまでの経路を正解の経路から派生した木構造にする
	if (!subLogs.empty())
	{
		if (mLogs.empty())
		{
			mLogs = subLogs;
		}
		else
		{
			mLogs.back().thread.push_back(subLogs);
		}
	}
}

void Game::AddLog(const sf::Vector2i& playerPos1, const sf::Vector2i& playerPos2, const Player::Direction& direction1, const Player::Direction& direction2)
{
	// 現在の日付と時刻を取得
	auto now = std::chrono::system_clock::now();
	auto now_ms = std::chrono::time_point_cast<std::chrono::milliseconds>(now);
	auto ms = now_ms.time_since_epoch().count() % 1000;  // ミリ秒部分を取得

	// 年月日と時刻を文字列にフォーマット
	std::time_t now_time = std::chrono::system_clock::to_time_t(now);
	std::tm timeinfo;
	localtime_s(&timeinfo, &now_time);
	char buffer[24];
	std::strftime(buffer, sizeof(buffer), "%Y-%m-%d-%H-%M-%S", &timeinfo);

	// ミリ秒を3桁表示するためにパディングを追加
	std::string ms_str = std::to_string(ms);
	ms_str = std::string(3 - ms_str.length(), '0') + ms_str;

	// ミリ秒を追加して文字列を表示
	std::string datetime_str(buffer);
	datetime_str += "." + ms_str;

	Log tmpLog = { std::make_pair(playerPos1, playerPos2), std::make_pair(sf::Vector2i{ 0, 0 }, sf::Vector2i{ 0, 0 }), false, direction1, direction2, datetime_str };
	mLogs.push_back(tmpLog);
}

void Game::AddLog(const sf::Vector2i& playerPos1, const sf::Vector2i& playerPos2, const sf::Vector2i& baggagePos1, const sf::Vector2i& baggagePos2, const Player::Direction& direction1, const Player::Direction& direction2)
{
	// 現在の日付と時刻を取得
	auto now = std::chrono::system_clock::now();
	auto now_ms = std::chrono::time_point_cast<std::chrono::milliseconds>(now);
	auto ms = now_ms.time_since_epoch().count() % 1000;  // ミリ秒部分を取得

	// 年月日と時刻を文字列にフォーマット
	std::time_t now_time = std::chrono::system_clock::to_time_t(now);
	std::tm timeinfo;
	localtime_s(&timeinfo, &now_time);
	char buffer[24];
	std::strftime(buffer, sizeof(buffer), "%Y-%m-%d-%H-%M-%S", &timeinfo);

	// ミリ秒を3桁表示するためにパディングを追加
	std::string ms_str = std::to_string(ms);
	ms_str = std::string(3 - ms_str.length(), '0') + ms_str;

	// ミリ秒を追加して文字列を表示
	std::string datetime_str(buffer);
	datetime_str += "." + ms_str;

	Log tmpLog = { std::make_pair(playerPos1, playerPos2), std::make_pair(baggagePos1, baggagePos2), true, direction1, direction2, datetime_str };
	mLogs.push_back(tmpLog);
}

void Game::OutputLogs()
{
	std::string datetime_str = GetDateTime(), filename = "Assets/Logs/" + datetime_str + ".txt";

	// ログを出力
	std::ofstream outFile(filename);

	if (outFile.is_open())
	{
		for (const auto& row : mInitBoardData[mGameBoard->GetBoardName()])
		{
			outFile << row << std::endl;
		}
		outFile << ConvertLogToStr(mLogs, 0);
		outFile.close();
	}
}

std::string Game::GetDateTime()
{
	// 日付と時刻を取得
	// 現在の日付と時刻を取得
	auto now = std::chrono::system_clock::now();
	std::time_t now_time = std::chrono::system_clock::to_time_t(now);

	// 年月日と時刻を文字列にフォーマット
	std::tm timeinfo;
	localtime_s(&timeinfo, &now_time);
	char buffer[20];
	std::strftime(buffer, sizeof(buffer), "%Y-%m-%d-%H-%M-%S", &timeinfo);
	std::string datetime_str(buffer);

	return datetime_str;
}

std::string Game::ConvertLogToStr(const std::vector<Log>& logs, const unsigned long long& current)
{
	// currentは現在のステップ数
	std::string result = "";

	result += "( ";
	for (unsigned long long i = 0; i < logs.size(); i++)
	{
		result += std::to_string(current + i) + "{ " + logs[i].time + " }" + " : [" + std::to_string(logs[i].pCoordinate.first.x) + ", " + std::to_string(logs[i].pCoordinate.first.y) + "] ";
		result += "[" + std::to_string(logs[i].pCoordinate.second.x) + ", " + std::to_string(logs[i].pCoordinate.second.y) + "] ";
		if (logs[i].isBMoved)
		{
			result += "[" + std::to_string(logs[i].bCoordinate.first.x) + ", " + std::to_string(logs[i].bCoordinate.first.y) + "] ";
			result += "[" + std::to_string(logs[i].bCoordinate.second.x) + ", " + std::to_string(logs[i].bCoordinate.second.y) + "] ";
		}

		// 分岐した部分は、分岐地点でカットし、全ての分岐を丸括弧で囲ってカンマ区切りで並べる
		if (!logs[i].thread.empty())
		{
			for (const auto& log : logs[i].thread)
			{
				result += " " + ConvertLogToStr(log, current);
			}
		}
	}
	result += " ) ";

	return result;
}

void Game::HasComplete()
{
	// 全ての荷物がゴール上にあるか調べる
	// 1つでもゴール上にないものがあれば、ゲームを継続する
	mIsComplete = true;
	for (const auto& item : mBaggages)
	{
		auto it = std::find(mGoalPos.begin(), mGoalPos.end(), item->GetBoardCoordinate());

		if (it == mGoalPos.end())
		{
			mIsComplete = false;
			break;
		}
	}
}

void Game::DisplayResult()
{
	bool isChildWindowOpened = true;
	bool isPlayLog = false;
	// 表示用のウィンドウを作成
	auto child = tgui::ChildWindow::create();
	child->setRenderer(mTheme->getRenderer("ChildWindow"));
	child->setClientSize({ 960, 540 });
	child->setPosition(420, 80);
	child->setTitle("Notice");
	child->onClose([&isChildWindowOpened]() {
		isChildWindowOpened = false;
		});
	mGui->add(child);

	// ListBoxでテキストを表示
	auto listBox = tgui::ListBox::create();
	listBox->setRenderer(mTheme->getRenderer("ListBox"));
	listBox->setSize(960, 500);
	listBox->setItemHeight(32);
	listBox->setPosition(0, 0);
	listBox->setTextSize(20);
	listBox->addItem("Result");
	listBox->addItem("Steps : " + std::to_string(mStep));
	long time = static_cast<long>(GetSecTime());
	std::stringstream m{}, s{};
	m << std::setw(2) << std::setfill('0') << time / 60;
	s << std::setw(2) << std::setfill('0') << time % 60;
	listBox->addItem("Time : " + m.str() + ":" + s.str());
	listBox->addItem("");
	listBox->addItem("");
	child->add(listBox);

	// 入力終了用ボタン
	auto exitButton = tgui::Button::create("Close");
	exitButton->setRenderer(mTheme->getRenderer("Button"));
	exitButton->setSize(120, 30);
	exitButton->setPosition(static_cast<int>(child->getSize().x - exitButton->getSize().x) - 10, static_cast<int>(listBox->getSize().y) + 5);
	exitButton->onPress([&]() {
		std::cout << "Exit child window action triggered!" << std::endl;
		isChildWindowOpened = false;
		});
	child->add(exitButton);

	// プレイ履歴を再生するモードに移るボタン
	auto playLogButton = tgui::Button::create("Play Log");
	playLogButton->setRenderer(mTheme->getRenderer("Button"));
	playLogButton->setSize(120, 30);
	playLogButton->setPosition(static_cast<int>(child->getSize().x - exitButton->getSize().x * 2.0f) - 30, static_cast<int>(listBox->getSize().y) + 5);
	playLogButton->onPress([&]() {
		std::cout << "Play Log action triggered!" << std::endl;
		isChildWindowOpened = false;
		isPlayLog = true;
		});
	child->add(playLogButton);

	// ゲームループを停止
	mWindow->setActive(false);

	// デルタタイム関連
	sf::Time ticksCount = mClock.getElapsedTime();

	// 入力と更新とウィンドウの終了処理
	// ウィンドウが閉じるまでループ
	while (isChildWindowOpened)
	{
		// 60FPSに合わせて遅延をかける
		while (mClock.getElapsedTime().asMilliseconds() - ticksCount.asMilliseconds() < 16);

		float deltaTime = static_cast<float>(mClock.getElapsedTime().asMilliseconds() - ticksCount.asMilliseconds()) / 1000.0f;
		if (deltaTime > 0.05f)
		{
			deltaTime = 0.05f;
		}
		ticksCount = mClock.getElapsedTime();

		// イベントキューが存在する場合、それに応じた処理を全て行う
		sf::Event event;
		while (mWindow->pollEvent(event))
		{
			if (event.type == sf::Event::Closed ||
				event.key.code == sf::Keyboard::Escape ||
				event.key.code == sf::Keyboard::Enter)
			{
				isChildWindowOpened = false;
			}

			mGui->handleEvent(event);
		}

		mWindow->clear();

		// 背面から描画
		for (auto& sprite : mSprites)
		{
			sprite->Draw(mWindow);
		}

		// UIはゲームオブジェクトの上に描画するのでここに処理を書く
		for (const auto& ui : mUIStack)
		{
			ui->Draw(mWindow);
		}

		mGui->draw(); // TGUIの描画
		mWindow->display();
	}

	child->close();

	if (isPlayLog)
	{
		DisplayPlayLogs(mCurrentKey);
	}

	// 1手戻す
	CallUndo();

	// ゲームループを再開
	mWindow->setActive(true);
}

bool Game::InputBoardData()
{
	bool result = true;
	bool isChildWindowOpened = true;
	// 入力用のウィンドウを作成
	auto child = tgui::ChildWindow::create();
	child->setRenderer(mTheme->getRenderer("ChildWindow"));	
	child->setClientSize({ 960, 540 });
	child->setPosition(420, 80);
	child->setTitle("Input Prompt");
	child->onClose([&isChildWindowOpened]() {
		isChildWindowOpened = false;
		});
	mGui->add(child);

	// テキスト
	struct BoardInfo
	{
		std::string mName;
		double mMin;
		double mMax;
		double mInitialValue;
		sf::Vector2i mPos;
		bool mIsInteger;
	};
	std::vector<BoardInfo> textInfoes(8, BoardInfo{ "", 0.0, 0.0, 0.0, sf::Vector2i{ 0, 0 } });
	textInfoes[0] = BoardInfo{ "width : ", static_cast<double>(mSizeMin.x), static_cast<double>(mSizeMax.x), static_cast<double>(mBoardSize.x), sf::Vector2i{ 500, 12 }, true };
	textInfoes[1] = BoardInfo{ "height : ", static_cast<double>(mSizeMin.y), static_cast<double>(mSizeMax.y), static_cast<double>(mBoardSize.y), sf::Vector2i{ 500, 48 }, true };
	textInfoes[2] = BoardInfo{ "baggage : ", 1.0, static_cast<double>(GetBaggageNumLimit(mBoardSize)), static_cast<double>(mBaggageNum), sf::Vector2i{500, 84}, true};
	textInfoes[3] = BoardInfo{ "number of times reset : ", 0.0, 32.0, static_cast<double>(mRepetition01), sf::Vector2i{ 500, 120 }, true };
	textInfoes[4] = BoardInfo{ "number of times transportation : ", 1.0, 256.0, static_cast<double>(mRepetition02), sf::Vector2i{ 500, 156 }, true };
	textInfoes[5] = BoardInfo{ "wall tile rate : ", 0.0, mWallRateLimit, mRepetition03, sf::Vector2i{ 500, 192 }, false };
	textInfoes[6] = BoardInfo{ "visited tile rate : ", 0.0, mVisitedRateLimit, mRepetition04, sf::Vector2i{ 500, 228 }, false };
	textInfoes[7] = BoardInfo{ "evaluation function : ", static_cast<double>(mEvaluateFancIndexRange.first), static_cast<double>(mEvaluateFancIndexRange.second), static_cast<double>(mRepetition05), sf::Vector2i{ 500, 264 }, true };
	auto listBox = tgui::ListBox::create();
	listBox->setRenderer(mTheme->getRenderer("ListBox"));
	listBox->setSize(960, 500);
	listBox->setItemHeight(36);
	listBox->setPosition(0, 0);
	for (int i = 0, size = static_cast<int>(textInfoes.size()); i < size; ++i)
	{
		listBox->addItem(textInfoes[i].mName, std::to_string(i));
	}
	child->add(listBox);

	// 入力用のスライダーとテキストボックスの追加
	std::map<int, std::pair<tgui::Slider::Ptr, tgui::EditBox::Ptr>> inputers{};
	int index = 0;
	for (const auto& textInfo : textInfoes)
	{
		// スライダーの追加
		auto slider = tgui::Slider::create();
		slider->setRenderer(mTheme->getRenderer("Slider"));
		slider->setPosition(textInfo.mPos.x, textInfo.mPos.y);
		slider->setSize(400, 18);
		slider->setValue(static_cast<float>(textInfo.mInitialValue));
		slider->setMinimum(static_cast<float>(textInfo.mMin));
		slider->setMaximum(static_cast<float>(textInfo.mMax));
		// 入力を整数と実数で分ける
		if (textInfo.mIsInteger)
		{
			slider->setStep(1.0f);
		}
		else
		{
			slider->setStep(0.001f);
		}
		child->add(slider);

		// テキストボックスの追加
		auto editBox = tgui::EditBox::create();
		editBox->setRenderer(mTheme->getRenderer("EditBox"));
		editBox->setSize(80, 25);
		editBox->setTextSize(18);
		editBox->setPosition(textInfo.mPos.x - 100, textInfo.mPos.y);
		child->add(editBox);
		if (textInfo.mIsInteger)
		{
			editBox->setText(std::to_string(static_cast<int>(textInfo.mInitialValue)));

		}
		else
		{
			editBox->setText(std::to_string(std::floor(textInfo.mInitialValue * 1000.0) / 1000.0));
		}

		// スライダーとテキストボックスを同期
		inputers.emplace(index, std::pair<tgui::Slider::Ptr, tgui::EditBox::Ptr>{ slider, editBox });
		SyncSliderWithEditBox(inputers[index].first, inputers[index].second, textInfo.mIsInteger);

		++index;
	}

	// 入力終了用ボタン
	auto exitButton = tgui::Button::create("Enter");
	exitButton->setRenderer(mTheme->getRenderer("Button"));
	exitButton->setSize(120, 30);
	exitButton->setPosition(static_cast<int>(child->getSize().x - exitButton->getSize().x) - 10, static_cast<int>(listBox->getSize().y) + 5);
	exitButton->onPress([&]() {
		std::cout << "Exit child window action triggered!" << std::endl;
		isChildWindowOpened = false;
		});
	child->add(exitButton);

	// 入力キャンセル用ボタン
	auto canselButton = tgui::Button::create("Cansel");
	canselButton->setRenderer(mTheme->getRenderer("Button"));
	canselButton->setSize(120, 30);
	canselButton->setPosition(static_cast<int>(child->getSize().x - exitButton->getSize().x * 2.0) - 15, static_cast<int>(listBox->getSize().y) + 5);
	canselButton->onPress([&]() {
		std::cout << "Cansel input action triggered!" << std::endl;
		isChildWindowOpened = false;
		result = false;
		});
	child->add(canselButton);

	// デルタタイム関連
	sf::Time ticksCount = mClock.getElapsedTime();

	// 入力と更新とウィンドウの終了処理
	// ウィンドウが閉じるまでループ
	while (isChildWindowOpened)
	{
		// 60FPSに合わせて遅延をかける
		while (mClock.getElapsedTime().asMilliseconds() - ticksCount.asMilliseconds() < 16);

		float deltaTime = static_cast<float>(mClock.getElapsedTime().asMilliseconds() - ticksCount.asMilliseconds()) / 1000.0f;
		if (deltaTime > 0.05f)
		{
			deltaTime = 0.05f;
		}
		ticksCount = mClock.getElapsedTime();

		// 荷物の数は盤面サイズとwall rateによって上限が変わるので、更新する
		int tmpBaggageLimit = GetBaggageNumLimit(sf::Vector2i{ static_cast<int>(inputers[0].first->getValue()), static_cast<int>(inputers[1].first->getValue()) }, inputers[5].first->getValue());
		if (static_cast<int>(inputers[2].first->getValue()) > tmpBaggageLimit)
		{
			inputers[2].first->setValue(static_cast<float>(tmpBaggageLimit));
		}
		textInfoes[2].mMax = static_cast<double>(tmpBaggageLimit);
		inputers[2].first->setMaximum(static_cast<float>(tmpBaggageLimit));

		// ListBoxの内容を更新
		textInfoes[0].mName = "width : " + std::to_string(static_cast<int>(inputers[0].first->getValue()));
		textInfoes[1].mName = "height : " + std::to_string(static_cast<int>(inputers[1].first->getValue()));
		textInfoes[2].mName = "baggage : " + std::to_string(static_cast<int>(inputers[2].first->getValue()));
		textInfoes[3].mName = "number of times reset : " + std::to_string(static_cast<int>(inputers[3].first->getValue()));
		textInfoes[4].mName = "number of times transportation : " + std::to_string(static_cast<int>(inputers[4].first->getValue()));
		textInfoes[5].mName = "wall tile rate : " + std::to_string(inputers[5].first->getValue());
		textInfoes[6].mName = "visited tile rate : " + std::to_string(inputers[6].first->getValue());
		textInfoes[7].mName = "evaluation function : " + std::to_string(static_cast<int>(inputers[7].first->getValue()));

		// イベントキューが存在する場合、それに応じた処理を全て行う
		sf::Event event;
		while (mWindow->pollEvent(event))
		{
			if (event.type == sf::Event::Closed)
			{
				isChildWindowOpened = false;
			}

			mGui->handleEvent(event);
		}

		mWindow->clear();

		// 背面から描画
		for (auto& sprite : mSprites)
		{
			sprite->Draw(mWindow);
		}

		// UIはゲームオブジェクトの上に描画するのでここに処理を書く
		for (const auto& ui : mUIStack)
		{
			ui->Draw(mWindow);
		}

		mGui->draw(); // TGUIの描画
		mWindow->display();
	}

	// 入力をキャンセルしないのであれば、数値を更新
	if (result)
	{
		mBoardSize = sf::Vector2i{ static_cast<int>(inputers[0].first->getValue()), static_cast<int>(inputers[1].first->getValue()) };
		mBaggageNum = static_cast<int>(inputers[2].first->getValue());
		mRepetition01 = static_cast<int>(inputers[3].first->getValue());
		mRepetition02 = static_cast<int>(inputers[4].first->getValue());
		mRepetition03 = inputers[5].first->getValue();
		mRepetition04 = inputers[6].first->getValue();
		mRepetition05 = static_cast<int>(inputers[7].first->getValue());
	}

	child->close();

	// ゲームループを再開
	mWindow->setActive(true);

	return result;
}

void Game::SyncSliderWithEditBox(tgui::Slider::Ptr slider, tgui::EditBox::Ptr editBox, const bool& isInteger)
{
	if (isInteger)
	{
		slider->onValueChange([editBox](float newValue)
			{
				editBox->setText(std::to_string(static_cast<int>(newValue)));
			});

		editBox->onReturnKeyPress([slider, editBox](const tgui::String& text)
			{
				try
				{
					int value = std::stoi(editBox->getText().toStdString()); // テキストを整数に変換
					if (value >= static_cast<int>(slider->getMinimum()) && value <= static_cast<int>(slider->getMaximum()))
					{
						slider->setValue(static_cast<float>(value)); // 値をSliderに反映
					}
					else
					{
						// 範囲外の場合、元のSliderの値を表示
						editBox->setText(std::to_string(static_cast<int>(slider->getValue())));
					}
				}
				catch (const std::exception&)
				{
					// 数値以外の入力があった場合、元のSliderの値を表示
					editBox->setText(std::to_string(static_cast<int>(slider->getValue())));
				}
			});
	}
	else
	{
		slider->onValueChange([editBox](float newValue)
			{
				editBox->setText(std::to_string(newValue));
			});

		editBox->onReturnKeyPress([slider, editBox](const tgui::String& text)
			{
				try
				{
					float value = std::stof(editBox->getText().toStdString()); // テキストを整数に変換
					if (value >= slider->getMinimum() && value <= slider->getMaximum())
					{
						slider->setValue(value); // 値をSliderに反映
					}
					else 
					{
						// 範囲外の場合、元のSliderの値を表示
						editBox->setText(std::to_string(slider->getValue()));
					}
				}
				catch (const std::exception&)
				{
					// 数値以外の入力があった場合、元のSliderの値を表示
					editBox->setText(std::to_string(slider->getValue()));
				}
			});
	}
}

void Game::DisplayHelpWindow()
{
	// テキストデータの読み込み
	std::string filename = "Assets/help.txt";
	std::ifstream file(filename);
	if (!file.is_open())
	{
		std::cerr << "Failed to open file" << std::endl;
		return;
	}
	std::vector<std::string> helpTexts;
	std::string line = "";
	while (std::getline(file, line))
	{
		helpTexts.emplace_back(line);
	}
	
	file.close();

	bool isChildWindowOpened = true;
	// 表示用のウィンドウを作成
	auto child = tgui::ChildWindow::create();
	child->setRenderer(mTheme->getRenderer("ChildWindow"));
	child->setClientSize({ 960, 540 });
	child->setPosition(420, 80);
	child->setTitle("How to play");
	child->onClose([&isChildWindowOpened]() {
		isChildWindowOpened = false;
		});
	mGui->add(child);

	// ListBoxでテキストを表示
	auto listBox = tgui::ListBox::create();
	listBox->setRenderer(mTheme->getRenderer("ListBox"));
	listBox->setSize(960, 500);
	listBox->setItemHeight(32);
	listBox->setPosition(0, 0);
	listBox->setTextSize(20);
	for (const auto& row : helpTexts)
	{
		listBox->addItem(row);
	}
	child->add(listBox);

	// 入力終了用ボタン
	auto exitButton = tgui::Button::create("Close");
	exitButton->setRenderer(mTheme->getRenderer("Button"));
	exitButton->setSize(120, 30);
	exitButton->setPosition(static_cast<int>(child->getSize().x - exitButton->getSize().x) - 10, static_cast<int>(listBox->getSize().y) + 5);
	exitButton->onPress([&]() {
		std::cout << "Exit child window action triggered!" << std::endl;
		isChildWindowOpened = false;
		});
	child->add(exitButton);

	// ゲームループを停止
	mWindow->setActive(false);

	// デルタタイム関連
	sf::Time ticksCount = mClock.getElapsedTime();

	// 入力と更新とウィンドウの終了処理
	// ウィンドウが閉じるまでループ
	while (isChildWindowOpened)
	{
		// 60FPSに合わせて遅延をかける
		while (mClock.getElapsedTime().asMilliseconds() - ticksCount.asMilliseconds() < 16);

		float deltaTime = static_cast<float>(mClock.getElapsedTime().asMilliseconds() - ticksCount.asMilliseconds()) / 1000.0f;
		if (deltaTime > 0.05f)
		{
			deltaTime = 0.05f;
		}
		ticksCount = mClock.getElapsedTime();

		// イベントキューが存在する場合、それに応じた処理を全て行う
		sf::Event event;
		while (mWindow->pollEvent(event))
		{
			if (event.type == sf::Event::Closed ||
				event.key.code == sf::Keyboard::Escape ||
				event.key.code == sf::Keyboard::Enter)
			{
				isChildWindowOpened = false;
			}

			mGui->handleEvent(event);
		}

		mWindow->clear();

		// 背面から描画
		for (auto& sprite : mSprites)
		{
			sprite->Draw(mWindow);
		}

		// UIはゲームオブジェクトの上に描画するのでここに処理を書く
		for (const auto& ui : mUIStack)
		{
			ui->Draw(mWindow);
		}

		mGui->draw(); // TGUIの描画
		mWindow->display();
	}

	child->close();

	// ゲームループを再開
	mWindow->setActive(true);
}

void Game::SelectBoards()
{
	// 盤面リストの読み込みを行う
	struct Contents
	{
		std::vector<std::string> mBoard;
		sf::Texture mTexture;
	};
	std::map<std::string, Contents> fileContents{};

	// 参照する盤面リストのディレクトリを指定
	std::filesystem::directory_iterator iter("Assets/Boards"), end;
	sf::Vector2i tileSize{ mTextures["Assets/Floor.png"]->getSize() };
	std::error_code err;

	try
	{
		// ディレクトリ内のすべてのエントリをチェック
		for (; iter != end && !err; iter.increment(err))
		{
			// エントリがファイルであり、拡張子が.txtの場合
			const std::filesystem::directory_entry entry = *iter;
			if (entry.is_regular_file() && entry.path().extension() == ".txt")
			{
				std::ifstream file(entry.path());
				if (file)
				{
					std::vector<std::string> lines;
					std::string line;
					sf::Vector2u totalSize{ 0, 0 };

					// ファイルを1行ずつ読み込む
					while (std::getline(file, line))
					{
						lines.push_back(line);
						if (totalSize.x < static_cast<unsigned int>(line.length()))
						{
							totalSize.x = static_cast<unsigned int>(line.length());
						}
					}
					totalSize.y = static_cast<unsigned int>(lines.size());

					// 読みこんだ内容から、盤面のテクスチャを作成
					sf::Texture tmpBoardTex{};

					// RenderTextureを作成
					sf::RenderTexture renderTexture{};
					if (!renderTexture.create(totalSize.x * tileSize.x, totalSize.y * tileSize.y))
					{
						std::cerr << "Failed to create render texture: " << entry.path() << "\n";
					}
					renderTexture.clear(sf::Color::Transparent); // 背景を透明に

					// スプライトをrender textureに貼り付けていき、タイル画像を配置
					for (int y = 0; y < static_cast<int>(totalSize.y); ++y)
					{
						for (int x = 0; x < line.length(); ++x)
						{
							std::vector<sf::Texture*> currentTex{};
							switch (lines[y][x])
							{
							case ' ':
								currentTex.emplace_back(mTextures["Assets/Floor.png"]);
								break;
							case '#':
								currentTex.emplace_back(mTextures["Assets/Wall.png"]);
								break;
							case '.':
								currentTex.emplace_back(mTextures["Assets/Goal.png"]);
								break;
							case '$':
								currentTex.emplace_back(mTextures["Assets/Floor.png"]);
								currentTex.emplace_back(mTextures["Assets/Box.png"]);
								break;
							case '*':
								currentTex.emplace_back(mTextures["Assets/Goal.png"]);
								currentTex.emplace_back(mTextures["Assets/ShinyBox.png"]);
								break;
							case '@':
								currentTex.emplace_back(mTextures["Assets/Floor.png"]);
								currentTex.emplace_back(mTextures["Assets/playerN.png"]);
								break;
							case '+':
								currentTex.emplace_back(mTextures["Assets/Goal.png"]);
								currentTex.emplace_back(mTextures["Assets/playerN.png"]);
								break;
							default:
								break;
							}

							for (auto& tmpTex : currentTex)
							{
								sf::Sprite tmpSprite(*tmpTex);
								tmpSprite.setPosition(static_cast<float>(x * tileSize.x), static_cast<float>(y * tileSize.y));
								renderTexture.draw(tmpSprite);
							}
						}
					}
					// 貼り付けたスプライトをrender textureに適用
					renderTexture.display();

					// 完成したテクスチャを取得
					const sf::Texture& combinedTexture = renderTexture.getTexture();

					// ファイル名（拡張子なし）をキーとして格納
					fileContents[entry.path().stem().string()] = Contents{ lines, combinedTexture };

					// もしゲームクラスに存在しない盤面の場合、追加処理を行う
					if (!mInitBoardData.count(entry.path().stem().string()))
					{
						mInitBoardData.emplace(entry.path().stem().string(), lines);
						mBoardData.emplace(entry.path().stem().string(), lines);
						mFilenames.emplace_back(entry.path().stem().string());
					}
				}
				else 
				{
					std::cerr << "Failed to open file: " << entry.path() << "\n";
				}
			}
		}
	}
	catch (const std::exception& e) 
	{
		std::cerr << "Error while accessing directory: " << e.what() << "\n";
	}

	std::string selectedBoardName{};
	bool isChildWindowOpened = true;
	// 表示用のウィンドウを作成
	auto child = tgui::ChildWindow::create();
	child->setRenderer(mTheme->getRenderer("ChildWindow"));
	child->setClientSize({ 960, 540 });
	child->setPosition(420, 80);
	child->setTitle("Board manager");
	child->onClose([&isChildWindowOpened]() {
		isChildWindowOpened = false;
		});
	mGui->add(child);

	// ListBoxでテキストを表示
	auto listBox = tgui::ListBox::create();
	listBox->setRenderer(mTheme->getRenderer("ListBox"));
	listBox->setSize(240, 500);
	listBox->setItemHeight(24);
	listBox->setPosition(0, 0);
	listBox->setTextSize(20);
	listBox->setAutoScroll(false);
	for (const auto& item : fileContents)
	{
		listBox->addItem(item.first, item.first);
	}
	child->add(listBox);

	// Pictureウィジェットを作成して画像表示用に追加
	BoundingBox childPictureBB{ sf::Vector2i{ static_cast<int>(listBox->getPosition().x + listBox->getSize().x), static_cast<int>(listBox->getPosition().y) },
		sf::Vector2i{ static_cast<int>(child->getSize().x), static_cast<int>(listBox->getPosition().y + listBox->getSize().y)}};
	auto picture = tgui::Picture::create();
	picture->setSize(childPictureBB.second.x - childPictureBB.first.x, childPictureBB.second.y - childPictureBB.first.y);
	picture->setPosition(childPictureBB.first.x, childPictureBB.first.y);
	child->add(picture);

	// ListBoxの要素が選択されたら、対応する盤面をウィンドウ右側に表示
	listBox->onItemSelect([&](const tgui::String& selectedItem) {
		std::cout << "Select board on listbox action triggered!" << std::endl;
		selectedBoardName = "";
		if (fileContents.count(selectedItem.toStdString()))
		{
			selectedBoardName = selectedItem.toStdString();
			// pictureのサイズと位置をリセット
			picture->setSize(childPictureBB.second.x - childPictureBB.first.x, childPictureBB.second.y - childPictureBB.first.y);
			picture->setPosition(childPictureBB.first.x, childPictureBB.first.y);
			// sf::Textureからtgui::textureへの変換
			sf::Image tmpImage = fileContents[selectedItem.toStdString()].mTexture.copyToImage();
			tgui::Texture tmpTGuiTex{};
			tmpTGuiTex.loadFromPixelData(tmpImage.getSize(), tmpImage.getPixelsPtr());
			picture->getRenderer()->setTexture(tmpTGuiTex);
			// 画像のスケーリングとオフセットの設定
			float scaleFactor = std::min(picture->getSize().x / static_cast<float>(tmpImage.getSize().x), picture->getSize().y / static_cast<float>(tmpImage.getSize().y));
			picture->setSize(tgui::Layout2d{ static_cast<float>(tmpImage.getSize().x) * scaleFactor, static_cast<float>(tmpImage.getSize().y) * scaleFactor });
			picture->setPosition({ childPictureBB.first.x + (child->getSize().x - childPictureBB.first.x - static_cast<float>(tmpImage.getSize().x) * scaleFactor) / 2.0f, childPictureBB.first.y + (childPictureBB.second.y - childPictureBB.first.y - static_cast<float>(tmpImage.getSize().y) * scaleFactor) / 2.0f });
		}
		});

	// 入力終了用ボタン
	auto applyButton = tgui::Button::create("Apply");
	applyButton->setRenderer(mTheme->getRenderer("Button"));
	applyButton->setSize(120, 30);
	applyButton->setPosition(static_cast<int>(child->getSize().x - applyButton->getSize().x * 2.0f) - 30, static_cast<int>(listBox->getSize().y) + 5);
	applyButton->onPress([&]() {
		std::cout << "Apply board action triggered!" << std::endl;
		if (std::find(mFilenames.begin(), mFilenames.end(), selectedBoardName) != mFilenames.end())
		{
			mCurrentKey = selectedBoardName;
			ChangeBoard();
			isChildWindowOpened = false;
		}
		});
	child->add(applyButton);

	auto exitButton = tgui::Button::create("Cancel");
	exitButton->setRenderer(mTheme->getRenderer("Button"));
	exitButton->setSize(120, 30);
	exitButton->setPosition(static_cast<int>(child->getSize().x - exitButton->getSize().x) - 10, static_cast<int>(listBox->getSize().y) + 5);
	exitButton->onPress([&]() {
		std::cout << "Cansel selection action triggered!" << std::endl;
		isChildWindowOpened = false;
		});
	child->add(exitButton);

	// ゲームループを停止
	mWindow->setActive(false);

	// デルタタイム関連
	sf::Time ticksCount = mClock.getElapsedTime();

	// 入力と更新とウィンドウの終了処理
	// ウィンドウが閉じるまでループ
	while (isChildWindowOpened)
	{
		// 60FPSに合わせて遅延をかける
		while (mClock.getElapsedTime().asMilliseconds() - ticksCount.asMilliseconds() < 16);

		float deltaTime = static_cast<float>(mClock.getElapsedTime().asMilliseconds() - ticksCount.asMilliseconds()) / 1000.0f;
		if (deltaTime > 0.05f)
		{
			deltaTime = 0.05f;
		}
		ticksCount = mClock.getElapsedTime();

		// イベントキューが存在する場合、それに応じた処理を全て行う
		sf::Event event;
		while (mWindow->pollEvent(event))
		{
			if (event.type == sf::Event::Closed ||
				event.key.code == sf::Keyboard::Escape ||
				event.key.code == sf::Keyboard::Enter)
			{
				isChildWindowOpened = false;
			}

			mGui->handleEvent(event);
		}

		mWindow->clear();

		// 背面から描画
		for (auto& sprite : mSprites)
		{
			sprite->Draw(mWindow);
		}

		// UIはゲームオブジェクトの上に描画するのでここに処理を書く
		for (const auto& ui : mUIStack)
		{
			ui->Draw(mWindow);
		}

		mGui->draw(); // TGUIの描画
		mWindow->display();
	}

	// ウィンドウのメモリ解放
	child->close();

	// ゲームループを再開
	mWindow->setActive(true);
}

void Game::ChangeBoard()
{
	// ログをファイル出力してから全て消す
	// TODO 盤面が変わった後のものになってしまっている
	OutputLogs();
	mLogs.clear();

	// 更新されたGameクラスのメンバ変数を参照して盤面を生成
	std::vector<std::string> lines{ mInitBoardData[mCurrentKey] };

	mBoardSize = sf::Vector2i{ 0, 0 };
	mBaggageNum = 0;
	mInitialPlayerPos = sf::Vector2i{ -1, -1 };
	mInitialBaggagePos.clear();
	mGoalPos.clear();
	mBoardState.clear();
	mStep = 0;
	delete  mHUDHelper;

	// 生成された盤面のキーは時刻を文字列に変換したものにする
	mBoardSize = sf::Vector2i{ static_cast<int>(lines.front().length()), static_cast<int>(lines.size()) };
	mBaggageLimit = GetBaggageNumLimit(mBoardSize);

	// 盤面の初期状態をセット
	std::vector<sf::Vector2i> mBoxesPos;
	{
		int i = 0, j = 0;
		std::string tmpstr = "";
		for (const auto& line : lines)
		{

			for (const auto& item : line)
			{
				switch (item)
				{
				case ' ':
					tmpstr += ' ';
					break;
				case '#':
					tmpstr += '#';
					break;
				case '$':
					tmpstr += ' ';
					++mBaggageNum;
					mBoxesPos.push_back(sf::Vector2i(j, i));
					break;
				case '.':
					tmpstr += '.';
					mGoalPos.push_back(sf::Vector2i(j, i));
					break;
				case '*':
					tmpstr += '.';
					++mBaggageNum;
					mBoxesPos.push_back(sf::Vector2i(j, i));
					mGoalPos.push_back(sf::Vector2i(j, i));
					break;
				case '@':
					tmpstr += ' ';
					mInitialPlayerPos = sf::Vector2i(j, i);
					break;
				case '+':
					tmpstr += '.';
					mInitialPlayerPos = sf::Vector2i(j, i);
					mGoalPos.push_back(sf::Vector2i(j, i));
					break;
				default:
					break;
				}
				j++;
			}
			mBoardState.push_back(tmpstr);
			tmpstr = "";
			i++;
			j = 0;
		}
	}

	// ボードを再構築
	mGameBoard->Reload();

	// プレイヤーを再構築
	mPlayer->Reload();

	// 荷物を再構築
	while (!mBaggages.empty())
	{
		delete mBaggages.back();
		mBaggages.pop_back();
	}
	mBaggages.clear();

	for (const auto& item : mBoxesPos)
	{
		mBaggages.emplace_back(new Baggage(this, item));
		mInitialBaggagePos.emplace(mBaggages.back(), item);
	}

	// HUDHelperを再構築
	mHUDHelper = new HUDHelper(this);

	mStart = std::chrono::system_clock::now();
}

void Game::DisplayPlayLogs(const std::string& boardKey)
{
	// 履歴データを読みこむ
	// 空でなければログデータをそのまま持ってくる
	std::map<std::string, std::pair<Board, std::vector<Log>>> loadedPlayLogs{};
	if (!boardKey.empty())
	{
		loadedPlayLogs.emplace(mCurrentKey, std::make_pair(mInitBoardData[mCurrentKey], mLogs));
	}
	else
	{
		// TODO フォルダから読みこむ
	}

	if (!loadedPlayLogs.empty())
	{
		// 表示用のウィンドウを作成
		bool isChildWindowOpened = true;
		auto child = tgui::ChildWindow::create();
		child->setRenderer(mTheme->getRenderer("ChildWindow"));
		child->setClientSize({ 960, 540 });
		child->setPosition(420, 80);
		child->setTitle("Play Log Viewer");
		child->onClose([&isChildWindowOpened]() {
			isChildWindowOpened = false;
			});
		mGui->add(child);

		// ListBoxでログのリストを表示
		auto logListBox = tgui::ListBox::create();
		logListBox->setRenderer(mTheme->getRenderer("ListBox"));
		logListBox->setSize(240, 260);
		logListBox->setItemHeight(20);
		logListBox->setPosition(0, 0);
		logListBox->setTextSize(16);
		logListBox->setAutoScroll(false);
		for (const auto& item : loadedPlayLogs)
		{
			logListBox->addItem(item.first, item.first);
		}
		child->add(logListBox);

		// ListBoxで再生中のログの情報を表示
		auto infoListBox = tgui::ListBox::create();
		infoListBox->setRenderer(mTheme->getRenderer("ListBox"));
		infoListBox->setSize(240, 240);
		infoListBox->setItemHeight(20);
		infoListBox->setPosition(0, logListBox->getSize().y);
		infoListBox->setTextSize(16);
		infoListBox->setAutoScroll(false);
		child->add(infoListBox);

		// プレイヤー終了用ボタン
		auto exitButton = tgui::Button::create("exit");
		exitButton->setRenderer(mTheme->getRenderer("Button"));
		exitButton->setSize(120, 30);
		exitButton->setPosition(static_cast<int>(child->getSize().x - exitButton->getSize().x) - 10, static_cast<int>(logListBox->getSize().y + infoListBox->getSize().y) + 5);
		exitButton->onPress([&]() {
			std::cout << "Exit selection action triggered!" << std::endl;
			isChildWindowOpened = false;
			});
		child->add(exitButton);

		// TODO その他の

		// ゲームループを停止
		mWindow->setActive(false);

		// デルタタイム関連
		sf::Time ticksCount = mClock.getElapsedTime();

		// 入力と更新とウィンドウの終了処理
		// ウィンドウが閉じるまでループ
		while (isChildWindowOpened)
		{
			// 60FPSに合わせて遅延をかける
			while (mClock.getElapsedTime().asMilliseconds() - ticksCount.asMilliseconds() < 16);

			float deltaTime = static_cast<float>(mClock.getElapsedTime().asMilliseconds() - ticksCount.asMilliseconds()) / 1000.0f;
			if (deltaTime > 0.05f)
			{
				deltaTime = 0.05f;
			}
			ticksCount = mClock.getElapsedTime();

			// イベントキューが存在する場合、それに応じた処理を全て行う
			sf::Event event;
			while (mWindow->pollEvent(event))
			{
				if (event.type == sf::Event::Closed ||
					event.key.code == sf::Keyboard::Escape ||
					event.key.code == sf::Keyboard::Enter)
				{
					isChildWindowOpened = false;
				}

				mGui->handleEvent(event);
			}

			mWindow->clear();

			// 背面から描画
			for (auto& sprite : mSprites)
			{
				sprite->Draw(mWindow);
			}

			// UIはゲームオブジェクトの上に描画するのでここに処理を書く
			for (const auto& ui : mUIStack)
			{
				ui->Draw(mWindow);
			}

			mGui->draw(); // TGUIの描画
			mWindow->display();
		}

		// ウィンドウのメモリ解放
		child->close();

		// ゲームループを再開
		mWindow->setActive(true);
	}
}

std::vector<sf::Vector2i> Game::GetBaggagesPos() const
{
	std::vector<sf::Vector2i> result{};

	for (const auto& baggage : mBaggages)
	{
		result.emplace_back(baggage->GetBoardCoordinate());
	}

	return result;
}

int Game::GetBaggageNumLimit(const sf::Vector2i& size, const double& wallRate) const
{
	int result = 1;

	result = static_cast<int>(std::floor(static_cast<double>((size.x - 1) * (size.y - 1)) * (1.0 - wallRate)));

	return result;
}

void Game::ResetParameters()
{
	// ログをクリア
	mLogs.clear();

	// 初期化処理
	mInitialPlayerPos = sf::Vector2i{ -1, -1 };
	mInitialBaggagePos.clear();
	mGoalPos.clear();
	mBoardState.clear();
	mCurrentKey.clear();
	mStep = 0;

	// HUDHelperを削除
	delete mHUDHelper;
	mHUDHelper = nullptr;
}
