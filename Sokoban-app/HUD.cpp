#include "HUD.h"
#include <iomanip>
#include <sstream>

HUD::HUD(class Game* game)
	: mGame(game)
	, mFont(new Font())
	, mTitle(new sf::Texture())
	, mBackground(new sf::Texture())
	, mButtonOn(new sf::Texture())
	, mButtonOff(new sf::Texture())
	, mUndoTex(new sf::Texture())
	, mRedoTex(new sf::Texture())
	, mResetTex(new sf::Texture())
	, mRedoAllTex(new sf::Texture())
	, mSaveBoardTex(new sf::Texture())
	, mSaveLogTex(new sf::Texture())
	, mLoadBoardTex(new sf::Texture())
	, mGenerateBoardTex(new sf::Texture())
	, mHelpTex(new sf::Texture())
	, mButtons(std::vector<std::pair<Button*, sf::Texture*>>{})
	, mState(IUIScreen::UIState::EActive)
{
	mGame->PushUI(this);

	// TODO テクスチャをGameクラスから取り込むように変える
	// フォントをロードし、画像を読み込んでテクスチャにしておく
	mFont->Load("Assets/fonts/arial.ttf");
	mButtonOn->loadFromFile("Assets/ButtonYellow.png");
	mButtonOff->loadFromFile("Assets/ButtonBlue.png");
	mUndoTex->loadFromFile("Assets/Undo.png");
	mRedoTex->loadFromFile("Assets/Redo.png");
	mResetTex->loadFromFile("Assets/Reset.png");
	mRedoAllTex->loadFromFile("Assets/RedoAll.png");
	mSaveBoardTex->loadFromFile("Assets/SaveBoard.png");
	mSaveLogTex->loadFromFile("Assets/SaveLog.png");

	// 各種ボタンを追加
	// TODO ボタンの座標を絶対座標から相対座標に変える
	// Note: SetTitle関数はmFont->Loadを呼び出した後
	SetTitle(" ");
	mTitlePos = sf::Vector2f{ 0.0f, 0.0f };
	mBGPos = sf::Vector2f{ 0.0f, 0.0f };
	// Undoボタン
	mUndoButtonPos = sf::Vector2f{ 20.0f, 40.0f };
	AddButton("Undo", mUndoButtonPos, [this]() {
		mGame->CallUndo();
		},
		mUndoTex);
	// Redoボタン
	mRedoButtonPos = sf::Vector2f{ mUndoButtonPos.x + static_cast<float>(mButtonOn->getSize().x) + 20.0f, 40.0f };
	AddButton("Redo", mRedoButtonPos, [this]() {
		mGame->CallRedo();
		},
		mRedoTex);
	// Resetボタン
	mResetButtonPos = sf::Vector2f{ 20.0f, mUndoButtonPos.y + static_cast<float>(mButtonOn->getSize().y) + 20.0f };
	AddButton("Reset", mResetButtonPos, [this]() {
		mGame->CallReset();
		},
		mResetTex);
	// Redo Allボタン
	mRedoAllButtonPos = sf::Vector2f{ mResetButtonPos.x + static_cast<float>(mButtonOn->getSize().x) + 20.0f, mRedoButtonPos.y + static_cast<float>(mButtonOn->getSize().y) + 20.0f };
	AddButton("Redo All", mRedoAllButtonPos, [this]() {
		mGame->CallRedoAll();
		},
		mRedoAllTex);
	// Save Boardボタン
	mSaveBoardButtonPos = sf::Vector2f{ 20.0f, mResetButtonPos.y + static_cast<float>(mButtonOn->getSize().y) + 20.0f };
	AddButton("Save Board", mSaveBoardButtonPos, [this]() {
		mGame->CallSave();
		},
		mSaveBoardTex,
		26);
	// Save Logボタン
	mSaveLogButtonPos = sf::Vector2f{ mSaveBoardButtonPos.x + static_cast<float>(mButtonOn->getSize().x) + 20.0f, mRedoAllButtonPos.y + static_cast<float>(mButtonOn->getSize().y) + 20.0f };
	AddButton("Save Log", mSaveLogButtonPos, [this]() {
		mGame->OutputLogs();
		},
		mSaveLogTex,
		26);
	// Load Boardボタン
	mLoadBoardButtonPos = sf::Vector2f{ mRedoButtonPos.x + static_cast<float>(mButtonOn->getSize().x) + 20.0f, 40.0f };
	AddButton("Load Board", mLoadBoardButtonPos, [this]() {
		// TODO 作り直し
		},
		mLoadBoardTex,
		26);
	// Generate Boardボタン
	mGenerateBoardButtonPos = sf::Vector2f{ mRedoAllButtonPos.x + static_cast<float>(mButtonOn->getSize().x) + 20.0f, mLoadBoardButtonPos.y + static_cast<float>(mButtonOn->getSize().y) + 20.0f };
	AddButton("Generate Board", mGenerateBoardButtonPos, [this]() {
		mGame->CallReload();
		},
		mGenerateBoardTex,
		24);
	// Helpボタン
	mHelpButtonPos = sf::Vector2f{ mSaveLogButtonPos.x + static_cast<float>(mButtonOn->getSize().x) + 20.0f, mGenerateBoardButtonPos.y + static_cast<float>(mButtonOn->getSize().y) + 20.0f };
	AddButton("Help", mHelpButtonPos, [this]() {
		mGame->DisplayHelpWindow();
		},
		mHelpTex);

	// 各種情報のテキストを表示する矩形
	mTextInfoBox = new sf::RectangleShape(sf::Vector2f(mGame->GetWindowSize().x - mGame->GetWindowSize().y - 40, mGame->GetWindowSize().y - (mSaveBoardButtonPos.y + mButtonOn->getSize().y) - 40 ));
	mTextInfoBox->setPosition(20, mSaveBoardButtonPos.y + mButtonOn->getSize().y + 20); // 左側に配置
	mTextInfoBox->setFillColor(sf::Color(200, 200, 200)); // 薄いグレー

	// テキストの初期設定（情報エリアの座標を基準に相対座標で設定）
	mMoveCountText = new sf::Text();
	AddText(mMoveCountText, "Moves : " + std::to_string(mGame->GetStep()), mTextInfoBox->getPosition() + sf::Vector2f(10, 10));
	
	mTimeText = new sf::Text();
	AddText(mTimeText, "Time : 0:00", mTextInfoBox->getPosition() + sf::Vector2f(10, 40));

	mStateText = new sf::Text();
	AddText(mStateText, "Status : ", mTextInfoBox->getPosition() + sf::Vector2f(10, 70));
	
	mMovableBaggagesText = new sf::Text();
	AddText(mMovableBaggagesText, "Movable bagggages : ", mTextInfoBox->getPosition() + sf::Vector2f(10, 100));

	mGoaledBaggagesText = new sf::Text();
	AddText(mGoaledBaggagesText, "Goaled baggages : ", mTextInfoBox->getPosition() + sf::Vector2f(10, 130));

	mDeadlockedBaggagesText = new sf::Text();
	AddText(mDeadlockedBaggagesText, "Deadlocked baggages : ", mTextInfoBox->getPosition() + sf::Vector2f(10, 160));
}

