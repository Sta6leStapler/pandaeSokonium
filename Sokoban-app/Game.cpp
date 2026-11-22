#include "Game.h"

#include "IActor.h"
#include "IComponent.h"
#include "SpriteComponent.h"
#include "MoveAnimationComponent.h"

#include "GameBoard.h"
#include "BackGround.h"
#include "Baggage.h"
#include "IUIScreen.h"
#include "PauseMenu.h"
#include "MySolution.h"
#include "THUD.h"
#include "HUDHelper.h"
#include "SnapshotDialog.h"
#include "SnapshotScreen.h"
#include "EditorSetupDialog.h"
#include "EditorScreen.h"
#include "BoardConfigDialog.h"
#include "LevelFormat.h"
#include "LocalizationManager.h"
#include "PlatformInput.h"
#include "PlatformTime.h"

#include "SFML/Window/Clipboard.hpp"

#include <algorithm>
#include <chrono>
#include <cmath>
#include <ctime>
#include <filesystem>
#include <fstream>
#include <iomanip>
#include <iostream>
#include <stdexcept>

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
	, mBoardViewArea(BoundingBox{ sf::Vector2f{ mWindowSize.x - mWindowSize.y + 60.0f, 0.0 }, sf::Vector2f{ mWindowSize.x, mWindowSize.y - 60.0f } })
	, mKeyHeldDuration(std::map<sf::Keyboard::Key, float>{})
	, mAutoRepeatTimer(std::map<sf::Keyboard::Key, float>{})
	, mBaggageLimit(1)
	, mStep(0)
	, mSolutionPlaybackIndex(0)
	, mIsPlayingSolution(false)
	, mSolutionPlaybackTimer(0.0f)
	, mLoc(nullptr)
	, mWasDeadlocked(false)
	, mWasComplete(false)
{
	// 盤面をロードする度に更新するのは以下のメンバ変数
	// mGenParams.boardSize
	// 現在の盤面サイズ
	// mBaggageNum
	// 現在の盤面の荷物の数
	// m	Params
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
	try
	{
		mPaths.Initialize();
		const sf::VideoMode desktop = sf::VideoMode::getDesktopMode();
		const sf::Vector2u initialSize = PlatformInput::ComputeInitialWindowSize(mWindowSize, desktop.width, desktop.height);
		mWindow = new sf::RenderWindow(sf::VideoMode(initialSize.x, initialSize.y),
			"SFML Window", sf::Style::Titlebar | sf::Style::Close);
		mWindow->setView(sf::View(sf::FloatRect(0.0f, 0.0f, mWindowSize.x, mWindowSize.y)));
		LoadData();
		mClock.restart();
		mTicksCount = mClock.getElapsedTime();
		mStart = std::chrono::system_clock::now();
		return true;
	}
	catch (const std::exception& error)
	{
		std::cerr << "Initialization failed: " << error.what() << std::endl;
		return false;
	}
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
	// フォントをロードし、初期化
	const auto fontPath = mPaths.Asset("Assets/fonts/BIZUDPGothic-Regular.ttf");
	const auto font_name = fontPath.u8string();
	mFontData = AppPaths::ReadFile(fontPath);
	if (!mFontSFML.loadFromMemory(mFontData.data(), mFontData.size()))
	{
		throw std::runtime_error("Failed to load font: " + font_name);
	}
	mInfoTxt.setFont(mFontSFML);
	mInfoTxt.setCharacterSize(static_cast<unsigned int>(mWindowSize.y));
	mInfoTxt.setScale(20.0f / mWindowSize.y, 20.0f / mWindowSize.y);

	// UIのテキストを読み込み
	mLoc = new LocalizationManager();
	if (!mLoc->LoadLanguage(mPaths.Asset("Assets/Languages/jp.txt")))
		throw std::runtime_error("Failed to load default language"); // デフォルト言語は日本語

	// 効果音を読み込み（音声デバイス非対応環境でも起動は継続）
	mSoundManager = std::make_unique<SoundManager>();
	mSoundManager->Initialize(mPaths);

	// ウィンドウのタイトルを設定
	mWindow->setTitle(mLoc->Get("GAME_TITLE"));

	// guiのテーマをロード
	mGui = std::make_unique<tgui::Gui>(*mWindow);
	mTheme = std::make_unique<tgui::Theme>(mPaths.Asset("Assets/themes/Black.txt").u8string());
	// guiのフォントもロード
	mFontTGUI = std::make_unique<tgui::Font>(mFontData.data(), mFontData.size());
	if (!*mFontTGUI)
		throw std::runtime_error("Failed to load GUI font: " + font_name);
	mGui->setFont(*mFontTGUI);

	// 盤面データを読み取る
	//*
	const auto filename = mPaths.Asset("Assets/Boards/default.txt");
	const auto defaultCollection = LevelFormat::LoadCollectionFromFile(filename);
	const auto& defaultLevel = defaultCollection.mLevels.front();
	std::string name = "bundled:default.txt";
	mCurrentKey = name;
	mFilenames.push_back(name);
	std::vector<std::string> lines = defaultLevel.mBoard;

	mBoardData.emplace(name, lines);
	mInitBoardData.emplace(name, lines);
	mBoardSolutions[name] = defaultLevel.mSolutions;
	mCurrentLevelSolutions = defaultLevel.mSolutions;
	//*/

	// 自動生成の場合
	// MySolution
	/*
	InputBoardData();
	MySolution* gen = new MySolution(mGenParams.boardSize, mBaggageNum, mRepetition01, mRepetition02, mRepetition03, mRepetition04, mRepetition05);
	std::vector<std::string> lines = gen->GetBoard();
	delete(gen);
	mCurrentKey = GetDateTime();
	mFilenames.emplace_back(mCurrentKey);
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

	// Validate file-backed textures before actors register their components.
	for (const auto* name : {
		"BackGround.png", "Floor.png", "Wall.png", "Goal.png",
		"Box.png", "ShinyBox.png", "DeadlockedBox.png",
		"playerE.png", "playerN.png", "playerW.png", "playerS.png",
		"MoveGuideGrid.png", "PushGuideGrid.png",
		"PushGuideArrowIdleFromE.png", "PushGuideArrowIdleFromS.png",
		"PushGuideArrowIdleFromW.png", "PushGuideArrowIdleFromN.png",
		"PushGuideArrowOnCursorFromE.png", "PushGuideArrowOnCursorFromS.png",
		"PushGuideArrowOnCursorFromW.png", "PushGuideArrowOnCursorFromN.png",
		"Undo.png", "Redo.png", "Reset.png", "RedoAll.png", "SaveBoard.png", "SaveLog.png" })
	{
		LoadTexture(std::string("Assets/") + name);
	}

	// 背景の作成
	// 背景は画面の中央に配置
	new BackGround(this);

	// ボードを作成
	mGameBoard = new GameBoard(this);

	// 自動生成用のパラメータを初期化
	// デフォルトの盤面は自動生成されないので、適当な値を入れておく
	mGenParams.boardSize = sf::Vector2i{ static_cast<int>(lines.front().size()), static_cast<int>(lines.size()) };
	mGenParams.baggageNum = static_cast<int>(mBoxesPos.size());
	mGenParams.resetCount = 1;
	mGenParams.transportCount = mGenParams.baggageNum * 2;
	mGenParams.wallRate = 0.0f;
	mGenParams.visitedRate = 0.0f;
	mGenParams.evalFuncIndex = 0;

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
	delete mLoc;
	mLoc = nullptr;
}

void Game::ProcessInput()
{
	sf::Event event;
	while (mWindow->pollEvent(event))
	{
		// ウィンドウを終了するイベントを検知したら、ゲーム状態を終了にしてイベント処理を即座に離脱
		switch (event.type)
		{
		case sf::Event::Closed:
			mGameState = GameState::EQuit;
			break;
		}

		// ゲームプレイ状態ならば、キーボードやボタンの入力処理を受け付ける
		if (mGameState == GameState::EGamePlay)
		{
			// 解手順アニメーション再生中はEscキーで停止可能
			if (mIsPlayingSolution)
			{
				if (event.type == sf::Event::KeyPressed && event.key.code == sf::Keyboard::Escape)
				{
					StopSolutionPlayback();
				}
			}
			// アニメーション中でなければ入力を受け付ける
			else if (!IsAnyMovementAnimating())
			{
				// ゲーム全体に関する各種入力処理
				// 単押し処理
				if (event.type == sf::Event::KeyPressed)
				{
					// このキーが押されてから経過した時間を確認
					float duration = mKeyHeldDuration.count(event.key.code) ? mKeyHeldDuration.at(event.key.code) : 0.0f;

					// 押された最初のフレーム（押下時間が非常に短い）かを判定
					if (duration < GetTapThresHold())
					{
						// Escキーでポーズメニューを開く
						if (event.key.code == sf::Keyboard::Escape && mGameState == GameState::EGamePlay)
						{
							// ボタンの長押しを解除
							CancelAllUIHolds();
							new PauseMenu(this, mWindow);
							mGameState = GameState::EPaused;
						}

						// z でundo処理
						if (event.key.code == sf::Keyboard::Z)
						{
							// ボタンの長押しを解除
							CancelAllUIHolds();
							CallUndo();
						}

						// y でredo処理
						if (event.key.code == sf::Keyboard::Y)
						{
							// ボタンの長押しを解除
							CancelAllUIHolds();
							CallRedo();
						}

						// PGUPで最新の状態にする
						if (event.key.code == sf::Keyboard::PageUp)
						{
							// ボタンの長押しを解除
							CancelAllUIHolds();
							CallRedoAll();
						}

						// PGDNで初期状態にする
						if (event.key.code == sf::Keyboard::PageDown)
						{
							// ボタンの長押しを解除
							CancelAllUIHolds();
							CallReset();
						}

						// Ctrl + r (macOSではCmd + rも可) で全てリセット
						if (PlatformInput::IsCommandModifierPressed(event.key) && event.key.code == sf::Keyboard::R)
						{
							// ボタンの長押しを解除
							CancelAllUIHolds();
							CallRestart();
						}

						// Ctrl + s (macOSではCmd + sも可) で現在の盤面をセーブ
						if (PlatformInput::IsCommandModifierPressed(event.key) && event.key.code == sf::Keyboard::S)
						{
							// ボタンの長押しを解除
							CancelAllUIHolds();
							CallSave();
						}

						// H でヘルプ画面の表示
						if (event.key.code == sf::Keyboard::H)
						{
							// ボタンの長押しを解除
							CancelAllUIHolds();
							DisplayHelpWindow();
						}

						// F5 で盤面のリロード（自動生成の盤面なら新たな盤面の生成）
						if (event.key.code == sf::Keyboard::F5)
						{
							// ボタンの長押しを解除
							CancelAllUIHolds();
							CallReload();
						}
					}
				}

				// 長押しの処理
				// イベントとは関係なく、現在のキー押下状態とタイマーから判断
				switch (event.key.code)
				{
				case sf::Keyboard::Z:
					// Undo処理
					// 長押し状態にあり、かつリピートタイマーが0以下か？
					if (mKeyHeldDuration.at(sf::Keyboard::Z) > GetHoldThresHold() && mAutoRepeatTimer.at(sf::Keyboard::Z) <= 0.0f)
					{
						CallUndo();
					}
					break;
				case sf::Keyboard::Y:
					// Redo処理
					// 長押し状態にあり、かつリピートタイマーが0以下か？
					if (mKeyHeldDuration.at(sf::Keyboard::Y) > GetHoldThresHold() && mAutoRepeatTimer.at(sf::Keyboard::Y) <= 0.0f)
					{
						CallRedo();
					}
					break;
				default:
					break;
				}
			}

			// 解手順再生中はアクターへの手動入力を受け付けない
			if (!mIsPlayingSolution)
			{
				// 全てのActorの入力処理を行う
				mUpdatingActors = true;

				for (auto& actor : mActiveActors)
				{
					actor->ProcessInput(&event, mKeyHeldDuration, mAutoRepeatTimer);
				}

				mUpdatingActors = false;
				// アクターの入力処理はここまで
			}
		}

		// その他の状態ではUIの入力処理を行う
		// ゲームプレイ中に使えるUIは未実装
		if (!mUIStack.empty())// && mGameState != GameState::EUIPaused)
		{
			// 一番手前のレイヤのUIの入力処理のみ行う（縮小表示時は論理座標へ変換）
			const sf::Vector2f logicalMouse = MapPixelToLogicalCoords(sf::Mouse::getPosition(*mWindow));
			mUIStack.back()->ProcessInput(&event, sf::Vector2i{ static_cast<int>(logicalMouse.x), static_cast<int>(logicalMouse.y) });
		}
	}
}

void Game::UpdateGame()
{
	// デルタタイムを計算する
	// 最後のフレームから16msだけ待つ
	WaitForNextFrame(mTicksCount);

	float deltaTime = static_cast<float>(mClock.getElapsedTime().asMilliseconds() - mTicksCount.asMilliseconds()) / 1000.0f;
	if (deltaTime > 0.05f)
	{
		deltaTime = 0.05f;
	}
	mTicksCount = mClock.getElapsedTime();

	// 全てのキーをスキャンしてタイマーを更新
	// sf::Keyboard::KeyCount はキーの総数を示す
	for (int k = 0; k < sf::Keyboard::KeyCount; ++k)
	{
		sf::Keyboard::Key currentKey = static_cast<sf::Keyboard::Key>(k);

		if (sf::Keyboard::isKeyPressed(currentKey))
		{
			// ---- キーが押されている場合の処理 ----
			// 押下時間を加算
			mKeyHeldDuration[currentKey] += deltaTime;

			// 長押し状態か？
			if (mKeyHeldDuration[currentKey] > HOLD_THRESHOLD)
			{
				// リピートタイマーを減算
				mAutoRepeatTimer[currentKey] -= deltaTime;
			}
		}
		else
		{
			// ---- キーが離されている場合の処理 ----
			if (mKeyHeldDuration[currentKey] > 0.0f)
			{
				// 押下時間とタイマーをリセット
				mKeyHeldDuration[currentKey] = 0.0f;
				mAutoRepeatTimer[currentKey] = 0.0f;
			}
		}
	}

	// 解手順アニメーション再生中なら1手ずつ進める
	UpdateSolutionPlayback(deltaTime);

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
	const bool isDeadlockedNow = !mHUDHelper->GetDeadlockedBaggages().empty();
	if (isDeadlockedNow && !mWasDeadlocked)
	{
		PlaySound(SoundManager::SoundEffect::Deadlock);
	}
	mWasDeadlocked = isDeadlockedNow;

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
	if (mIsComplete && !IsAnyMovementAnimating())
	{
		DisplayResult();
	}
}


sf::Texture* Game::LoadTexture(const std::string& fileName)
{
	auto iter = mTextures.find(fileName);
	if (iter != mTextures.end())
		return iter->second;

	const auto path = mPaths.Asset(fileName);
	const auto data = AppPaths::ReadFile(path);
	auto texture = std::make_unique<sf::Texture>();
	if (!texture->loadFromMemory(data.data(), data.size()))
		throw std::runtime_error("Failed to load texture: " + path.u8string());
	auto result = texture.get();
	mTextures.emplace(fileName, result);
	texture.release();
	return result;
}


void Game::Shutdown()
{
	UnloadData();
	mSoundManager.reset();
	mFontTGUI.reset();
	mTheme.reset();
	mGui.reset();
	if (mWindow) mWindow->close();
	delete mWindow;
	mWindow = nullptr;
}

void Game::WaitForNextFrame(const sf::Time& previousTicks) const
{
	const sf::Int32 elapsedMs = mClock.getElapsedTime().asMilliseconds() - previousTicks.asMilliseconds();
	if (elapsedMs < 16)
	{
		sf::sleep(sf::milliseconds(16 - elapsedMs));
	}
}

void Game::ShowFileError(const std::string& message)
{
	std::cerr << message << std::endl;
	if (mGameState == GameState::EQuit) return;

	bool open = true;
	auto dialog = tgui::MessageBox::create(mLoc->Get("FILE_ERROR_TITLE"),
		mLoc->Get("FILE_ERROR_MESSAGE") + "\n" + message, { mLoc->Get("BTN_OK") });
	dialog->setRenderer(mTheme->getRenderer("MessageBox"));
	dialog->setPosition("(&.width - width) / 2", "(&.height - height) / 2");
	dialog->onButtonPress([&]() { open = false; });
	dialog->onClose([&]() { open = false; });
	mGui->add(dialog);

	while (open)
	{
		sf::Event event;
		while (mWindow->pollEvent(event))
		{
			if (event.type == sf::Event::Closed)
			{
				mGameState = GameState::EQuit;
				open = false;
				break;
			}
			mGui->handleEvent(event);
		}
		mWindow->clear();
		DrawSprites();
		DrawUI();
		mGui->draw();
		mWindow->display();
		sf::sleep(sf::milliseconds(16));
	}
	mGui->remove(dialog);
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

bool Game::IsAnyMovementAnimating() const
{
	if (mIsPlayingSolution)
	{
		return true;
	}

	// プレイヤーのアニメーション確認
	if (mPlayer && mPlayer->GetMoveAnimation()->IsAnimating())
	{
		return true;
	}

	// 全ての荷物のアニメーション確認
	for (auto baggage : mBaggages)
	{
		if (baggage->GetMoveAnimation()->IsAnimating())
		{
			return true;
		}
	}

	return false;
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

sf::Vector2f Game::MapPixelToLogicalCoords(const sf::Vector2i& pixelPos) const
{
	if (!mWindow)
	{
		return sf::Vector2f{ static_cast<float>(pixelPos.x), static_cast<float>(pixelPos.y) };
	}
	return mWindow->mapPixelToCoords(pixelPos);
}

sf::Vector2i Game::ScreenToTileCoords(const sf::Vector2f& screenPos) const
{
	sf::Vector2f bounding_area_pos(screenPos);
	bounding_area_pos -= mBoardViewArea.first;
	sf::Vector2i result{ -1, -1 };
	// スケーリングを求める
	// 現状描画範囲は正方形なので、盤面が横長か縦長かでスケーリングを変える
	// スケーリングは大きい方に合わせる
	float scaling = (mGenParams.boardSize.x > mGenParams.boardSize.y) ? ((mBoardViewArea.second.x - mBoardViewArea.first.x) / static_cast<float>(mGenParams.boardSize.x)) : ((mBoardViewArea.second.y - mBoardViewArea.first.y) / static_cast<float>(mGenParams.boardSize.y));

	if (bounding_area_pos.x >= 0.0f && bounding_area_pos.y >= 0.0f)
	{
		// 横長なら縦方向の平行移動、縦長なら横方向の平行移動
		if (mGenParams.boardSize.x > mGenParams.boardSize.y)
		{
			bounding_area_pos.y -= ((mBoardViewArea.second.y - mBoardViewArea.first.y) - ((mBoardViewArea.second.x - mBoardViewArea.first.x) / static_cast<float>(mGenParams.boardSize.x)) * static_cast<float>(mGenParams.boardSize.y)) / 2.0f;
		}
		else if (mGenParams.boardSize.x < mGenParams.boardSize.y)
		{
			bounding_area_pos.x -= ((mBoardViewArea.second.x - mBoardViewArea.first.x) - ((mBoardViewArea.second.y - mBoardViewArea.first.y) / static_cast<float>(mGenParams.boardSize.y)) * static_cast<float>(mGenParams.boardSize.x)) / 2.0f;
		}

		result.x = static_cast<int>(bounding_area_pos.x / scaling);
		result.y = static_cast<int>(bounding_area_pos.y / scaling);
	}

	//std::cout << "ScreenToTileCoords : (" << screenPos.x << ", " << screenPos.y << ") -> (" << bounding_area_pos.x << ", " << bounding_area_pos.y << ") -> (" << result.x << ", " << result.y << ")" << std::endl;

	return result;
}

sf::Vector2f Game::TileToScreenCoords(const sf::Vector2i& tileCorrdsPos) const
{
	sf::Vector2i bounding_area_pos(tileCorrdsPos);
	sf::Vector2f result{ -1.0f, -1.0f };
	float scaling = (mGenParams.boardSize.x > mGenParams.boardSize.y) ? (mBoardViewArea.second.x - mBoardViewArea.first.x) / mGenParams.boardSize.x : (mBoardViewArea.second.y - mBoardViewArea.first.y) / mGenParams.boardSize.y;

	if (bounding_area_pos.x >= 0 && bounding_area_pos.x < mGenParams.boardSize.x &&
		bounding_area_pos.y >= 0 && bounding_area_pos.y < mGenParams.boardSize.y)
	{
		result.x = static_cast<float>(static_cast<float>(bounding_area_pos.x) * scaling);
		result.y = static_cast<float>(static_cast<float>(bounding_area_pos.y) * scaling);

		// 横長なら縦方向の平行移動、縦長なら横方向の平行移動
		if (mGenParams.boardSize.x > mGenParams.boardSize.y)
		{
			result.y += ((mBoardViewArea.second.y - mBoardViewArea.first.y) - ((mBoardViewArea.second.x - mBoardViewArea.first.x) / static_cast<float>(mGenParams.boardSize.x)) * static_cast<float>(mGenParams.boardSize.y)) / 2.0f;
		}
		else if (mGenParams.boardSize.x < mGenParams.boardSize.y)
		{
			result.x += ((mBoardViewArea.second.x - mBoardViewArea.first.x) - ((mBoardViewArea.second.y - mBoardViewArea.first.y) / static_cast<float>(mGenParams.boardSize.y)) * static_cast<float>(mGenParams.boardSize.x)) / 2.0f;
		}
	}

	result += mBoardViewArea.first;

	std::cout << "TileToScreenCoords : (" << tileCorrdsPos.x << ", " << tileCorrdsPos.y << ") -> (" << bounding_area_pos.x << ", " << bounding_area_pos.y << ") -> (" << result.x << ", " << result.y << ")" << std::endl;

	return result;
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
		ClearMoveHighlights();
		ClearPushHighlights();
		ClearPushDirections();
		mStep--;
		PlaySound(SoundManager::SoundEffect::Undo);
		
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
		ClearMoveHighlights();
		ClearPushHighlights();
		ClearPushDirections();
		mStep++;
		PlaySound(SoundManager::SoundEffect::Redo);
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

bool Game::CallSave(const std::vector<std::string>& boardData)
{
	// 保存対象のデータを決定
	std::vector<std::string> dataToSave = boardData;
	if (dataToSave.empty())
	{
		// 引数が空なら、メインゲームの現在の盤面（初期状態）を使用
		dataToSave = mInitBoardData[mCurrentKey];
	}

	bool isDialogActive = true;
	bool saved = false;
	std::string saveError;

	// ファイルダイアログの作成
	auto fileDialog = tgui::FileDialog::create();
	fileDialog->setRenderer(mTheme->getRenderer("FileDialog"));
	fileDialog->setTitle(mLoc->Get("BOARD_SAVE_DIALOG_TITLE"));
	fileDialog->setFileMustExist(false); // 保存用なので存在しないファイル名も許可
	fileDialog->setConfirmButtonText(mLoc->Get("BTN_SAVE"));
	fileDialog->setCancelButtonText(mLoc->Get("BTN_CANCEL"));
	fileDialog->setListViewColumnCaptions(mLoc->Get("BOARD_SAVE_DIALOG_COL_NAME"), mLoc->Get("BOARD_SAVE_DIALOG_COL_SIZE"), mLoc->Get("BOARD_SAVE_DIALOG_COL_MODIFIED"));
	fileDialog->setFilenameLabelText(mLoc->Get("BOARD_SAVE_DIALOG_LBL_FILENAME"));
	fileDialog->setAllowCreateFolder(true);
	fileDialog->setCreateFolderButtonText(mLoc->Get("BOARD_SAVE_DIALOG_CREATE_FOLDER"));
	fileDialog->setPath(mPaths.GetBoardsDirectory().u8string()); // デフォルトパス
	fileDialog->setFilename("Level_" + GetDateTime() + ".sok"); // デフォルトファイル名
	fileDialog->setFileTypeFilters({
		{ "Sokoban Levels (*.sok, *.xsb, *.txt)", { "*.sok", "*.xsb", "*.txt" } },
		{ "SOK (*.sok)", { "*.sok" } },
		{ "XSB (*.xsb)", { "*.xsb" } },
		{ "Text (*.txt)", { "*.txt" } }
	});
	// 既存ファイルを選択した場合の上書き確認
	// (TGUIのバージョンによっては setConfirmOverwrite(true) がデフォルトまたは存在しない場合がありますが、
	//  多くのバージョンでサポートされています)
	// fileDialog->setConfirmOverwrite(true); 

	// シグナル設定

	fileDialog->onFileSelect([&](const tgui::String& selectedPath) {
		try
		{
			const auto path = std::filesystem::u8path(selectedPath.toStdString());
			LevelFormat::LevelData levelData;
			levelData.mTitle = path.stem().u8string();
			levelData.mBoard = dataToSave;
			if (boardData.empty())
			{
				const std::string currentLurd = GetCurrentLurdSolution(true);
				if (!currentLurd.empty())
				{
					levelData.mSolutions.push_back(currentLurd);
				}
			}
			LevelFormat::SaveLevelToFile(path, levelData);
			saved = true;
			std::cout << "Saved to " << path << std::endl;
		}
		catch (const std::exception& error)
		{
			saveError = error.what();
		}
		isDialogActive = false;
		});

	fileDialog->onCancel([&]() {
		std::cout << "Save cancelled." << std::endl;
		isDialogActive = false;
		});

	fileDialog->onClose([&]() {
		isDialogActive = false;
		});

	// 画面中央に配置（サイズ決定後に計算したほうが正確ですが、簡易的に）
	fileDialog->setSize(600, 400);
	fileDialog->setPosition((mWindowSize.x - 600) / 2, (mWindowSize.y - 400) / 2);

	mGui->add(fileDialog);

	// ブロッキングループ（ダイアログが閉じるまで待機）
	// ゲームループを一時停止
	mWindow->setActive(false);
	sf::Time ticksCount = mClock.getElapsedTime();

	while (isDialogActive)
	{
		WaitForNextFrame(ticksCount);
		float deltaTime = static_cast<float>(mClock.getElapsedTime().asMilliseconds() - ticksCount.asMilliseconds()) / 1000.0f;
		if (deltaTime > 0.05f) deltaTime = 0.05f;
		ticksCount = mClock.getElapsedTime();

		sf::Event event;
		while (mWindow->pollEvent(event))
		{
			if (event.type == sf::Event::Closed)
			{
				isDialogActive = false;
				mGameState = GameState::EQuit; // アプリ終了
			}
			mGui->handleEvent(event);
		}

		mWindow->clear();

		// 背面の描画（ゲーム画面と他のUI）
		for (auto& sprite : mSprites) sprite->Draw(mWindow);
		for (const auto& ui : mUIStack) ui->Draw(mWindow);

		// TGUI（ダイアログ）の描画
		mGui->draw();
		mWindow->display();
	}

	mGui->remove(fileDialog);
	mWindow->setActive(true);
	if (!saveError.empty()) ShowFileError(saveError);
	return saved;
}

void Game::CallReload()
{
	StopSolutionPlayback();
	// プレイヤーと荷物と盤面を更新
	// パラメータ設定際に、closeが押されなければ、続きの更新処理を行う
	if (InputBoardData())
	{
		// ログをファイル出力してから全て消す
		OutputLogs();
		ClearParameters();
		mCurrentLevelSolutions.clear();

		// スナップショットもリセット
		ClearSnapshots();

		// 更新されたGameクラスのメンバ変数を参照して盤面を生成
		MySolution* gen = new MySolution(
			mGenParams.boardSize,
			mGenParams.baggageNum,
			mGenParams.resetCount,
			mGenParams.transportCount,
			mGenParams.wallRate,
			mGenParams.visitedRate,
			mGenParams.evalFuncIndex
		);
		std::vector<std::string> lines = gen->GetBoard();
		delete(gen);

		// 生成された盤面のキーは時刻を文字列に変換したものにする
		mCurrentKey = GetDateTime();
		mFilenames.emplace_back(mCurrentKey);
		mBoardData.emplace(mCurrentKey, lines);
		mInitBoardData.emplace(mCurrentKey, lines);
		mBaggageLimit = GetBaggageNumLimit(mGenParams.boardSize, mGenParams.wallRate);

		// 盤面の初期状態をセット（生成結果に合わせてサイズを再確定）
		mGenParams.boardSize = sf::Vector2i{ static_cast<int>(lines.front().length()), static_cast<int>(lines.size()) };
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
			mBaggages.pop_back();
		}

		for (const auto& item : mBoxesPos)
		{
			mBaggages.emplace_back(new Baggage(this, item));
			mInitialBaggagePos.emplace(mBaggages.back(), item);
		}

		// HUDHelperを再構築
		mHUDHelper = new HUDHelper(this);

		mStart = std::chrono::system_clock::now();
	}
}

void Game::CallRestart()
{
	StopSolutionPlayback();
	// ログをファイル出力してからパラメータをリセット
	OutputLogs();
	ResetParameters();

	// スナップショットもリセット
	ClearSnapshots();

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

	// HUDHelperを再構築
	mHUDHelper = new HUDHelper(this);

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
	std::tm timeinfo = PlatformTime::GetLocalTime(now_time);
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
	std::tm timeinfo = PlatformTime::GetLocalTime(now_time);
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
	try
	{
		std::string contents;
		for (const auto& row : mInitBoardData.at(mGameBoard->GetBoardName()))
			contents += row + "\n";
		contents += ConvertLogToStr(mLogs, 0);
		AppPaths::WriteFile(mPaths.NewLogPath(GetDateTime()), contents);
	}
	catch (const std::exception& error)
	{
		ShowFileError(error.what());
	}
}

void Game::CallSaveLog()
{
	// 保存するログデータを取得
	std::string logData = ConvertLogToStr(mLogs);

	bool isDialogActive = true;
	std::string saveError;

	// ファイルダイアログの作成（CallSaveと同様の設定）
	auto fileDialog = tgui::FileDialog::create();
	fileDialog->setRenderer(mTheme->getRenderer("FileDialog"));
	fileDialog->setTitle(mLoc->Get("LOG_SAVE_DIALOG_TITLE")); // 辞書にキーを追加してください
	fileDialog->setFileMustExist(false);
	fileDialog->setConfirmButtonText(mLoc->Get("BTN_SAVE"));
	fileDialog->setCancelButtonText(mLoc->Get("BTN_CANCEL"));
	fileDialog->setListViewColumnCaptions(mLoc->Get("BOARD_SAVE_DIALOG_COL_NAME"), mLoc->Get("BOARD_SAVE_DIALOG_COL_SIZE"), mLoc->Get("BOARD_SAVE_DIALOG_COL_MODIFIED"));
	fileDialog->setFilenameLabelText(mLoc->Get("BOARD_SAVE_DIALOG_LBL_FILENAME"));
	fileDialog->setAllowCreateFolder(true);
	fileDialog->setCreateFolderButtonText(mLoc->Get("BOARD_SAVE_DIALOG_CREATE_FOLDER"));
	fileDialog->setPath(mPaths.GetLogsDirectory().u8string()); // ログ用のデフォルトパス
	fileDialog->setFilename("Log_" + GetDateTime() + ".txt");

	// シグナル設定

	fileDialog->onFileSelect([&](const tgui::String& selectedPath) {
		try
		{
			const auto path = std::filesystem::u8path(selectedPath.toStdString());
			AppPaths::WriteFile(path, logData);
			std::cout << "Log saved to " << path << std::endl;
		}
		catch (const std::exception& error)
		{
			saveError = error.what();
		}
		isDialogActive = false;
		});

	fileDialog->onCancel([&]() { isDialogActive = false; });
	fileDialog->onClose([&]() { isDialogActive = false; });

	fileDialog->setSize(600, 400);
	fileDialog->setPosition((mWindowSize.x - 600) / 2, (mWindowSize.y - 400) / 2);

	mGui->add(fileDialog);

	// ブロッキングループ
	mWindow->setActive(false);
	sf::Time ticksCount = mClock.getElapsedTime();

	while (isDialogActive)
	{
		WaitForNextFrame(ticksCount);
		ticksCount = mClock.getElapsedTime();

		sf::Event event;
		while (mWindow->pollEvent(event))
		{
			if (event.type == sf::Event::Closed)
			{
				isDialogActive = false;
				mGameState = GameState::EQuit;
			}
			mGui->handleEvent(event);
		}

		mWindow->clear();
		for (auto& sprite : mSprites) sprite->Draw(mWindow);
		for (const auto& ui : mUIStack) ui->Draw(mWindow);
		mGui->draw();
		mWindow->display();
	}

	mGui->remove(fileDialog);
	mWindow->setActive(true);
	if (!saveError.empty()) ShowFileError(saveError);
}

// =========================================================================
// 解手順 (rludRLUD) 関連の実装
// =========================================================================

std::string Game::GetCurrentLurdSolution(bool upToCurrentStep) const
{
	const std::size_t limit = upToCurrentStep
		? std::min(static_cast<std::size_t>(mStep), mLogs.size())
		: mLogs.size();

	std::string lurd;
	lurd.reserve(limit);
	for (std::size_t i = 0; i < limit; ++i)
	{
		const auto& log = mLogs[i];
		const int dx = log.pCoordinate.second.x - log.pCoordinate.first.x;
		const int dy = log.pCoordinate.second.y - log.pCoordinate.first.y;
		const char ch = SolutionCodec::EncodeMove(dx, dy, log.isBMoved);
		if (ch != '\0')
		{
			lurd.push_back(ch);
		}
	}
	return lurd;
}

bool Game::SaveLurdSolutionDialog(const std::string& lurd)
{
	const std::string textToSave = lurd.empty() ? GetCurrentLurdSolution(true) : lurd;
	if (textToSave.empty())
	{
		return false;
	}

	bool isDialogActive = true;
	bool saved = false;
	std::string saveError;

	auto fileDialog = tgui::FileDialog::create();
	fileDialog->setRenderer(mTheme->getRenderer("FileDialog"));
	fileDialog->setTitle(mLoc->Get("SOLUTION_SAVE_DIALOG_TITLE"));
	fileDialog->setFileMustExist(false);
	fileDialog->setConfirmButtonText(mLoc->Get("BTN_SAVE"));
	fileDialog->setCancelButtonText(mLoc->Get("BTN_CANCEL"));
	fileDialog->setListViewColumnCaptions(mLoc->Get("BOARD_SAVE_DIALOG_COL_NAME"), mLoc->Get("BOARD_SAVE_DIALOG_COL_SIZE"), mLoc->Get("BOARD_SAVE_DIALOG_COL_MODIFIED"));
	fileDialog->setFilenameLabelText(mLoc->Get("BOARD_SAVE_DIALOG_LBL_FILENAME"));
	fileDialog->setAllowCreateFolder(true);
	fileDialog->setCreateFolderButtonText(mLoc->Get("BOARD_SAVE_DIALOG_CREATE_FOLDER"));
	fileDialog->setPath(mPaths.GetLogsDirectory().u8string());
	fileDialog->setFilename("Solution_" + GetDateTime() + ".txt");
	fileDialog->setFileTypeFilters({
		{ "Solution Text (*.txt, *.lurd)", { "*.txt", "*.lurd" } },
		{ "All Files (*.*)", { "*" } }
	});

	fileDialog->onFileSelect([&](const tgui::String& selectedPath) {
		try
		{
			const auto path = std::filesystem::u8path(selectedPath.toStdString());
			AppPaths::WriteFile(path, textToSave + "\n");
			saved = true;
			std::cout << "LURD solution saved to " << path << std::endl;
		}
		catch (const std::exception& error)
		{
			saveError = error.what();
		}
		isDialogActive = false;
		});

	fileDialog->onCancel([&]() { isDialogActive = false; });
	fileDialog->onClose([&]() { isDialogActive = false; });

	fileDialog->setSize(600, 400);
	fileDialog->setPosition((mWindowSize.x - 600) / 2, (mWindowSize.y - 400) / 2);
	mGui->add(fileDialog);

	mWindow->setActive(false);
	sf::Time ticksCount = mClock.getElapsedTime();
	while (isDialogActive)
	{
		WaitForNextFrame(ticksCount);
		ticksCount = mClock.getElapsedTime();

		sf::Event event;
		while (mWindow->pollEvent(event))
		{
			if (event.type == sf::Event::Closed)
			{
				isDialogActive = false;
				mGameState = GameState::EQuit;
			}
			mGui->handleEvent(event);
		}

		mWindow->clear();
		DrawSprites();
		DrawUI();
		mGui->draw();
		mWindow->display();
	}

	mGui->remove(fileDialog);
	mWindow->setActive(true);
	if (!saveError.empty()) ShowFileError(saveError);
	return saved;
}

void Game::StopSolutionPlayback()
{
	mIsPlayingSolution = false;
	mSolutionPlaybackSteps.clear();
	mSolutionPlaybackIndex = 0;
	mSolutionPlaybackTimer = 0.0f;
}

void Game::ExecuteSolutionStep(const SolutionCodec::SolutionStep& step, bool animate)
{
	if (!mPlayer)
	{
		return;
	}

	const Player::Direction prevDir = mPlayer->GetDirection();
	Player::Direction nextDir = prevDir;
	const int dx = step.mToX - step.mFromX;
	const int dy = step.mToY - step.mFromY;
	if (dx > 0) nextDir = Player::EEast;
	else if (dx < 0) nextDir = Player::EWest;
	else if (dy > 0) nextDir = Player::ESouth;
	else if (dy < 0) nextDir = Player::ENorth;

	mPlayer->SetDirection(nextDir);

	const sf::Vector2i pFrom{ step.mFromX, step.mFromY };
	const sf::Vector2i pTo{ step.mToX, step.mToY };

	if (step.mIsPush)
	{
		const sf::Vector2i bFrom{ step.mBaggageFromX, step.mBaggageFromY };
		const sf::Vector2i bTo{ step.mBaggageToX, step.mBaggageToY };
		Baggage* baggage = GetBaggageFromPos(bFrom);
		if (baggage)
		{
			if (animate)
			{
				baggage->GetMoveAnimation()->Start(TileToScreenCoords(bFrom), TileToScreenCoords(bTo), SOLUTION_PLAYBACK_INTERVAL);
				baggage->SetLogicalCoordinate(bTo);
			}
			else
			{
				baggage->SetBoardCoordinate(bTo);
			}
		}

		if (animate)
		{
			mPlayer->GetMoveAnimation()->Start(mPlayer->GetPosition(), TileToScreenCoords(pTo), SOLUTION_PLAYBACK_INTERVAL);
			mPlayer->SetLogicalCoordinate(pTo);
		}
		else
		{
			mPlayer->SetBoardCoordinate(pTo);
		}

		RemoveRedo();
		AddStep();
		AddLog(pFrom, pTo, bFrom, bTo, prevDir, nextDir);
		if (animate)
		{
			PlaySound(mBoardState[bTo.y][bTo.x] == '.'
				? SoundManager::SoundEffect::GoalIn
				: SoundManager::SoundEffect::Push);
		}
	}
	else
	{
		if (animate)
		{
			mPlayer->GetMoveAnimation()->Start(mPlayer->GetPosition(), TileToScreenCoords(pTo), SOLUTION_PLAYBACK_INTERVAL);
			mPlayer->SetLogicalCoordinate(pTo);
		}
		else
		{
			mPlayer->SetBoardCoordinate(pTo);
		}

		RemoveRedo();
		AddStep();
		AddLog(pFrom, pTo, prevDir, nextDir);
		if (animate)
		{
			PlaySound(SoundManager::SoundEffect::Move);
		}
	}

	ClearMoveHighlights();
	ClearPushHighlights();
	ClearPushDirections();
}

void Game::UpdateSolutionPlayback(float deltaTime)
{
	if (!mIsPlayingSolution)
	{
		return;
	}

	// 直前の1手のアニメーション中は完了を待つ
	if ((mPlayer && mPlayer->GetMoveAnimation()->IsAnimating()))
	{
		return;
	}
	for (auto* baggage : mBaggages)
	{
		if (baggage->GetMoveAnimation()->IsAnimating())
		{
			return;
		}
	}

	mSolutionPlaybackTimer -= deltaTime;
	if (mSolutionPlaybackTimer > 0.0f)
	{
		return;
	}

	if (mSolutionPlaybackIndex >= mSolutionPlaybackSteps.size())
	{
		StopSolutionPlayback();
		return;
	}

	ExecuteSolutionStep(mSolutionPlaybackSteps[mSolutionPlaybackIndex], true);
	++mSolutionPlaybackIndex;

	if (mSolutionPlaybackIndex >= mSolutionPlaybackSteps.size())
	{
		StopSolutionPlayback();
	}
	else
	{
		mSolutionPlaybackTimer = SOLUTION_PLAYBACK_INTERVAL;
	}
}

bool Game::ApplyLurdSolution(const std::string& rawLurd, bool fromInitialState, bool animate, std::string& outError)
{
	outError.clear();
	StopSolutionPlayback();

	const std::vector<std::string> baseBoard = fromInitialState
		? mInitBoardData.at(mCurrentKey)
		: GetBoardStateWithObjects();

	// 標準の寛容モードで事前検証（違法手のみエラー）
	const auto sim = SolutionCodec::SimulateLurd(baseBoard, rawLurd, false);
	if (!sim.mIsValid)
	{
		outError = sim.mErrorMessage;
		return false;
	}

	if (sim.mSteps.empty())
	{
		outError = mLoc->Get("SOLUTION_MSG_EMPTY");
		return false;
	}

	// 初期状態から適用する場合はUndo経由で初期状態へ戻す
	if (fromInitialState)
	{
		CallReset();
	}

	if (animate)
	{
		mSolutionPlaybackSteps = sim.mSteps;
		mSolutionPlaybackIndex = 0;
		mSolutionPlaybackTimer = 0.0f;
		mIsPlayingSolution = true;
	}
	else
	{
		for (const auto& step : sim.mSteps)
		{
			ExecuteSolutionStep(step, false);
		}
	}

	return true;
}

void Game::OpenSolutionDialog(const std::string& initialLurd)
{
	CancelAllUIHolds();
	StopSolutionPlayback();

	bool isDialogOpen = true;
	bool shouldStartPlayback = false;
	bool playbackFromInitial = true;
	bool playbackAnimate = true;
	std::string pendingLurd;

	const std::string currentLurd = GetCurrentLurdSolution(true);

	auto child = tgui::ChildWindow::create();
	child->setRenderer(mTheme->getRenderer("ChildWindow"));
	child->setClientSize({ 760, 520 });
	child->setPosition((mWindowSize.x - 760) / 2.0f, (mWindowSize.y - 520) / 2.0f);
	child->setTitle(mLoc->Get("SOLUTION_DIALOG_TITLE"));
	child->onClose([&]() { isDialogOpen = false; });
	mGui->add(child);

	// 現在の手順セクション
	auto currentLabel = tgui::Label::create(mLoc->Get("SOLUTION_LBL_CURRENT"));
	currentLabel->setRenderer(mTheme->getRenderer("Label"));
	currentLabel->setPosition(16, 12);
	currentLabel->setTextSize(16);
	child->add(currentLabel);

	auto currentArea = tgui::TextArea::create();
	currentArea->setRenderer(mTheme->getRenderer("TextArea"));
	currentArea->setPosition(16, 38);
	currentArea->setSize(728, 110);
	currentArea->setTextSize(15);
	currentArea->setReadOnly(true);
	currentArea->setText(currentLurd);
	child->add(currentArea);

	auto statusLabel = tgui::Label::create("");
	statusLabel->setRenderer(mTheme->getRenderer("Label"));
	statusLabel->setPosition(16, 442);
	statusLabel->setSize(728, 28);
	statusLabel->setTextSize(14);
	child->add(statusLabel);

	auto copyBtn = tgui::Button::create(mLoc->Get("SOLUTION_BTN_COPY"));
	copyBtn->setRenderer(mTheme->getRenderer("Button"));
	copyBtn->setPosition(16, 156);
	copyBtn->setSize(130, 32);
	copyBtn->onPress([&]() {
		if (currentLurd.empty())
		{
			statusLabel->setText(mLoc->Get("SOLUTION_MSG_EMPTY"));
			return;
		}
		sf::Clipboard::setString(currentLurd);
		statusLabel->setText(mLoc->Get("SOLUTION_MSG_COPIED"));
		});
	child->add(copyBtn);

	auto saveBtn = tgui::Button::create(mLoc->Get("SOLUTION_BTN_SAVE"));
	saveBtn->setRenderer(mTheme->getRenderer("Button"));
	saveBtn->setPosition(156, 156);
	saveBtn->setSize(130, 32);
	saveBtn->onPress([&]() {
		if (currentLurd.empty())
		{
			statusLabel->setText(mLoc->Get("SOLUTION_MSG_EMPTY"));
			return;
		}
		child->setVisible(false);
		if (SaveLurdSolutionDialog(currentLurd))
		{
			statusLabel->setText(mLoc->Get("SOLUTION_MSG_SAVED"));
		}
		child->setVisible(true);
		});
	child->add(saveBtn);

	// 読み込み手順セクション
	auto inputLabel = tgui::Label::create(mLoc->Get("SOLUTION_LBL_INPUT"));
	inputLabel->setRenderer(mTheme->getRenderer("Label"));
	inputLabel->setPosition(16, 204);
	inputLabel->setTextSize(16);
	child->add(inputLabel);

	auto inputArea = tgui::TextArea::create();
	inputArea->setRenderer(mTheme->getRenderer("TextArea"));
	inputArea->setPosition(16, 230);
	inputArea->setSize(728, 120);
	inputArea->setTextSize(15);
	inputArea->setText(initialLurd);
	child->add(inputArea);

	auto pasteBtn = tgui::Button::create(mLoc->Get("SOLUTION_BTN_PASTE"));
	pasteBtn->setRenderer(mTheme->getRenderer("Button"));
	pasteBtn->setPosition(16, 358);
	pasteBtn->setSize(130, 32);
	pasteBtn->onPress([&]() {
		inputArea->setText(sf::Clipboard::getString().toAnsiString());
		});
	child->add(pasteBtn);

	auto loadFileBtn = tgui::Button::create(mLoc->Get("SOLUTION_BTN_LOAD_FILE"));
	loadFileBtn->setRenderer(mTheme->getRenderer("Button"));
	loadFileBtn->setPosition(156, 358);
	loadFileBtn->setSize(140, 32);
	loadFileBtn->onPress([&]() {
		bool fileDialogOpen = true;
		std::string loadError;
		auto fd = tgui::FileDialog::create();
		fd->setRenderer(mTheme->getRenderer("FileDialog"));
		fd->setTitle(mLoc->Get("SOLUTION_LOAD_DIALOG_TITLE"));
		fd->setFileMustExist(true);
		fd->setConfirmButtonText(mLoc->Get("BTN_OK"));
		fd->setCancelButtonText(mLoc->Get("BTN_CANCEL"));
		fd->setPath(mPaths.GetLogsDirectory().u8string());
		fd->setFileTypeFilters({
			{ "Solution Text (*.txt, *.lurd, *.sok)", { "*.txt", "*.lurd", "*.sok" } },
			{ "All Files (*.*)", { "*" } }
		});
		fd->onFileSelect([&](const tgui::String& selectedPath) {
			try
			{
				const auto path = std::filesystem::u8path(selectedPath.toStdString());
				const auto bytes = AppPaths::ReadFile(path);
				inputArea->setText(std::string(bytes.begin(), bytes.end()));
			}
			catch (const std::exception& error)
			{
				loadError = error.what();
			}
			fileDialogOpen = false;
			});
		fd->onCancel([&]() { fileDialogOpen = false; });
		fd->onClose([&]() { fileDialogOpen = false; });
		fd->setSize(600, 400);
		fd->setPosition((mWindowSize.x - 600) / 2.0f, (mWindowSize.y - 400) / 2.0f);

		child->setVisible(false);
		mGui->add(fd);
		sf::Time fdTicks = mClock.getElapsedTime();
		while (fileDialogOpen)
		{
			WaitForNextFrame(fdTicks);
			fdTicks = mClock.getElapsedTime();
			sf::Event ev;
			while (mWindow->pollEvent(ev))
			{
				if (ev.type == sf::Event::Closed)
				{
					fileDialogOpen = false;
					isDialogOpen = false;
					mGameState = GameState::EQuit;
				}
				mGui->handleEvent(ev);
			}
			mWindow->clear();
			DrawSprites();
			DrawUI();
			mGui->draw();
			mWindow->display();
		}
		mGui->remove(fd);
		child->setVisible(true);
		if (!loadError.empty())
		{
			statusLabel->setText(loadError);
		}
		});
	child->add(loadFileBtn);

	if (!mCurrentLevelSolutions.empty())
	{
		auto embeddedBtn = tgui::Button::create(mLoc->Get("SOLUTION_BTN_USE_EMBEDDED"));
		embeddedBtn->setRenderer(mTheme->getRenderer("Button"));
		embeddedBtn->setPosition(306, 358);
		embeddedBtn->setSize(160, 32);
		embeddedBtn->onPress([&]() {
			inputArea->setText(mCurrentLevelSolutions.front());
			});
		child->add(embeddedBtn);
	}

	// 適用開始位置の選択ラジオボタン
	auto radioGroup = tgui::RadioButtonGroup::create();
	auto radioInitial = tgui::RadioButton::create();
	radioInitial->setRenderer(mTheme->getRenderer("RadioButton"));
	radioInitial->setText(mLoc->Get("SOLUTION_RADIO_FROM_INITIAL"));
	radioInitial->setPosition(16, 404);
	radioInitial->setChecked(true);
	radioGroup->add(radioInitial);

	auto radioCurrent = tgui::RadioButton::create();
	radioCurrent->setRenderer(mTheme->getRenderer("RadioButton"));
	radioCurrent->setText(mLoc->Get("SOLUTION_RADIO_FROM_CURRENT"));
	radioCurrent->setPosition(260, 404);
	radioGroup->add(radioCurrent);
	child->add(radioGroup);

	// 実行・閉じるボタン
	auto tryApply = [&](bool animate) {
		const std::string raw = inputArea->getText().toStdString();
		const bool fromInit = radioInitial->isChecked();
		const std::vector<std::string> baseBoard = fromInit
			? mInitBoardData.at(mCurrentKey)
			: GetBoardStateWithObjects();
		const auto sim = SolutionCodec::SimulateLurd(baseBoard, raw, false);
		if (!sim.mIsValid)
		{
			statusLabel->setText(mLoc->Get("SOLUTION_MSG_INVALID") + sim.mErrorMessage);
			return;
		}
		if (sim.mSteps.empty())
		{
			statusLabel->setText(mLoc->Get("SOLUTION_MSG_EMPTY"));
			return;
		}
		pendingLurd = raw;
		playbackFromInitial = fromInit;
		playbackAnimate = animate;
		shouldStartPlayback = true;
		isDialogOpen = false;
	};

	auto playBtn = tgui::Button::create(mLoc->Get("SOLUTION_BTN_PLAY"));
	playBtn->setRenderer(mTheme->getRenderer("Button"));
	playBtn->setPosition(344, 476);
	playBtn->setSize(120, 32);
	playBtn->onPress([&]() { tryApply(true); });
	child->add(playBtn);

	auto applyBtn = tgui::Button::create(mLoc->Get("SOLUTION_BTN_APPLY_ALL"));
	applyBtn->setRenderer(mTheme->getRenderer("Button"));
	applyBtn->setPosition(474, 476);
	applyBtn->setSize(130, 32);
	applyBtn->onPress([&]() { tryApply(false); });
	child->add(applyBtn);

	auto closeBtn = tgui::Button::create(mLoc->Get("BTN_CLOSE"));
	closeBtn->setRenderer(mTheme->getRenderer("Button"));
	closeBtn->setPosition(614, 476);
	closeBtn->setSize(130, 32);
	closeBtn->onPress([&]() { isDialogOpen = false; });
	child->add(closeBtn);

	mWindow->setActive(false);
	sf::Time ticksCount = mClock.getElapsedTime();
	while (isDialogOpen)
	{
		WaitForNextFrame(ticksCount);
		ticksCount = mClock.getElapsedTime();

		sf::Event event;
		while (mWindow->pollEvent(event))
		{
			if (event.type == sf::Event::Closed)
			{
				isDialogOpen = false;
				mGameState = GameState::EQuit;
			}
			else if (event.type == sf::Event::KeyPressed && event.key.code == sf::Keyboard::Escape)
			{
				isDialogOpen = false;
			}
			mGui->handleEvent(event);
		}

		mWindow->clear();
		DrawSprites();
		DrawUI();
		mGui->draw();
		mWindow->display();
	}

	mGui->remove(child);
	mWindow->setActive(true);

	if (shouldStartPlayback)
	{
		std::string err;
		if (!ApplyLurdSolution(pendingLurd, playbackFromInitial, playbackAnimate, err))
		{
			ShowFileError(err);
		}
	}
}

// =========================================================================
// 効果音・ライセンス表示・Festival ソルバー関連の実装
// =========================================================================

void Game::PlaySound(SoundManager::SoundEffect effect)
{
	if (mSoundManager)
	{
		mSoundManager->Play(effect);
	}
}

void Game::DisplayLicenseWindow()
{
	CancelAllUIHolds();

	std::string licenseText;
	try
	{
		const auto path = mPaths.Asset("Assets/licenses/THIRD_PARTY_LICENSES.txt");
		const auto bytes = AppPaths::ReadFile(path);
		licenseText.assign(bytes.begin(), bytes.end());
	}
	catch (const std::exception& error)
	{
		licenseText = std::string("Unable to load license file: ") + error.what();
	}

	bool isOpen = true;
	auto child = tgui::ChildWindow::create();
	child->setRenderer(mTheme->getRenderer("ChildWindow"));
	child->setClientSize({ 820, 560 });
	child->setPosition((mWindowSize.x - 820) / 2.0f, (mWindowSize.y - 560) / 2.0f);
	child->setTitle(mLoc->Get("LICENSE_DIALOG_TITLE"));
	child->onClose([&]() { isOpen = false; });
	mGui->add(child);

	auto textArea = tgui::TextArea::create();
	textArea->setRenderer(mTheme->getRenderer("TextArea"));
	textArea->setPosition(14, 14);
	textArea->setSize(792, 490);
	textArea->setTextSize(14);
	textArea->setReadOnly(true);
	textArea->setText(licenseText);
	child->add(textArea);

	auto closeBtn = tgui::Button::create(mLoc->Get("BTN_CLOSE"));
	closeBtn->setRenderer(mTheme->getRenderer("Button"));
	closeBtn->setSize(120, 32);
	closeBtn->setPosition(686, 514);
	closeBtn->onPress([&]() { isOpen = false; });
	child->add(closeBtn);

	mWindow->setActive(false);
	sf::Time ticksCount = mClock.getElapsedTime();
	while (isOpen)
	{
		WaitForNextFrame(ticksCount);
		ticksCount = mClock.getElapsedTime();

		sf::Event event;
		while (mWindow->pollEvent(event))
		{
			if (event.type == sf::Event::Closed)
			{
				isOpen = false;
				mGameState = GameState::EQuit;
			}
			else if (event.type == sf::Event::KeyPressed && event.key.code == sf::Keyboard::Escape)
			{
				isOpen = false;
			}
			mGui->handleEvent(event);
		}

		mWindow->clear();
		DrawSprites();
		DrawUI();
		mGui->draw();
		mWindow->display();
	}

	mGui->remove(child);
	mWindow->setActive(true);
}

void Game::OpenSolverDialog()
{
	CancelAllUIHolds();
	StopSolutionPlayback();

	SolverRunner runner;
	bool isDialogOpen = true;
	bool hasSolvedResult = false;
	bool solvedFromInitial = false;
	std::string solvedLurd;

	bool shouldApplyAfterClose = false;
	bool applyAnimate = true;
	bool applyHintOnly = false;

	auto child = tgui::ChildWindow::create();
	child->setRenderer(mTheme->getRenderer("ChildWindow"));
	child->setClientSize({ 780, 540 });
	child->setPosition((mWindowSize.x - 780) / 2.0f, (mWindowSize.y - 540) / 2.0f);
	child->setTitle(mLoc->Get("SOLVER_DIALOG_TITLE"));
	child->onClose([&]() {
		runner.Cancel();
		isDialogOpen = false;
		});
	mGui->add(child);

	// 開始状態の選択（現在状態 or 初期状態）
	auto radioGroup = tgui::RadioButtonGroup::create();
	auto radioCurrent = tgui::RadioButton::create();
	radioCurrent->setRenderer(mTheme->getRenderer("RadioButton"));
	radioCurrent->setText(mLoc->Get("SOLVER_RADIO_CURRENT"));
	radioCurrent->setPosition(18, 18);
	radioCurrent->setChecked(true);
	radioGroup->add(radioCurrent);

	auto radioInitial = tgui::RadioButton::create();
	radioInitial->setRenderer(mTheme->getRenderer("RadioButton"));
	radioInitial->setText(mLoc->Get("SOLVER_RADIO_INITIAL"));
	radioInitial->setPosition(300, 18);
	radioGroup->add(radioInitial);
	child->add(radioGroup);

	// 制限時間選択
	auto timeLabel = tgui::Label::create(mLoc->Get("SOLVER_LBL_TIME_LIMIT"));
	timeLabel->setRenderer(mTheme->getRenderer("Label"));
	timeLabel->setPosition(18, 56);
	timeLabel->setTextSize(15);
	child->add(timeLabel);

	auto timeCombo = tgui::ComboBox::create();
	timeCombo->setRenderer(mTheme->getRenderer("ComboBox"));
	timeCombo->setPosition(170, 52);
	timeCombo->setSize(120, 28);
	timeCombo->addItem("10", "10");
	timeCombo->addItem("30", "30");
	timeCombo->addItem("60", "60");
	timeCombo->addItem("180", "180");
	timeCombo->addItem("300", "300");
	timeCombo->setSelectedItemById("30");
	child->add(timeCombo);

	// Festival実行ファイルパス表示 & 選択ボタン
	auto exeLabel = tgui::Label::create(mLoc->Get("SOLVER_LBL_EXECUTABLE"));
	exeLabel->setRenderer(mTheme->getRenderer("Label"));
	exeLabel->setPosition(18, 96);
	exeLabel->setTextSize(15);
	child->add(exeLabel);

	auto exeBox = tgui::EditBox::create();
	exeBox->setRenderer(mTheme->getRenderer("EditBox"));
	exeBox->setPosition(170, 92);
	exeBox->setSize(470, 28);
	exeBox->setReadOnly(true);
	child->add(exeBox);

	auto statusLabel = tgui::Label::create("");
	statusLabel->setRenderer(mTheme->getRenderer("Label"));
	statusLabel->setPosition(18, 174);
	statusLabel->setSize(744, 30);
	statusLabel->setTextSize(15);
	child->add(statusLabel);

	auto resultArea = tgui::TextArea::create();
	resultArea->setRenderer(mTheme->getRenderer("TextArea"));
	resultArea->setPosition(18, 210);
	resultArea->setSize(744, 185);
	resultArea->setTextSize(15);
	resultArea->setReadOnly(true);
	child->add(resultArea);

	auto startStopBtn = tgui::Button::create(mLoc->Get("SOLVER_BTN_START"));
	startStopBtn->setRenderer(mTheme->getRenderer("Button"));
	startStopBtn->setPosition(18, 132);
	startStopBtn->setSize(160, 34);
	child->add(startStopBtn);

	auto playBtn = tgui::Button::create(mLoc->Get("SOLUTION_BTN_PLAY"));
	playBtn->setRenderer(mTheme->getRenderer("Button"));
	playBtn->setPosition(18, 408);
	playBtn->setSize(120, 32);
	playBtn->setEnabled(false);
	child->add(playBtn);

	auto hintBtn = tgui::Button::create(mLoc->Get("SOLVER_BTN_HINT"));
	hintBtn->setRenderer(mTheme->getRenderer("Button"));
	hintBtn->setPosition(148, 408);
	hintBtn->setSize(130, 32);
	hintBtn->setEnabled(false);
	child->add(hintBtn);

	auto applyAllBtn = tgui::Button::create(mLoc->Get("SOLUTION_BTN_APPLY_ALL"));
	applyAllBtn->setRenderer(mTheme->getRenderer("Button"));
	applyAllBtn->setPosition(288, 408);
	applyAllBtn->setSize(130, 32);
	applyAllBtn->setEnabled(false);
	child->add(applyAllBtn);

	auto copyBtn = tgui::Button::create(mLoc->Get("SOLUTION_BTN_COPY"));
	copyBtn->setRenderer(mTheme->getRenderer("Button"));
	copyBtn->setPosition(428, 408);
	copyBtn->setSize(110, 32);
	copyBtn->setEnabled(false);
	child->add(copyBtn);

	auto closeBtn = tgui::Button::create(mLoc->Get("BTN_CLOSE"));
	closeBtn->setRenderer(mTheme->getRenderer("Button"));
	closeBtn->setPosition(642, 408);
	closeBtn->setSize(120, 32);
	closeBtn->onPress([&]() {
		runner.Cancel();
		isDialogOpen = false;
		});
	child->add(closeBtn);

	// Festival ライセンス・クレジット表記（必須表示）
	auto creditLabel = tgui::Label::create(mLoc->Get("SOLVER_CREDIT"));
	creditLabel->setRenderer(mTheme->getRenderer("Label"));
	creditLabel->setPosition(18, 494);
	creditLabel->setSize(560, 28);
	creditLabel->setTextSize(14);
	child->add(creditLabel);

	auto licenseBtn = tgui::Button::create(mLoc->Get("BTN_LICENSES"));
	licenseBtn->setRenderer(mTheme->getRenderer("Button"));
	licenseBtn->setPosition(612, 488);
	licenseBtn->setSize(150, 32);
	licenseBtn->onPress([&]() {
		PlaySound(SoundManager::SoundEffect::ButtonClick);
		child->setVisible(false);
		DisplayLicenseWindow();
		child->setVisible(true);
		});
	child->add(licenseBtn);

	std::filesystem::path resolvedExe;
	auto refreshExecutableStatus = [&]() {
		resolvedExe = mPaths.FindFestivalExecutable(mCustomFestivalPath);
		if (!resolvedExe.empty())
		{
			exeBox->setText(resolvedExe.u8string());
			statusLabel->setText(mLoc->Get("SOLVER_STATUS_READY"));
			startStopBtn->setEnabled(true);
		}
		else
		{
			exeBox->setText("");
			statusLabel->setText(mLoc->Get("SOLVER_STATUS_NOT_FOUND"));
			startStopBtn->setEnabled(false);
		}
	};
	refreshExecutableStatus();

	// 任意のFestival実行ファイルを選択するボタン
	auto browseBtn = tgui::Button::create(mLoc->Get("SOLVER_BTN_BROWSE"));
	browseBtn->setRenderer(mTheme->getRenderer("Button"));
	browseBtn->setPosition(650, 92);
	browseBtn->setSize(112, 28);
	browseBtn->onPress([&]() {
		if (runner.GetState() == SolverRunner::SolverState::Running)
		{
			return;
		}
		bool fdOpen = true;
		auto fd = tgui::FileDialog::create();
		fd->setRenderer(mTheme->getRenderer("FileDialog"));
		fd->setTitle(mLoc->Get("SOLVER_BROWSE_TITLE"));
		fd->setFileMustExist(true);
		fd->setConfirmButtonText(mLoc->Get("BTN_OK"));
		fd->setCancelButtonText(mLoc->Get("BTN_CANCEL"));
		fd->onFileSelect([&](const tgui::String& selectedPath) {
			mCustomFestivalPath = std::filesystem::u8path(selectedPath.toStdString());
			fdOpen = false;
			});
		fd->onCancel([&]() { fdOpen = false; });
		fd->onClose([&]() { fdOpen = false; });
		fd->setSize(620, 420);
		fd->setPosition((mWindowSize.x - 620) / 2.0f, (mWindowSize.y - 420) / 2.0f);

		child->setVisible(false);
		mGui->add(fd);
		sf::Time fdTicks = mClock.getElapsedTime();
		while (fdOpen)
		{
			WaitForNextFrame(fdTicks);
			fdTicks = mClock.getElapsedTime();
			sf::Event ev;
			while (mWindow->pollEvent(ev))
			{
				if (ev.type == sf::Event::Closed)
				{
					fdOpen = false;
					isDialogOpen = false;
					mGameState = GameState::EQuit;
				}
				mGui->handleEvent(ev);
			}
			mWindow->clear();
			DrawSprites();
			DrawUI();
			mGui->draw();
			mWindow->display();
		}
		mGui->remove(fd);
		child->setVisible(true);
		refreshExecutableStatus();
		});
	child->add(browseBtn);

	auto setActionButtonsEnabled = [&](bool enabled) {
		playBtn->setEnabled(enabled);
		hintBtn->setEnabled(enabled);
		applyAllBtn->setEnabled(enabled);
		copyBtn->setEnabled(enabled);
	};

	playBtn->onPress([&]() {
		if (!hasSolvedResult || solvedLurd.empty()) return;
		shouldApplyAfterClose = true;
		applyAnimate = true;
		applyHintOnly = false;
		isDialogOpen = false;
		});
	hintBtn->onPress([&]() {
		if (!hasSolvedResult || solvedLurd.empty()) return;
		shouldApplyAfterClose = true;
		applyAnimate = true;
		applyHintOnly = true;
		isDialogOpen = false;
		});
	applyAllBtn->onPress([&]() {
		if (!hasSolvedResult || solvedLurd.empty()) return;
		shouldApplyAfterClose = true;
		applyAnimate = false;
		applyHintOnly = false;
		isDialogOpen = false;
		});
	copyBtn->onPress([&]() {
		if (!hasSolvedResult || solvedLurd.empty()) return;
		sf::Clipboard::setString(solvedLurd);
		statusLabel->setText(mLoc->Get("SOLUTION_MSG_COPIED"));
		});

	bool wasRunning = false;
	startStopBtn->onPress([&]() {
		if (runner.GetState() == SolverRunner::SolverState::Running)
		{
			runner.Cancel();
			return;
		}
		if (resolvedExe.empty())
		{
			refreshExecutableStatus();
			return;
		}

		const bool useInitial = radioInitial->isChecked();
		const std::vector<std::string> boardToSolve = useInitial
			? mInitBoardData.at(mCurrentKey)
			: GetBoardStateWithObjects();

		// 既に全荷物がゴール上にあるか事前確認
		bool alreadyComplete = true;
		for (const auto& row : boardToSolve)
		{
			if (row.find('$') != std::string::npos)
			{
				alreadyComplete = false;
				break;
			}
		}
		if (alreadyComplete)
		{
			statusLabel->setText(mLoc->Get("SOLVER_STATUS_ALREADY_COMPLETE"));
			return;
		}

		int timeLimitSec = 30;
		try
		{
			const std::string idStr = timeCombo->getSelectedItemId().toStdString();
			if (!idStr.empty())
			{
				timeLimitSec = std::stoi(idStr);
			}
		}
		catch (const std::exception&)
		{
			timeLimitSec = 30;
		}

		hasSolvedResult = false;
		solvedLurd.clear();
		solvedFromInitial = useInitial;
		resultArea->setText("");
		setActionButtonsEnabled(false);

		SolverRunner::SolverRequest req{};
		req.mExecutablePath = resolvedExe;
		req.mStartBoard = boardToSolve;
		req.mTimeLimitSeconds = timeLimitSec;
		req.mCores = 1;

		if (runner.Start(req))
		{
			wasRunning = true;
			startStopBtn->setText(mLoc->Get("SOLVER_BTN_STOP"));
			radioCurrent->setEnabled(false);
			radioInitial->setEnabled(false);
			timeCombo->setEnabled(false);
			browseBtn->setEnabled(false);
		}
		});

	mWindow->setActive(false);
	sf::Time ticksCount = mClock.getElapsedTime();
	while (isDialogOpen)
	{
		WaitForNextFrame(ticksCount);
		ticksCount = mClock.getElapsedTime();

		const auto state = runner.GetState();
		if (state == SolverRunner::SolverState::Running)
		{
			std::ostringstream oss;
			oss << mLoc->Get("SOLVER_STATUS_RUNNING")
				<< std::fixed << std::setprecision(1) << runner.GetElapsedSeconds() << "s";
			statusLabel->setText(oss.str());
		}
		else if (wasRunning)
		{
			wasRunning = false;
			startStopBtn->setText(mLoc->Get("SOLVER_BTN_START"));
			radioCurrent->setEnabled(true);
			radioInitial->setEnabled(true);
			timeCombo->setEnabled(true);
			browseBtn->setEnabled(true);

			const auto res = runner.GetResult();
			if (res.mState == SolverRunner::SolverState::Succeeded)
			{
				hasSolvedResult = true;
				solvedLurd = res.mNormalizedLurd;
				std::ostringstream summary;
				summary << mLoc->Get("SOLVER_STATUS_SOLVED") << "  "
					<< mLoc->Get("SOLVER_LBL_RESULT_MOVES") << res.mMoveCount << "  "
					<< mLoc->Get("SOLVER_LBL_RESULT_PUSHES") << res.mPushCount << "  "
					<< mLoc->Get("SOLVER_LBL_RESULT_TIME") << std::fixed << std::setprecision(2) << res.mElapsedSeconds << "s";
				statusLabel->setText(summary.str());
				resultArea->setText(solvedLurd);
				setActionButtonsEnabled(true);
				PlaySound(SoundManager::SoundEffect::GoalIn);
			}
			else if (res.mState == SolverRunner::SolverState::TimedOut)
			{
				statusLabel->setText(mLoc->Get("SOLVER_STATUS_TIMEOUT"));
			}
			else if (res.mState == SolverRunner::SolverState::Cancelled)
			{
				statusLabel->setText(mLoc->Get("SOLVER_STATUS_CANCELLED"));
			}
			else
			{
				statusLabel->setText(mLoc->Get("SOLVER_STATUS_FAILED") + res.mErrorMessage);
			}
		}

		sf::Event event;
		while (mWindow->pollEvent(event))
		{
			if (event.type == sf::Event::Closed)
			{
				runner.Cancel();
				isDialogOpen = false;
				mGameState = GameState::EQuit;
			}
			else if (event.type == sf::Event::KeyPressed && event.key.code == sf::Keyboard::Escape)
			{
				runner.Cancel();
				isDialogOpen = false;
			}
			mGui->handleEvent(event);
		}

		mWindow->clear();
		DrawSprites();
		DrawUI();
		mGui->draw();
		mWindow->display();
	}

	runner.Cancel();
	mGui->remove(child);
	mWindow->setActive(true);

	if (shouldApplyAfterClose && !solvedLurd.empty())
	{
		const std::string toApply = applyHintOnly ? solvedLurd.substr(0, 1) : solvedLurd;
		std::string err;
		if (!ApplyLurdSolution(toApply, solvedFromInitial, applyAnimate, err))
		{
			ShowFileError(err);
		}
	}
}

void Game::AddSnapshot(const std::string& name, const std::string& comment)
{
	GameSnapshot snapshot;

	// UI用データを設定
	snapshot.snapshotName = name;
	snapshot.snapshotComment = comment;
	snapshot.timestamp = GetDateTime(); // 既存のヘルパー関数を利用

	// 復元用データを保存
	snapshot.playerPosition = mPlayer->GetBoardCoordinate();
	snapshot.playerDirection = mPlayer->GetDirection();
	snapshot.stepCount = mStep;
	snapshot.logs = mLogs;

	// 荷物の座標を保存
	for (const auto& baggage : mBaggages)
	{
		snapshot.baggagePositions.emplace_back(baggage->GetBoardCoordinate());
	}

	mSnapshots.emplace_back(snapshot);
	std::cout << "Snapshot saved: " << name << std::endl;
}

void Game::DisplaySnapshot()
{
	// 新しいSnapshotScreenのインスタンスを作成
	// コンストラクタ内で、UIスタックへのプッシュと
	// ゲーム状態の EPaused への設定が行われる
	new SnapshotScreen(this, mWindow);

	std::cout << "DisplaySnapshot called. Game state set to EPaused." << std::endl;
}

void Game::AddSnapshotDialog()
{
	// 新しいAddSnapshotDialogのインスタンスを作成
	// コンストラクタ内で、ダイアログ自身がUIスタックにプッシュされ、
	// ゲーム状態が EPaused に設定される
	new SnapshotDialog(this, mWindow);

	std::cout << "DisplayAddSnapshotDialog called. Game state set to EPaused." << std::endl;
}

void Game::RestoreSnapshot(int snapshotIndex)
{
	if (snapshotIndex < 0 || snapshotIndex >= mSnapshots.size())
	{
		return; // 無効なインデックス
	}

	const auto& snapshot = mSnapshots[snapshotIndex];

	// プレイヤーの状態を復元
	mPlayer->SetBoardCoordinate(snapshot.playerPosition);
	mPlayer->SetDirection(snapshot.playerDirection);

	// 荷物の状態を復元 (mBaggagesの順序が変わらないことが前提)
	for (int i = 0; i < mBaggages.size(); ++i)
	{
		if (i < snapshot.baggagePositions.size())
		{
			mBaggages[i]->SetBoardCoordinate(snapshot.baggagePositions[i]);
		}
	}

	// ゲームの状態を復元
	mStep = snapshot.stepCount;
	mLogs = snapshot.logs;

	// ハイライトなどをクリア
	ClearMoveHighlights();
	ClearPushHighlights();
	ClearPushDirections();
	mPlayer->SetIdleState();

	std::cout << "Snapshot restored: " << snapshot.snapshotName << std::endl;
}

void Game::ClearSnapshots()
{
	mSnapshots.clear();
	std::cout << "All snapshots cleared." << std::endl;
}

sf::Texture* Game::GenerateThumbnail(const GameSnapshot& snapshotData)
{
	// 盤面テクスチャを取得（これが背景になる）
	sf::Texture* boardTexture = mGameBoard->GetBoardTexture();
	if (!boardTexture)
	{
		std::cout << "Game::GenerateThumbnail: Failed get board texture!" << std::endl;
		return nullptr;
	}

	sf::Vector2u texSize = boardTexture->getSize();

	// 1. 描画ターゲットとしてのRenderTextureを作成
	sf::RenderTexture* rt = new sf::RenderTexture();
	if (!rt->create(texSize.x, texSize.y))
	{
		std::cout << "Game::GenerateThumbnail: Failed create render texture!" << std::endl;
		delete rt;
		return nullptr;
	}
	rt->clear(sf::Color::Transparent);

	// 2. 背景（盤面）を描画
	sf::Sprite boardSprite(*boardTexture);
	// 上下反転
	boardSprite.setScale(1.f, -1.f);
	// そのままだと上下がずれるので、原点を画像の高さにする
	boardSprite.setOrigin(0.0f, static_cast<float>(boardTexture->getSize().y));
	rt->draw(boardSprite);

	// 3. 荷物を描画
	// 荷物の数が0出なければ描画
	if (!mBaggages.empty())
	{
		for (const auto& pos : snapshotData.baggagePositions)
		{
			// Baggageクラスからテクスチャを取得
			sf::Texture* baggageTex(nullptr);
			if (std::find(mGoalPos.begin(), mGoalPos.end(), pos) == mGoalPos.end())
			{
				baggageTex = mBaggages.front()->GetBaggageTexture(Baggage::BState::OnFloor);
			}
			else
			{
				baggageTex = mBaggages.front()->GetBaggageTexture(Baggage::BState::OnGoal);
			}
			sf::Sprite baggageSprite(*baggageTex);

			// タイル座標をピクセル座標に変換
			sf::Vector2f pixelPos(static_cast<float>(pos.x * baggageTex->getSize().x), static_cast<float>(pos.y * baggageTex->getSize().y));
			baggageSprite.setPosition(pixelPos);
			rt->draw(baggageSprite);
		}
	}

	// 4. プレイヤーを描画
	sf::Texture* playerTex = mPlayer->GetTextureForDirection(snapshotData.playerDirection);
	if (playerTex)
	{
		sf::Sprite playerSprite(*playerTex);
		sf::Vector2f pixelPos(static_cast<float>(snapshotData.playerPosition.x * playerTex->getSize().x), static_cast<float>(snapshotData.playerPosition.y * playerTex->getSize().y));
		playerSprite.setPosition(pixelPos);
		rt->draw(playerSprite);
	}

	// 5. 描画を確定
	rt->display();

	// 6. RenderTextureからTextureをコピーして返す
	sf::Texture* thumbnail = new sf::Texture(rt->getTexture());
	delete rt; // RenderTextureは不要になったので破棄

	std::cout << "Game::GenerateThumbnail: Created thumbnail texture!" << std::endl;
	return thumbnail;
}

sf::Texture* Game::GenerateCurrentThumbnail()
{
	// 現在の状態からダミーのスナップショットを作ってGenerateThumbnailに渡す
	GameSnapshot currentData;
	currentData.playerPosition = mPlayer->GetBoardCoordinate();
	currentData.playerDirection = mPlayer->GetDirection();
	for (const auto& baggage : mBaggages)
	{
		currentData.baggagePositions.push_back(baggage->GetBoardCoordinate());
	}
	return GenerateThumbnail(currentData);
}

sf::Vector2u Game::GetThumbnailSize() const
{
	// 盤面テクスチャを取得（これが背景になる）
	sf::Texture* boardTexture = mGameBoard->GetBoardTexture();
	if (!boardTexture)
	{
		std::cout << "Game::GetThumbnailSize: Failed get board texture!" << std::endl;
		return sf::Vector2u{ 0, 0 };
	}
	return boardTexture->getSize();
}

void Game::CallEditorSetup()
{
	// エディタの初期設定ダイアログを起動
	new EditorSetupDialog(this, mWindow);
}

void Game::DisplayEditorScreen(const int mode)
{
	// エディタを起動
	new EditorScreen(this, mWindow, mode);
	std::cout << "Display EditorScreen from Game::DisplayEditorScreen()!" << std::endl;
}

void Game::ApplyEditedBoard(const std::vector<std::string>& newBoardData)
{
	StopSolutionPlayback();
	mCurrentLevelSolutions.clear();
	// 一時的な名前で盤面リストに追加
	std::string tempName = "Applied_" + GetDateTime();
	AddBoard(tempName, newBoardData);

	// カレントキーを更新
	mCurrentKey = tempName;

	// ログをファイル出力してから全て消す
	OutputLogs();
	ClearParameters();

	// スナップショットもリセット
	ClearSnapshots();

	// 生成された盤面のキーは時刻を文字列に変換したものにする
	mCurrentKey = GetDateTime();
	mFilenames.emplace_back(mCurrentKey);
	mBoardData.emplace(mCurrentKey, newBoardData);
	mInitBoardData.emplace(mCurrentKey, newBoardData);
	mBaggageLimit = GetBaggageNumLimit(mGenParams.boardSize, mGenParams.wallRate);

	// 盤面の初期状態をセット
	mGenParams.boardSize = sf::Vector2i{ static_cast<int>(newBoardData.front().length()), static_cast<int>(newBoardData.size()) };
	std::vector<sf::Vector2i> mBoxesPos;
	{
		int i = 0, j = 0;
		std::string tmpstr = "";
		for (const auto& line : newBoardData)
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
		mBaggages.pop_back();
	}

	for (const auto& item : mBoxesPos)
	{
		mBaggages.emplace_back(new Baggage(this, item));
		mInitialBaggagePos.emplace(mBaggages.back(), item);
	}

	// HUDHelperを再構築
	mHUDHelper = new HUDHelper(this);

	mStart = std::chrono::system_clock::now();

	std::cout << "Applied edited board: " << tempName << std::endl;
}

std::string Game::GetDateTime()
{
	// 日付と時刻を取得
	// 現在の日付と時刻を取得
	auto now = std::chrono::system_clock::now();
	std::time_t now_time = std::chrono::system_clock::to_time_t(now);

	// 年月日と時刻を文字列にフォーマット
	std::tm timeinfo = PlatformTime::GetLocalTime(now_time);
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

	if (mIsComplete && !mWasComplete)
	{
		PlaySound(SoundManager::SoundEffect::Clear);
	}
	mWasComplete = mIsComplete;
}

void Game::DisplayResult()
{
	// ボタンの長押しを解除
	CancelAllUIHolds();

	bool isChildWindowOpened = true;
	bool isPlayLog = false;
	bool isSelectStage = false; // ステージ選択へ行くかどうかのフラグ

	// 表示用のウィンドウを作成
	auto child = tgui::ChildWindow::create();
	child->setRenderer(mTheme->getRenderer("ChildWindow"));
	child->setClientSize({ 960, 540 });
	child->setPosition(420, 80);
	child->setTitle(mLoc->Get("RESULT_WINDOW_TITLE"));
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
	listBox->addItem(mLoc->Get("RESULT_WINDOW_LBL_TEXT"));
	listBox->addItem(mLoc->Get("RESULT_WINDOW_LBL_STEPS") + " " + std::to_string(mStep));
	long time = static_cast<long>(GetSecTime());
	std::stringstream m{}, s{};
	m << std::setw(2) << std::setfill('0') << time / 60;
	s << std::setw(2) << std::setfill('0') << time % 60;
	listBox->addItem(mLoc->Get("RESULT_WINDOW_LBL_TIME") + " " + m.str() + ":" + s.str());
	listBox->addItem("");
	listBox->addItem("");
	child->add(listBox);

	// 入力終了用ボタン
	auto exitButton = tgui::Button::create(mLoc->Get("BTN_CLOSE"));
	exitButton->setRenderer(mTheme->getRenderer("Button"));
	exitButton->setSize(120, 30);
	exitButton->setPosition(static_cast<int>(child->getSize().x - exitButton->getSize().x) - 10, static_cast<int>(listBox->getSize().y) + 5);
	exitButton->onPress([&]() {
		std::cout << "Exit child window action triggered!" << std::endl;
		isChildWindowOpened = false;
		});
	child->add(exitButton);

	// プレイ履歴を再生するモードに移るボタン
	auto playLogButton = tgui::Button::create(mLoc->Get("RESULT_WINDOW_BTN_PLAY_LOG"));
	playLogButton->setRenderer(mTheme->getRenderer("Button"));
	playLogButton->setSize(120, 30);
	playLogButton->setPosition(static_cast<int>(child->getSize().x - exitButton->getSize().x * 2.0f) - 25, static_cast<int>(listBox->getSize().y) + 5);
	playLogButton->onPress([&]() {
		std::cout << "Play Log action triggered!" << std::endl;
		isChildWindowOpened = false;
		isPlayLog = true;
		});
	child->add(playLogButton);

	// ステージ選択画面を呼び出すボタン
	auto boardSelectButton = tgui::Button::create(mLoc->Get("RESULT_WINDOW_BTN_BOARD_SELECT"));
	boardSelectButton->setRenderer(mTheme->getRenderer("Button"));
	boardSelectButton->setSize(140, 30);
	boardSelectButton->setPosition(static_cast<int>(child->getSize().x - 120 * 2 - 140) - 40, static_cast<int>(listBox->getSize().y) + 5);
	boardSelectButton->onPress([&]() {
		std::cout << "Call board select window triggered!" << std::endl;
		isChildWindowOpened = false;	// ステージ選択ウィンドウを出す前にリザルト画面を消すため
		isSelectStage = true;
		});
	child->add(boardSelectButton);

	// 解手順 (rludRLUD) をクリップボードへコピーするボタン
	const std::string clearLurd = GetCurrentLurdSolution(true);
	auto copyLurdButton = tgui::Button::create(mLoc->Get("RESULT_WINDOW_BTN_COPY_LURD"));
	copyLurdButton->setRenderer(mTheme->getRenderer("Button"));
	copyLurdButton->setSize(130, 30);
	copyLurdButton->setPosition(10, static_cast<int>(listBox->getSize().y) + 5);
	copyLurdButton->onPress([&]() {
		if (!clearLurd.empty())
		{
			sf::Clipboard::setString(clearLurd);
			listBox->addItem(mLoc->Get("SOLUTION_MSG_COPIED"));
		}
		});
	child->add(copyLurdButton);

	// 解手順 (rludRLUD) をファイルへ保存するボタン
	auto saveLurdButton = tgui::Button::create(mLoc->Get("RESULT_WINDOW_BTN_SAVE_LURD"));
	saveLurdButton->setRenderer(mTheme->getRenderer("Button"));
	saveLurdButton->setSize(130, 30);
	saveLurdButton->setPosition(150, static_cast<int>(listBox->getSize().y) + 5);
	saveLurdButton->onPress([&]() {
		if (!clearLurd.empty())
		{
			child->setVisible(false);
			if (SaveLurdSolutionDialog(clearLurd))
			{
				listBox->addItem(mLoc->Get("SOLUTION_MSG_SAVED"));
			}
			child->setVisible(true);
		}
		});
	child->add(saveLurdButton);

	// ゲームループを停止
	mWindow->setActive(false);

	// デルタタイム関連
	sf::Time ticksCount = mClock.getElapsedTime();

	// 入力と更新とウィンドウの終了処理
	// ウィンドウが閉じるまでループ
	while (isChildWindowOpened)
	{
		// 60FPSに合わせて遅延をかける
		WaitForNextFrame(ticksCount);

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
	else if (isSelectStage)
	{
		SelectBoards();
	}

	// 1手戻す
	CallUndo();

	// ゲームループを再開
	mWindow->setActive(true);
}

bool Game::InputBoardData()
{
	BoardConfigDialog dialog(this, *mGui, *mTheme, mGenParams);

	auto [result, newParams] = dialog.run(*mWindow, mClock);

	if (result == BoardConfigDialog::Result::Apply)
	{
		mGenParams = newParams; // 構造体を一括更新
		return true;
	}

	return false;

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

void Game::CancelAllUIHolds()
{

	// UIスタックの中からTHUDを探して、長押し状態を解除する
	for (auto uiScreen : mUIStack)
	{
		// dynamic_castを使って、IUIScreen*がTHUD*に変換できるか試す
		if (THUD* thud = dynamic_cast<THUD*>(uiScreen))
		{
			thud->CancelButtonHolds();
			break; // THUDは一つしかないので見つけたらループを抜ける
		}
	}
}

void Game::DisplayHelpWindow()
{
	// テキストデータの読み込み
	const auto filename = mPaths.Asset(mLoc->Get("PATH_HELP"));
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
	child->setTitle(mLoc->Get("HELP_WINDOW_TITLE"));
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
	listBox->setAutoScroll(false);
	for (const auto& row : helpTexts)
	{
		listBox->addItem(row);
	}
	child->add(listBox);

	// 入力終了用ボタン
	auto exitButton = tgui::Button::create(mLoc->Get("BTN_CLOSE"));
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
		WaitForNextFrame(ticksCount);

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

sf::Texture Game::GenerateBoardPreviewTexture(const std::vector<std::string>& lines) const
{
	sf::Vector2u tileSize = mTextures.at("Assets/Floor.png")->getSize();
	sf::Vector2u totalSize{ 0, static_cast<unsigned int>(lines.size()) };
	for (const auto& line : lines)
	{
		if (totalSize.x < static_cast<unsigned int>(line.length()))
		{
			totalSize.x = static_cast<unsigned int>(line.length());
		}
	}
	if (totalSize.x == 0 || totalSize.y == 0)
	{
		throw std::runtime_error("Cannot generate preview for empty board");
	}

	sf::RenderTexture renderTexture{};
	if (!renderTexture.create(totalSize.x * tileSize.x, totalSize.y * tileSize.y))
	{
		throw std::runtime_error("Failed to create board preview texture");
	}
	renderTexture.clear(sf::Color::Transparent);

	for (int y = 0; y < static_cast<int>(lines.size()); ++y)
	{
		for (int x = 0; x < static_cast<int>(lines[y].length()); ++x)
		{
			std::vector<sf::Texture*> currentTex{};
			switch (lines[y][x])
			{
			case ' ':
				currentTex.emplace_back(mTextures.at("Assets/Floor.png"));
				break;
			case '#':
				currentTex.emplace_back(mTextures.at("Assets/Wall.png"));
				break;
			case '.':
				currentTex.emplace_back(mTextures.at("Assets/Goal.png"));
				break;
			case '$':
				currentTex.emplace_back(mTextures.at("Assets/Floor.png"));
				currentTex.emplace_back(mTextures.at("Assets/Box.png"));
				break;
			case '*':
				currentTex.emplace_back(mTextures.at("Assets/Goal.png"));
				currentTex.emplace_back(mTextures.at("Assets/ShinyBox.png"));
				break;
			case '@':
				currentTex.emplace_back(mTextures.at("Assets/Floor.png"));
				currentTex.emplace_back(mTextures.at("Assets/playerN.png"));
				break;
			case '+':
				currentTex.emplace_back(mTextures.at("Assets/Goal.png"));
				currentTex.emplace_back(mTextures.at("Assets/playerN.png"));
				break;
			default:
				break;
			}

			for (auto* tmpTex : currentTex)
			{
				sf::Sprite tmpSprite(*tmpTex);
				tmpSprite.setPosition(static_cast<float>(x * tileSize.x), static_cast<float>(y * tileSize.y));
				renderTexture.draw(tmpSprite);
			}
		}
	}

	renderTexture.display();
	return renderTexture.getTexture();
}

void Game::SelectBoards()
{
	CancelAllUIHolds();

	// 案A（2段階選択）: ファイルコレクション一覧 -> 収録レベル一覧 -> オンデマンドプレビュー
	struct FileEntry
	{
		std::string mFileKey;
		std::string mLabel;
		LevelFormat::LevelCollection mCollection;
	};
	std::map<std::string, FileEntry> fileEntries{};
	std::unordered_map<std::string, sf::Texture> previewCache{};

	auto addCollectionEntry = [&](const std::string& fileKey, const std::string& baseLabel, const std::filesystem::path& path, bool reportError) -> bool {
		try
		{
			LevelFormat::LevelCollection col = LevelFormat::LoadCollectionFromFile(path);
			std::string displayLabel = baseLabel;
			if (col.mLevels.size() > 1)
			{
				displayLabel += " (" + std::to_string(col.mLevels.size()) + ")";
			}
			fileEntries[fileKey] = FileEntry{ fileKey, displayLabel, std::move(col) };
			return true;
		}
		catch (const std::exception& e)
		{
			std::cerr << "Skipped unreadable level file '" << path.u8string() << "': " << e.what() << std::endl;
			if (reportError)
			{
				ShowFileError(e.what());
			}
			return false;
		}
	};

	try
	{
		for (const auto& boardFile : mPaths.GetBoardFiles())
		{
			const std::string baseLabel = mLoc->Get(boardFile.saved ? "BOARD_SOURCE_SAVED" : "BOARD_SOURCE_BUNDLED")
				+ " " + boardFile.path.filename().u8string();
			addCollectionEntry(boardFile.key, baseLabel, boardFile.path, false);
		}
	}
	catch (const std::exception& e)
	{
		ShowFileError(e.what());
		return;
	}

	std::string selectedFileKey{};
	std::size_t selectedLevelIndex = 0;
	bool hasValidSelection = false;
	bool isChildWindowOpened = true;

	// 表示用のウィンドウを作成
	auto child = tgui::ChildWindow::create();
	child->setRenderer(mTheme->getRenderer("ChildWindow"));
	child->setClientSize({ 1020, 560 });
	child->setPosition((mWindowSize.x - 1020) / 2.0f, (mWindowSize.y - 560) / 2.0f);
	child->setTitle(mLoc->Get("BOARD_SELECT_WINDOW_TITLE"));
	child->onClose([&isChildWindowOpened]() {
		isChildWindowOpened = false;
		});
	mGui->add(child);

	// 左ペイン: ファイル一覧ラベルとListBox
	auto filesLabel = tgui::Label::create(mLoc->Get("BOARD_SELECT_LBL_FILES"));
	filesLabel->setRenderer(mTheme->getRenderer("Label"));
	filesLabel->setPosition(10, 6);
	filesLabel->setTextSize(15);
	child->add(filesLabel);

	auto fileListBox = tgui::ListBox::create();
	fileListBox->setRenderer(mTheme->getRenderer("ListBox"));
	fileListBox->setSize(260, 476);
	fileListBox->setItemHeight(24);
	fileListBox->setPosition(10, 30);
	fileListBox->setTextSize(16);
	fileListBox->setAutoScroll(false);
	for (const auto& item : fileEntries)
	{
		fileListBox->addItem(item.second.mLabel, item.first);
	}
	child->add(fileListBox);

	// 中ペイン: 収録レベル一覧ラベルとListBox（複数レベル収録ファイル対応）
	auto levelsLabel = tgui::Label::create(mLoc->Get("BOARD_SELECT_LBL_LEVELS"));
	levelsLabel->setRenderer(mTheme->getRenderer("Label"));
	levelsLabel->setPosition(280, 6);
	levelsLabel->setTextSize(15);
	child->add(levelsLabel);

	auto levelListBox = tgui::ListBox::create();
	levelListBox->setRenderer(mTheme->getRenderer("ListBox"));
	levelListBox->setSize(220, 476);
	levelListBox->setItemHeight(24);
	levelListBox->setPosition(280, 30);
	levelListBox->setTextSize(16);
	levelListBox->setAutoScroll(false);
	child->add(levelListBox);

	// 右ペイン: 盤面プレビューと詳細情報ラベル
	const BoundingBox previewArea{
		sf::Vector2f{ 510.0f, 30.0f },
		sf::Vector2f{ 1010.0f, 470.0f }
	};
	auto picture = tgui::Picture::create();
	picture->setSize(previewArea.second.x - previewArea.first.x, previewArea.second.y - previewArea.first.y);
	picture->setPosition(previewArea.first.x, previewArea.first.y);
	child->add(picture);

	auto infoLabel = tgui::Label::create("");
	infoLabel->setRenderer(mTheme->getRenderer("Label"));
	infoLabel->setPosition(510, 478);
	infoLabel->setSize(500, 28);
	infoLabel->setTextSize(15);
	child->add(infoLabel);

	auto updatePreview = [&](const std::string& fileKey, std::size_t levelIdx) {
		hasValidSelection = false;
		auto it = fileEntries.find(fileKey);
		if (it == fileEntries.end() || levelIdx >= it->second.mCollection.mLevels.size())
		{
			infoLabel->setText("");
			return;
		}

		const auto& level = it->second.mCollection.mLevels[levelIdx];
		const std::string cacheKey = fileKey + "#" + std::to_string(levelIdx);
		try
		{
			if (!previewCache.count(cacheKey))
			{
				previewCache.emplace(cacheKey, GenerateBoardPreviewTexture(level.mBoard));
			}
			const sf::Texture& tex = previewCache.at(cacheKey);
			sf::Image tmpImage = tex.copyToImage();
			tgui::Texture tmpTGuiTex{};
			tmpTGuiTex.loadFromPixelData(tmpImage.getSize(), tmpImage.getPixelsPtr());
			picture->getRenderer()->setTexture(tmpTGuiTex);

			const float areaW = previewArea.second.x - previewArea.first.x;
			const float areaH = previewArea.second.y - previewArea.first.y;
			const float scaleFactor = std::min(areaW / static_cast<float>(tmpImage.getSize().x),
				areaH / static_cast<float>(tmpImage.getSize().y));
			const float drawW = static_cast<float>(tmpImage.getSize().x) * scaleFactor;
			const float drawH = static_cast<float>(tmpImage.getSize().y) * scaleFactor;
			picture->setSize(drawW, drawH);
			picture->setPosition(previewArea.first.x + (areaW - drawW) / 2.0f,
				previewArea.first.y + (areaH - drawH) / 2.0f);

			const std::size_t w = level.mBoard.empty() ? 0 : level.mBoard.front().size();
			const std::size_t h = level.mBoard.size();
			std::string info = level.mTitle + "  |  "
				+ mLoc->Get("BOARD_SELECT_LBL_INFO_SIZE") + std::to_string(w) + "x" + std::to_string(h);
			if (!level.mSolutions.empty())
			{
				info += "  |  " + mLoc->Get("BOARD_SELECT_LBL_HAS_SOLUTIONS") + std::to_string(level.mSolutions.size());
			}
			infoLabel->setText(info);

			selectedFileKey = fileKey;
			selectedLevelIndex = levelIdx;
			hasValidSelection = true;
		}
		catch (const std::exception& error)
		{
			infoLabel->setText(error.what());
		}
	};

	auto populateLevelsForFile = [&](const std::string& fileKey) {
		levelListBox->removeAllItems();
		hasValidSelection = false;
		auto it = fileEntries.find(fileKey);
		if (it == fileEntries.end())
		{
			return;
		}
		const auto& levels = it->second.mCollection.mLevels;
		for (std::size_t i = 0; i < levels.size(); ++i)
		{
			const std::string itemLabel = (levels.size() > 1)
				? ("#" + std::to_string(i + 1) + " " + levels[i].mTitle)
				: levels[i].mTitle;
			levelListBox->addItem(itemLabel, std::to_string(i));
		}
		if (!levels.empty())
		{
			levelListBox->setSelectedItemByIndex(0);
			updatePreview(fileKey, 0);
		}
	};

	// ファイル選択イベント（左ペイン）
	fileListBox->onItemSelect([&](const tgui::String&, const tgui::String& fileId) {
		const std::string key = fileId.toStdString();
		if (!key.empty())
		{
			selectedFileKey = key;
			populateLevelsForFile(key);
		}
		});

	// 収録レベル選択イベント（中ペイン）
	levelListBox->onItemSelect([&](const tgui::String&, const tgui::String& levelId) {
		if (selectedFileKey.empty() || levelId.empty())
		{
			return;
		}
		try
		{
			const std::size_t idx = static_cast<std::size_t>(std::stoul(levelId.toStdString()));
			updatePreview(selectedFileKey, idx);
		}
		catch (const std::exception&)
		{
		}
		});

	// 外部の .sok / .xsb / .txt ファイルを直接開くボタン
	auto openExternalButton = tgui::Button::create(mLoc->Get("BTN_OPEN_EXTERNAL_FILE"));
	openExternalButton->setRenderer(mTheme->getRenderer("Button"));
	openExternalButton->setSize(180, 32);
	openExternalButton->setPosition(10, 518);
	openExternalButton->onPress([&]() {
		bool fdOpen = true;
		std::filesystem::path chosenPath;
		auto fd = tgui::FileDialog::create();
		fd->setRenderer(mTheme->getRenderer("FileDialog"));
		fd->setTitle(mLoc->Get("BOARD_OPEN_DIALOG_TITLE"));
		fd->setFileMustExist(true);
		fd->setConfirmButtonText(mLoc->Get("BTN_OK"));
		fd->setCancelButtonText(mLoc->Get("BTN_CANCEL"));
		fd->setPath(mPaths.GetBoardsDirectory().u8string());
		fd->setFileTypeFilters({
			{ "Sokoban Levels (*.sok, *.xsb, *.txt)", { "*.sok", "*.xsb", "*.txt" } },
			{ "SOK (*.sok)", { "*.sok" } },
			{ "XSB (*.xsb)", { "*.xsb" } },
			{ "Text (*.txt)", { "*.txt" } }
		});
		fd->onFileSelect([&](const tgui::String& selectedPath) {
			chosenPath = std::filesystem::u8path(selectedPath.toStdString());
			fdOpen = false;
			});
		fd->onCancel([&]() { fdOpen = false; });
		fd->onClose([&]() { fdOpen = false; });
		fd->setSize(620, 420);
		fd->setPosition((mWindowSize.x - 620) / 2.0f, (mWindowSize.y - 420) / 2.0f);

		child->setVisible(false);
		mGui->add(fd);
		sf::Time fdTicks = mClock.getElapsedTime();
		while (fdOpen)
		{
			WaitForNextFrame(fdTicks);
			fdTicks = mClock.getElapsedTime();
			sf::Event ev;
			while (mWindow->pollEvent(ev))
			{
				if (ev.type == sf::Event::Closed)
				{
					fdOpen = false;
					isChildWindowOpened = false;
					mGameState = GameState::EQuit;
				}
				mGui->handleEvent(ev);
			}
			mWindow->clear();
			DrawSprites();
			DrawUI();
			mGui->draw();
			mWindow->display();
		}
		mGui->remove(fd);
		child->setVisible(true);

		if (!chosenPath.empty())
		{
			const std::string extKey = "external:" + chosenPath.u8string();
			const std::string baseLabel = "[File] " + chosenPath.filename().u8string();
			if (addCollectionEntry(extKey, baseLabel, chosenPath, true))
			{
				if (!fileListBox->containsId(extKey))
				{
					fileListBox->addItem(fileEntries.at(extKey).mLabel, extKey);
				}
				fileListBox->setSelectedItemById(extKey);
				selectedFileKey = extKey;
				populateLevelsForFile(extKey);
			}
		}
		});
	child->add(openExternalButton);

	// 適用ボタン
	auto applyButton = tgui::Button::create(mLoc->Get("BTN_APPLY"));
	applyButton->setRenderer(mTheme->getRenderer("Button"));
	applyButton->setSize(120, 32);
	applyButton->setPosition(760, 518);
	applyButton->onPress([&]() {
		if (!hasValidSelection)
		{
			return;
		}
		auto it = fileEntries.find(selectedFileKey);
		if (it == fileEntries.end() || selectedLevelIndex >= it->second.mCollection.mLevels.size())
		{
			return;
		}
		const auto& level = it->second.mCollection.mLevels[selectedLevelIndex];
		const std::string boardKey = (it->second.mCollection.mLevels.size() > 1)
			? (selectedFileKey + "#" + std::to_string(selectedLevelIndex))
			: selectedFileKey;

		mInitBoardData[boardKey] = level.mBoard;
		mBoardData[boardKey] = level.mBoard;
		mBoardSolutions[boardKey] = level.mSolutions;
		if (std::find(mFilenames.begin(), mFilenames.end(), boardKey) == mFilenames.end())
		{
			mFilenames.emplace_back(boardKey);
		}
		mCurrentKey = boardKey;
		ChangeBoard();
		isChildWindowOpened = false;
		});
	child->add(applyButton);

	// キャンセルボタン
	auto exitButton = tgui::Button::create(mLoc->Get("BTN_CANCEL"));
	exitButton->setRenderer(mTheme->getRenderer("Button"));
	exitButton->setSize(120, 32);
	exitButton->setPosition(890, 518);
	exitButton->onPress([&]() {
		isChildWindowOpened = false;
		});
	child->add(exitButton);

	// 初期選択（一覧が空でなければ先頭を選択）
	if (!fileEntries.empty())
	{
		const std::string firstKey = fileEntries.begin()->first;
		fileListBox->setSelectedItemById(firstKey);
		selectedFileKey = firstKey;
		populateLevelsForFile(firstKey);
	}

	// ゲームループを停止
	mWindow->setActive(false);
	sf::Time ticksCount = mClock.getElapsedTime();

	while (isChildWindowOpened)
	{
		WaitForNextFrame(ticksCount);
		ticksCount = mClock.getElapsedTime();

		sf::Event event;
		while (mWindow->pollEvent(event))
		{
			if (event.type == sf::Event::Closed)
			{
				isChildWindowOpened = false;
				mGameState = GameState::EQuit;
			}
			else if (event.type == sf::Event::KeyPressed && event.key.code == sf::Keyboard::Escape)
			{
				isChildWindowOpened = false;
			}

			mGui->handleEvent(event);
		}

		mWindow->clear();
		DrawSprites();
		DrawUI();
		mGui->draw();
		mWindow->display();
	}

	mGui->remove(child);
	mWindow->setActive(true);
}

void Game::ChangeBoard()
{
	StopSolutionPlayback();
	mWasDeadlocked = false;
	mWasComplete = false;
	// ログをファイル出力してから全て消す
	// TODO 盤面が変わった後のものになってしまっている
	OutputLogs();
	mLogs.clear();

	// スナップショットを消去
	ClearSnapshots();

	// 盤面の同梱解手順を同期
	auto solIt = mBoardSolutions.find(mCurrentKey);
	if (solIt != mBoardSolutions.end())
	{
		mCurrentLevelSolutions = solIt->second;
	}
	else
	{
		mCurrentLevelSolutions.clear();
	}

	// 更新されたGameクラスのメンバ変数を参照して盤面を生成
	std::vector<std::string> lines{ mInitBoardData[mCurrentKey] };

	// 既存の状態をリセットしつつ、mGenParamsをロードした盤面に同期させる準備
	mGenParams.boardSize = sf::Vector2i{ static_cast<int>(lines.front().length()), static_cast<int>(lines.size()) };
	mGenParams.baggageNum = 0; // 下記のループでカウント

	mInitialPlayerPos = sf::Vector2i{ -1, -1 };
	mInitialBaggagePos.clear();
	mGoalPos.clear();
	mBoardState.clear();
	mStep = 0;
	delete  mHUDHelper;

	// 生成された盤面のキーは時刻を文字列に変換したものにする
	mGenParams.boardSize = sf::Vector2i{ static_cast<int>(lines.front().length()), static_cast<int>(lines.size()) };
	mBaggageLimit = GetBaggageNumLimit(mGenParams.boardSize);

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
					++mGenParams.baggageNum;
					mBoxesPos.push_back(sf::Vector2i(j, i));
					break;
				case '.':
					tmpstr += '.';
					mGoalPos.push_back(sf::Vector2i(j, i));
					break;
				case '*':
					tmpstr += '.';
					++mGenParams.baggageNum;
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
			WaitForNextFrame(ticksCount);

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

sf::Vector2f Game::GetTileSize() const
{
	sf::Vector2f result{ 0.0f, 0.0f };
	sf::Vector2f viewAreaSize = mBoardViewArea.second - mBoardViewArea.first;

	result = viewAreaSize / static_cast<float>(std::max(mGenParams.boardSize.x, mGenParams.boardSize.y));

	return result;
}

Baggage* Game::GetBaggageFromPos(const sf::Vector2i& pos)
{
	for (auto& baggage : mBaggages)
	{
		if (pos == baggage->GetBoardCoordinate())
		{
			return baggage;
		}
	}

	return nullptr;
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
	mStep = 0;

	// HUDHelperを削除
	delete mHUDHelper;
	mHUDHelper = nullptr;
}

void Game::ClearParameters()
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

std::vector<std::string> Game::GetBoardStateWithObjects() const
{
	// mBoardState (地形のみ) をコピーし、現在のプレイヤーと荷物の位置を書き込む
	std::vector<std::string> currentData = mBoardState;

	// 荷物
	for (const auto& baggage : mBaggages)
	{
		sf::Vector2i pos = baggage->GetBoardCoordinate();
		char& c = currentData[pos.y][pos.x];
		if (c == '.') c = '*'; // ゴール上
		else c = '$';          // 床上
	}

	// プレイヤー
	sf::Vector2i pPos = mPlayer->GetBoardCoordinate();
	char& c = currentData[pPos.y][pPos.x];
	if (c == '.') c = '+'; // ゴール上
	else c = '@';          // 床上

	return currentData;
}

void Game::SetMoveHighlights(const std::vector<sf::Vector2i>& tiles)
{
	if (mGameBoard)
	{
		std::cout << "Game::UpdateMoveHighlights called with " << tiles.size() << " tiles." << std::endl;
		mGameBoard->SetMoveHighlightedTiles(tiles);
		mPlayer->SetHighlightingState();
	}
}

void Game::ClearMoveHighlights()
{
	if (mGameBoard)
	{
		std::cout << "Game::ClearmoveHighlights called." << std::endl;
		mGameBoard->ClearMoveHighlights();
		mPlayer->SetIdleState();
	}
}

void Game::SetPushHighlights(const std::vector<sf::Vector2i>& tiles, Baggage* baggage)
{
	if (mGameBoard && baggage)
	{
		std::cout << "Game::UpdatePushHighlights called with " << tiles.size() << " tiles." << std::endl;
		SetBaggagesIdleState();
		mGameBoard->SetPushHighlightedTiles(tiles, baggage);
		baggage->SetHighlightingState();
	}
}

void Game::ClearPushHighlights()
{
	if (mGameBoard)
	{
		std::cout << "Game::ClearPushHighlights called." << std::endl;
		mGameBoard->ClearPushHighlights();
		for (auto baggage : mBaggages)
		{
			if (baggage->GetCurrentHighlightState() == Baggage::HighlightState::Highlighting)
			{
				baggage->SetIdleState();
				break;
			}
		}
	}
}

void Game::SetPushDirections(const std::vector<int>& indexes, class Baggage* baggage)
{
	if (mGameBoard && baggage)
	{
		std::cout << "Game::UpdatePushHighlights called with " << indexes.size() << " tiles." << std::endl;
		mGameBoard->SetPushDirections(indexes, baggage);
		baggage->SetDisplayDirectionsState();
	}
}

void Game::ClearPushDirections()
{
	if (mGameBoard)
	{
		std::cout << "Game::ClearPushDirections called." << std::endl;
		mGameBoard->ClearPushDirections();
		for (auto baggage : mBaggages)
		{
			if (baggage->GetCurrentHighlightState() == Baggage::HighlightState::DisplayDirection)
			{
				baggage->SetIdleState();
				break;
			}
		}
	}
}

void Game::SetBaggagesIdleState()
{
	for (auto baggage : mBaggages)
	{
		baggage->SetIdleState();
	}
}

bool Game::ExistsBaggagesHighlightingState()
{
	for (auto baggage : mBaggages)
	{
		if (baggage->GetCurrentHighlightState() == Baggage::HighlightState::Highlighting || baggage->GetCurrentHighlightState() == Baggage::HighlightState::DisplayDirection) return true;
	}

	return false;
}

std::vector<std::string> EditorScreen::GetTrimmedBoardData() const
{
	int minX = mEditorCanvasSize.x;
	int maxX = -1;
	int minY = mEditorCanvasSize.y;
	int maxY = -1;

	// 1. オブジェクトが存在する範囲(Bounding Box)を検出
	for (int y = 0; y < mEditorCanvasSize.y; ++y)
	{
		for (int x = 0; x < mEditorCanvasSize.x; ++x)
		{
			// 床(' ')以外の文字があれば、有効な範囲として記録
			if (mEditorBoardData[y][x] != ' ')
			{
				if (x < minX) minX = x;
				if (x > maxX) maxX = x;
				if (y < minY) minY = y;
				if (y > maxY) maxY = y;
			}
		}
	}

	// 盤面が空っぽ、または床しかない場合
	if (maxX == -1)
	{
		// 空のデータを返すか、最小限の床を返す
		return std::vector<std::string>();
	}

	// 2. 検出した範囲だけを切り出す
	std::vector<std::string> trimmedData;
	int width = maxX - minX + 1;

	for (int y = minY; y <= maxY; ++y)
	{
		// substr(開始位置, 長さ) で行を切り出す
		trimmedData.push_back(mEditorBoardData[y].substr(minX, width));
	}

	return trimmedData;
}

void Game::NotifyLanguageChanged()
{
	// 全てのUIスクリーンに対して言語変更イベントを発火させる
	for (auto ui : mUIStack)
	{
		ui->OnLanguageChanged();
	}
}