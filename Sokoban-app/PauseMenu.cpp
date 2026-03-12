#include "PauseMenu.h"
#include "Game.h"
#include "LocalizationManager.h"

PauseMenu::PauseMenu(Game* game, sf::RenderWindow* window)
	: mGame(game)
	, mGui(std::make_unique<tgui::Gui>(*window)) // Gameからウィンドウを取得
	, mState(EActive)
{
	mGame->PushUI(this);
	mGame->SetState(Game::GameState::EPaused);

	mGui->setFont(mGame->GetFontTGUI());

	mWindow = tgui::ChildWindow::create();
	mWindow->setRenderer(mGame->GetTheme().getRenderer("ChildWindow"));
	mWindow->setSize(400, 300);
	mWindow->setPosition((mGame->GetWindowSize().x - mWindow->getSize().x) / 2.0f, (mGame->GetWindowSize().y - mWindow->getSize().y) / 2.0f);
	mWindow->onClose([this]() { Close(); });
	mGui->add(mWindow);

	OnLanguageChanged(); // 初期テキスト設定
}

PauseMenu::~PauseMenu() {
	mGame->SetState(Game::GameState::EGamePlay);
}

void PauseMenu::OnLanguageChanged()
{
	mWindow->removeAllWidgets(); // 一旦クリアして再構築（または保持したメンバを更新）
	mWindow->setTitle(mGame->GetLoc()->Get("MENU_PAUSED"));

	float yPos = 20;
	// 再開ボタン
	auto resumeBtn = tgui::Button::create(mGame->GetLoc()->Get("BTN_RESUME"));
	resumeBtn->setRenderer(mGame->GetTheme().getRenderer("Button"));
	resumeBtn->setSize(200, 40);
	resumeBtn->setPosition((mWindow->getSize().x - resumeBtn->getSize().x) / 2.0f, yPos);
	resumeBtn->onPress([this]() { Close(); });
	mWindow->add(resumeBtn);

	yPos += 60;
	// 言語切り替えコンボボックス
	auto langCombo = tgui::ComboBox::create();
	langCombo->setRenderer(mGame->GetTheme().getRenderer("ComboBox"));
	langCombo->setSize(200, 30);
	langCombo->setPosition((mWindow->getSize().x - langCombo->getSize().x) / 2.0f, yPos);
	langCombo->setDefaultText(mGame->GetLoc()->Get("LBL_LANGUAGE"));
	langCombo->addItem(u8"日本語", "jp");
	langCombo->addItem("English", "en");

	langCombo->onItemSelect([this](const tgui::String& item, const tgui::String& id) {
		mGame->GetLoc()->LoadLanguage("Assets/Languages/" + id.toStdString() + ".txt");
		mGame->NotifyLanguageChanged(); // 全UIへ通知
		});
	mWindow->add(langCombo);

	yPos += 100;
	// 終了ボタン
	auto quitBtn = tgui::Button::create(mGame->GetLoc()->Get("BTN_QUIT"));
	quitBtn->setRenderer(mGame->GetTheme().getRenderer("Button"));
	quitBtn->setSize(200, 40);
	quitBtn->setPosition((mWindow->getSize().x - quitBtn->getSize().x) / 2.0f, yPos);
	quitBtn->onPress([this]() { mGame->SetState(Game::GameState::EQuit); });
	mWindow->add(quitBtn);
}

void PauseMenu::Update(float deltaTime) {}
void PauseMenu::Draw(sf::RenderWindow* window) { mGui->draw(); }
void PauseMenu::ProcessInput(const sf::Event* event, const sf::Vector2i& mousePos) {
	mGui->handleEvent(*event);
}
void PauseMenu::Close() { mState = EClosing; }