HUD::~HUD()
{
	// ButtonクラスのメンバにFontクラスのメンバ変数が含まれるため、こちらを先に消去
	while (!mButtons.empty())
	{
		delete mButtons.back().first;
		delete mButtons.back().second;
		mButtons.pop_back();
	}
	delete mFont;
	delete mButtonOn;
	delete mButtonOff;
	while (!mTextInfoes.empty())
	{
		delete mTextInfoes.back();
		mTextInfoes.pop_back();
	}
	mGame->SetState(Game::GameState::EGamePlay);
}

void HUD::Update(float deltaTime)
{
	// テキスト表示する各種情報を更新
	mMoveCountText->setString("Moves : " + std::to_string(mGame->GetStep()));
	long time = static_cast<long>(mGame->GetSecTime());
	std::stringstream m, s;
	m << std::setw(2) << std::setfill('0') << time / 60;
	s << std::setw(2) << std::setfill('0') << time % 60;
	mTimeText->setString("Time : " + m.str() + ":" + s.str());

	std::unordered_map<int, sf::Vector2i> deadlockedBaggages{ mGame->GetHUDHelper()->GetDeadlockedBaggages() },
		goaledBaggages{ mGame->GetHUDHelper()->GetGoaledBaggages() };
	std::vector<sf::Vector2i> movableBaggages{ mGame->GetHUDHelper()->GetCandidates() };
	if (!deadlockedBaggages.empty())
	{
		mStateText->setString("Status : Deadlocked!");
		mStateText->setFillColor(sf::Color::Red);
	}
	else
	{
		mStateText->setString("Status : ");
		mStateText->setFillColor(sf::Color::Black);
	}

	std::string tmpStr{};
	for (const auto& baggage : goaledBaggages)
	{
		tmpStr += "( " + std::to_string(baggage.second.x) + ", " + std::to_string(baggage.second.y) + ") ";
	}
	mGoaledBaggagesText->setString("Goaled baggages : " + tmpStr);
	tmpStr = "";

	for (const auto& baggage : movableBaggages)
	{
		tmpStr += "( " + std::to_string(baggage.x) + ", " + std::to_string(baggage.y) + ") ";
	}
	mMovableBaggagesText->setString("Movable baggages : " + tmpStr);
	tmpStr = "";

	for (const auto& baggage : deadlockedBaggages)
	{
		tmpStr += "( " + std::to_string(baggage.second.x) + ", " + std::to_string(baggage.second.y) + ") ";
	}
	mDeadlockedBaggagesText->setString("Deadlocked baggages : " + tmpStr);
	tmpStr = "";
}

