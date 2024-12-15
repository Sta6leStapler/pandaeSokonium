#include "PauseMenu.h"

PauseMenu::PauseMenu(class Game* game)
	: mGame(game)
	, mFont(new Font())
	, mTitle(new sf::Texture())
	, mBackground(new sf::Texture())
	, mButtonOn(new sf::Texture())
	, mButtonOff(new sf::Texture())
	, mButtons(std::vector<Button*>{})
	, mState(IUIScreen::UIState::EActive)
{
	mGame->PushUI(this);
	// ポーズメニューはゲームループに割り込む処理なので、
	// ポーズメニューの起動中のみインスタンスが存在するようにする
	mGame->SetState(Game::GameState::EPaused);

	// TODO テクスチャをGameクラスから取り込むように変える
	// フォントをロードし、画像を読み込んでテクスチャにしておく
	mFont->Load("Assets/fonts/arial.ttf");
	mButtonOn->loadFromFile("Assets/ButtonYellow.png");
	mButtonOff->loadFromFile("Assets/ButtonBlue.png");

	// ポーズ解除とゲーム終了のボタンを追加
	// TODO ボタンの座標を絶対座標から相対座標に変える
	// Note: SetTitle関数はmFont->Loadを呼び出した後
	// Note: ボタンの座標は枠の左上、座標系もウィンドウの左上が(0,0)
	SetTitle("PAUSED");
	mTitlePos = sf::Vector2f{ static_cast<float>(game->GetWindowSize().x - mTitle->getSize().x) / 2.0f, 100.0f};
	mNextButtonPos = sf::Vector2f{ static_cast<float>(game->GetWindowSize().x - mButtonOn->getSize().x) / 2.0f, 200.0f };
	mBGPos = sf::Vector2f{ 0.0f, 0.0f };
	AddButton("Resume", [this]() {
		Close();
		}); 
	AddButton("Quit", [this]() {
		mGame->SetState(Game::EQuit);
		}); 
}

PauseMenu::~PauseMenu()
{
	// ButtonクラスのメンバにFontクラスのメンバ変数が含まれるため、こちらを先に消去
	while (!mButtons.empty())
	{
		delete mButtons.back();
		mButtons.pop_back();
	}
	delete mFont;
	delete mTitle;
	delete mBackground;
	delete mButtonOn;
	delete mButtonOff;
	mGame->SetState(Game::GameState::EGamePlay);
}

void PauseMenu::Update(float deltaTime)
{

}

void PauseMenu::Draw(sf::RenderWindow* mWindow)
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
		sf::Texture* tex = b->GetHighlighted() ? mButtonOn : mButtonOff;
		DrawTexture(mWindow, tex, b->GetPosition());
		// ボタンのテキストを描画
		DrawTexture(mWindow, b->GetNameTex(), b->GetPosition());
	}
}

void PauseMenu::ProcessInput(const sf::Event* event, const sf::Vector2i& mousePos)
{
	// ボタンがあるか？
	if (!mButtons.empty())
	{
		// マウスの位置を取得
		sf::Vector2f mouse_pos{ static_cast<float>(mousePos.x), static_cast<float>(mousePos.y) };

		// ボタンの強調
		for (auto b : mButtons)
		{
			if (b->ContainsPoint(mouse_pos))
			{
				b->SetHighlighted(true);
			}
			else
			{
				b->SetHighlighted(false);
			}
		}
	}

	// Escキーが押されたらポーズメニューを閉じる
	switch (event->key.code)
	{
	case sf::Keyboard::Escape:
		//Close();
		break;
	case sf::Keyboard::Insert:
		Close();
		break;
	default:
		break;
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
				if (b->GetHighlighted())
				{
					b->OnClick();
					break;
				}
			}
		}
		break;
	default:
		break;
	}

}

void PauseMenu::Close()
{
	mState = IUIScreen::UIState::EClosing;
}

void PauseMenu::SetTitle(const std::string& text, const sf::Color& color /*= sf::Color::White*/, int pointSize /*= 40*/)
{
	mTitle = mFont->RenderText(text, color, pointSize);
}

void PauseMenu::AddButton(const std::string& name, std::function<void()> onClick)
{
	sf::Vector2f dims{ static_cast<float>(mButtonOn->getSize().x), static_cast<float>(mButtonOn->getSize().y) };
	Button* b = new Button(name, mFont, onClick, mNextButtonPos, dims);
	b->SetNameCenter(name, dims);
	mButtons.emplace_back(b);

	// TODO ボタンの座標を絶対座標から相対座標に変える
	// ボタンの高さ + 余白　の分だけ位置を下げる
	mNextButtonPos.y += mButtonOff->getSize().y + 20.0f;
}

void PauseMenu::DrawTexture(sf::RenderWindow* mWindow, class sf::Texture* texture, const sf::Vector2f& offset /*= sf::Vector2f{ 0.0f, 0.0f }*/, float scale /*= 1.0f*/)
{
	sf::Sprite spr;
	spr.setTexture(*texture);

	// スプライトのスケーリングと移動を行う
	spr.setScale(scale, scale);
	spr.setPosition(offset);

	mWindow->draw(spr);
}