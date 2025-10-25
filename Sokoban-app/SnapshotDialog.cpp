#include "SnapshotDialog.h"
#include "Game.h"

#include "SnapshotScreen.h"

SnapshotDialog::SnapshotDialog(Game* game, sf::RenderWindow* window)
    : mGame(game),
    mState(IUIScreen::UIState::EActive),
    mGui(std::make_unique<tgui::Gui>(*window)), // 専用のGUIを作成
    mTheme(std::make_unique<tgui::Theme>("Assets/themes/Black.txt")), // テーマをロード
    parentManager(nullptr)
{
    // 1. ゲームを一時停止状態にする
    mGame->SetState(Game::GameState::EPaused);

    // 2. このUIスクリーンをGameのスタックにプッシュ
    mGame->PushUI(this);

    // 3. モーダル（他の操作をブロックする）の子ウィンドウを作成
    mChildWindow = tgui::ChildWindow::create();
    mChildWindow->setRenderer(mTheme->getRenderer("ChildWindow"));
    mChildWindow->setClientSize({ 400, 300 }); // 幅400, 高さ300
    mChildWindow->setTitle("Add Snapshot");
    // ウィンドウを中央に配置
    mChildWindow->setPosition("(&.width - width) / 2", "(&.height - height) / 2");
    mGui->add(mChildWindow);

    // 4. ウィジェットの配置
    // 子ウィンドウのサイズ
    sf::Vector2f childWindowSize{ mChildWindow->getSize().x, mChildWindow->getSize().y };
    // 名前ラベル
    auto nameLabel = tgui::Label::create("Name:");
    nameLabel->setRenderer(mTheme->getRenderer("Label"));
    nameLabel->setPosition(20, 20);
    nameLabel->setTextSize(16);
    mChildWindow->add(nameLabel);

    // 名前入力ボックス
    mNameBox = tgui::EditBox::create();
    mNameBox->setRenderer(mTheme->getRenderer("EditBox"));
    mNameBox->setSize(childWindowSize.x - 40, 30); // ウィンドウ幅 - 40px
    mNameBox->setPosition(20, 50);
    mNameBox->setTextSize(16);
    mNameBox->setDefaultText("Snapshot (" + mGame->GetDateTime() + ")"); // デフォルト名を設定
    //mNameBox->setFocused(true); // デフォルトではこのテキストボックスにフォーカス (うまく動作しない？)
    mChildWindow->add(mNameBox);

    // コメントラベル
    auto commentLabel = tgui::Label::create("Comment:");
    commentLabel->setRenderer(mTheme->getRenderer("Label"));
    commentLabel->setPosition(20, 90);
    commentLabel->setTextSize(16);
    mChildWindow->add(commentLabel);

    // コメント入力エリア
    mCommentBox = tgui::TextArea::create();
    mCommentBox->setRenderer(mTheme->getRenderer("TextArea"));
    mCommentBox->setSize(childWindowSize.x - 40, childWindowSize.y - 240); // 残りのスペースを埋める
    mCommentBox->setPosition(20, 120);
    mCommentBox->setTextSize(16);
    mCommentBox->setDefaultText("Optional: Add comments here...");
    mChildWindow->add(mCommentBox);

    // 保存ボタン
    auto saveButton = tgui::Button::create("Save");
    saveButton->setRenderer(mTheme->getRenderer("Button"));
    saveButton->setSize("(&.width - 50) / 2", 30); // 幅をウィンドウの約半分に
    saveButton->setPosition(20, "&.height - 80"); // ウィンドウ下部に配置
    saveButton->setTextSize(16);
    mChildWindow->add(saveButton);

    // キャンセルボタン
    auto cancelButton = tgui::Button::create("Cancel");
    cancelButton->setRenderer(mTheme->getRenderer("Button"));
    cancelButton->setSize("(&.width - 50) / 2", 30);
    cancelButton->setPosition("&.width - width - 20", "&.height - 80"); // 右側に配置
    cancelButton->setTextSize(16);
    mChildWindow->add(cancelButton);

    // --- 5. シグナル（イベント）の設定 ---

    // "Save" ボタンが押された時の処理
    saveButton->onPress([this]() {
        std::string name = mNameBox->getText().toStdString();
        std::string comment = mCommentBox->getText().toStdString();

        // Snapshotscreenから呼び出した場合はそちらにもスナップショットを追加する
        if (parentManager != nullptr)
        {
            parentManager->AddSnapshot(name, comment);
            parentManager->RemoveChildSnapshotDialog();
        }
        else
        {
            // Gameクラスの関数を呼び出してスナップショットを保存
            mGame->AddSnapshot(name, comment);
        }

        // このUIスクリーンを閉じる
        this->Close();
        });

    // "Cancel" ボタンが押された時の処理
    cancelButton->onPress([this]() {
        if (parentManager != nullptr) parentManager->RemoveChildSnapshotDialog();
        this->Close();
        });

    // ウィンドウの 'X' ボタンで閉じられた時の処理
    mChildWindow->onClose([this]() {
        if (parentManager != nullptr) parentManager->RemoveChildSnapshotDialog();
        this->Close();
        });
}

SnapshotDialog::~SnapshotDialog()
{
    // ゲームの一時停止を解除
    mGame->SetState(Game::GameState::EGamePlay);

    // mGuiはunique_ptrなので自動で解放される
}

void SnapshotDialog::Update(float deltaTime)
{
    // このUIは静的なので、Updateでの処理は不要
}

void SnapshotDialog::Draw(sf::RenderWindow* window)
{
    // このUIが持つGUIを描画
    mGui->draw();
}

void SnapshotDialog::ProcessInput(const sf::Event* event, const sf::Vector2i& mousePos)
{
    // このUIが持つGUIにイベントを渡す
    mGui->handleEvent(*event);
}

void SnapshotDialog::Close()
{
    // 状態をClosingに設定
    // Game::UpdateGame() のループがこのUIを安全に削除する
    mState = IUIScreen::UIState::EClosing;

    // ウィジェットをGUIから削除
    mGui->remove(mChildWindow);
}