void HUD::Draw(sf::RenderWindow* mWindow)
{
	// 背景を追加する場合はそれを描画
	if (mBackground)
	{
		DrawTexture(mWindow, mBackground, mBGPos);
	}
	// タイトルが存在する場合はそれを描画
	if (mTitle)
	{
		DrawTexture(mWindow, mTitle, mTitlePos);
	}
	// ボタン本体の描画
	for (auto b : mButtons)
	{
		// ボタンの背景を描画
		sf::Texture* tex = b.first->GetHighlighted() ? mButtonOn : mButtonOff;
		DrawTexture(mWindow, tex, b.first->GetPosition());
		// ボタンのアイコンの描画
		DrawTexture(mWindow, b.second, sf::Vector2f{ b.first->GetPosition().x + static_cast<float>(tex->getSize().x - b.second->getSize().x), b.first->GetPosition().y});
		// ボタンのテキストを描画
		DrawTexture(mWindow, b.first->GetNameTex(), b.first->GetPosition());
	}

	// テキストウィンドウの表示
	mWindow->draw(*mTextInfoBox);
	for (auto t : mTextInfoes)
	{
		mWindow->draw(*t);
	}
}

void HUD::ProcessInput(const sf::Event* event, const sf::Vector2i& mousePos)
{
	// ボタンがあるか？
	if (!mButtons.empty())
	{
		// マウスの位置を取得
		sf::Vector2f mouse_pos{ static_cast<float>(mousePos.x), static_cast<float>(mousePos.y) };

		// ボタンの強調
		for (auto b : mButtons)
		{
			if (b.first->ContainsPoint(mouse_pos))
			{
				b.first->SetHighlighted(true);
			}
			else
			{
				b.first->SetHighlighted(false);
			}
		}
	}

	// ボタン上でマウス左ボタンが押されたら、そのボタンの処理を行う
	switch (event->mouseButton.button)
	{
	case sf::Mouse::Left:
		// アクティブなボタンがあるか調べる
		if (!mButtons.empty())
		{
			for (auto b : mButtons)
			{
				if (b.first->GetHighlighted())
				{
					b.first->OnClick();
					break;
				}
			}
		}
		break;
	default:
		break;
	}
}

void HUD::SetTitle(const std::string& text, const sf::Color& color /*= sf::Color::White*/, int pointSize /*= 40*/)
{
	mTitle = mFont->RenderText(text, color, pointSize);
}

void HUD::AddButton(const std::string& name, const sf::Vector2f& button_pos, std::function<void()> onClick, sf::Texture* iconTexture, int pointSize, const sf::Color& color)
{
	// TODO ボタンの座標を絶対座標から相対座標に変える
	sf::Vector2f dims{ static_cast<float>(mButtonOn->getSize().x), static_cast<float>(mButtonOn->getSize().y) };
	Button* b = new Button(name, mFont, onClick, button_pos, dims);
	b->SetNameCenter(name, dims, color, pointSize);
	mButtons.emplace_back(std::pair<Button*, sf::Texture*>{ b, iconTexture });
}

void HUD::AddText(sf::Text* text, const std::string& textStr, const sf::Vector2f& pos, const unsigned int& charSize, const sf::Color& color)
{
	text->setFont(*mFont->GetFont());
	text->setString(textStr);
	text->setCharacterSize(charSize);
	text->setPosition(pos);
	text->setFillColor(color);
	mTextInfoes.emplace_back(text);
}

void HUD::DrawTexture(sf::RenderWindow* mWindow, class sf::Texture* texture, const sf::Vector2f& offset /*= sf::Vector2f{ 0.0f, 0.0f }*/, float scale /*= 1.0f*/)
{
	sf::Sprite spr;
	spr.setTexture(*texture);

	// スプライトのスケーリングと移動を行う
	spr.setScale(scale, scale);
	spr.setPosition(offset);

	mWindow->draw(spr);
}