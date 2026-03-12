#include "EditorSetupDialog.h"
#include "Game.h"

EditorSetupDialog::EditorSetupDialog(Game* game, sf::RenderWindow* window)
    : mGame(game),
    mState(IUIScreen::UIState::EActive),
    mGui(std::make_unique<tgui::Gui>(*window)), // 専用のGUIを作成
    mTheme(std::make_unique<tgui::Theme>("Assets/themes/Black.txt")), // テーマをロード
	mMode(-1)
{
    // 1. ゲームを一時停止状態にする
    mGame->SetState(Game::GameState::EPaused);

    // 2. このUIスクリーンをGameのスタックにプッシュ
    mGame->PushUI(this);

	// フォントを設定
	mGui->setFont(mGame->GetFontTGUI());

	// 起動モード選択ダイアログを表示
	auto msgBox = tgui::MessageBox::create();
	msgBox->setRenderer(mTheme->getRenderer("MessageBox"));
	msgBox->moveToFront();
	msgBox->setTitle(mGame->GetLoc()->Get("EDITOR_SETUP_LBL_TITLE"));
	msgBox->setText(mGame->GetLoc()->Get("EDITOR_SETUP_LBL_TEXT"));
	msgBox->addButton(mGame->GetLoc()->Get("EDITOR_SETUP_BTN_NEW"));
	msgBox->addButton(mGame->GetLoc()->Get("EDITOR_SETUP_BTN_CURRENT_INITIAL"));
	msgBox->addButton(mGame->GetLoc()->Get("EDITOR_SETUP_BTN_CURRENT_STATE"));
	msgBox->addButton(mGame->GetLoc()->Get("BTN_CANCEL"));
	msgBox->setPosition((mGame->GetWindowSize().x - msgBox->getFullSize().x) / 2.0f, (mGame->GetWindowSize().y - msgBox->getFullSize().y) / 2.0f);

	msgBox->onButtonPress([this, msgBox](const tgui::String& button) {
		if (button == mGame->GetLoc()->Get("BTN_CANCEL"))
		{
			Close();
			return;
		}

		// 選択に応じてロードモードを決定
		if (button == mGame->GetLoc()->Get("EDITOR_SETUP_BTN_NEW")) mMode = 0;
		else if (button == mGame->GetLoc()->Get("EDITOR_SETUP_BTN_CURRENT_INITIAL")) mMode = 1;
		else if (button == mGame->GetLoc()->Get("EDITOR_SETUP_BTN_CURRENT_STATE")) mMode = 2;

		// Gameクラスからエディット画面を起動
		mGame->DisplayEditorScreen(mMode);
		Close();
		});

	msgBox->onClose([this, msgBox]() {
		Close();
		});

	mGui->add(msgBox);
}

EditorSetupDialog::~EditorSetupDialog()
{
    // ゲームの一時停止を解除
    mGame->SetState(Game::GameState::EGamePlay);

    // mGuiはunique_ptrなので自動で解放される
}

void EditorSetupDialog::Update(float deltaTime)
{
    // このUIは静的なので、Updateでの処理は不要
}

void EditorSetupDialog::Draw(sf::RenderWindow* window)
{
    // このUIが持つGUIを描画
    mGui->draw();
}

void EditorSetupDialog::ProcessInput(const sf::Event* event, const sf::Vector2i& mousePos)
{
    // このUIが持つGUIにイベントを渡す
    mGui->handleEvent(*event);
}

void EditorSetupDialog::Close()
{
    // 状態をClosingに設定
    // Game::UpdateGame() のループがこのUIを安全に削除する
    mState = IUIScreen::UIState::EClosing;
}