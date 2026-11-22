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
	mWindow->setSize(420, 420);
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

	const float windowW = mWindow->getSize().x;
	float yPos = 18.0f;

	// 再開ボタン
	auto resumeBtn = tgui::Button::create(mGame->GetLoc()->Get("BTN_RESUME"));
	resumeBtn->setRenderer(mGame->GetTheme().getRenderer("Button"));
	resumeBtn->setSize(220, 38);
	resumeBtn->setPosition((windowW - resumeBtn->getSize().x) / 2.0f, yPos);
	resumeBtn->onPress([this]() {
		mGame->PlaySound(SoundManager::SoundEffect::ButtonClick);
		Close();
		});
	mWindow->add(resumeBtn);

	yPos += 54.0f;
	// 言語切り替えコンボボックス
	auto langCombo = tgui::ComboBox::create();
	langCombo->setRenderer(mGame->GetTheme().getRenderer("ComboBox"));
	langCombo->setSize(220, 30);
	langCombo->setPosition((windowW - langCombo->getSize().x) / 2.0f, yPos);
	langCombo->setDefaultText(mGame->GetLoc()->Get("LBL_LANGUAGE"));
	langCombo->addItem(u8"日本語", "jp");
	langCombo->addItem("English", "en");

	langCombo->onItemSelect([this](const tgui::String& item, const tgui::String& id) {
		if (!mGame->GetLoc()->LoadLanguage(mGame->GetPaths().Asset("Assets/Languages/" + id.toStdString() + ".txt")))
			return;
		mGame->NotifyLanguageChanged(); // 全UIへ通知
		});
	mWindow->add(langCombo);

	// 効果音音量スライダー & ミュート切り替え
	yPos += 50.0f;
	const float currentVol = mGame->GetSoundManager() ? mGame->GetSoundManager()->GetVolume() : 70.0f;
	const bool currentMuted = mGame->GetSoundManager() ? mGame->GetSoundManager()->IsMuted() : false;

	auto volumeLabel = tgui::Label::create(mGame->GetLoc()->Get("LBL_SFX_VOLUME") + std::to_string(static_cast<int>(currentVol)) + "%");
	volumeLabel->setRenderer(mGame->GetTheme().getRenderer("Label"));
	volumeLabel->setPosition((windowW - 220.0f) / 2.0f, yPos);
	volumeLabel->setTextSize(15);
	mWindow->add(volumeLabel);

	yPos += 26.0f;
	auto volumeSlider = tgui::Slider::create(0.0f, 100.0f);
	volumeSlider->setRenderer(mGame->GetTheme().getRenderer("Slider"));
	volumeSlider->setSize(220, 18);
	volumeSlider->setPosition((windowW - 220.0f) / 2.0f, yPos);
	volumeSlider->setStep(1.0f);
	volumeSlider->setValue(currentVol);
	volumeSlider->onValueChange([this, volumeLabel](float value) {
		if (mGame->GetSoundManager())
		{
			mGame->GetSoundManager()->SetVolume(value);
		}
		volumeLabel->setText(mGame->GetLoc()->Get("LBL_SFX_VOLUME") + std::to_string(static_cast<int>(value)) + "%");
		mGame->PlaySound(SoundManager::SoundEffect::ButtonClick);
		});
	mWindow->add(volumeSlider);

	yPos += 32.0f;
	auto muteCheck = tgui::CheckBox::create(mGame->GetLoc()->Get("CHK_SFX_MUTE"));
	muteCheck->setRenderer(mGame->GetTheme().getRenderer("CheckBox"));
	muteCheck->setPosition((windowW - 220.0f) / 2.0f, yPos);
	muteCheck->setChecked(currentMuted);
	muteCheck->onChange([this](bool checked) {
		if (mGame->GetSoundManager())
		{
			mGame->GetSoundManager()->SetMuted(checked);
			if (!checked)
			{
				mGame->PlaySound(SoundManager::SoundEffect::ButtonClick);
			}
		}
		});
	mWindow->add(muteCheck);

	// サードパーティライセンス情報ボタン
	yPos += 44.0f;
	auto licenseBtn = tgui::Button::create(mGame->GetLoc()->Get("BTN_LICENSES"));
	licenseBtn->setRenderer(mGame->GetTheme().getRenderer("Button"));
	licenseBtn->setSize(220, 34);
	licenseBtn->setPosition((windowW - licenseBtn->getSize().x) / 2.0f, yPos);
	licenseBtn->onPress([this]() {
		mGame->PlaySound(SoundManager::SoundEffect::ButtonClick);
		mGame->DisplayLicenseWindow();
		});
	mWindow->add(licenseBtn);

	// 終了ボタン
	yPos += 50.0f;
	auto quitBtn = tgui::Button::create(mGame->GetLoc()->Get("BTN_QUIT"));
	quitBtn->setRenderer(mGame->GetTheme().getRenderer("Button"));
	quitBtn->setSize(220, 38);
	quitBtn->setPosition((windowW - quitBtn->getSize().x) / 2.0f, yPos);
	quitBtn->onPress([this]() { mGame->SetState(Game::GameState::EQuit); });
	mWindow->add(quitBtn);
}

void PauseMenu::Update(float deltaTime) {}
void PauseMenu::Draw(sf::RenderWindow* window) { mGui->draw(); }
void PauseMenu::ProcessInput(const sf::Event* event, const sf::Vector2i& mousePos) {
	mGui->handleEvent(*event);
}
void PauseMenu::Close() { mState = EClosing; }