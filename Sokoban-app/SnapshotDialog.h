#pragma once

#include "SFML/Graphics.hpp"
#include "TGUI/TGUI.hpp"
#include "TGUI/Backend/SFML-Graphics.hpp"
#include <memory>

#include "IUIScreen.h"

class Game;

class SnapshotDialog : public IUIScreen
{
public:
    // @brief コンストラクタ
    SnapshotDialog(Game* game, sf::RenderWindow* window);

    // デストラクタ
    // ゲームの一時停止を解除する
    virtual ~SnapshotDialog();

    // IUIScreenインターフェースのオーバーライド
    void Update(float deltaTime) override;
    void Draw(sf::RenderWindow* window) override;
    void ProcessInput(const sf::Event* event, const sf::Vector2i& mousePos) override;
    UIState GetState() override { return mState; }

    // このUIスクリーンを閉じる
    void Close();

    // SnapshotScreenのポインタをセット
    void SetParentManager(class SnapshotScreen* parent) { parentManager = parent; }

private:
    Game* mGame;
    std::unique_ptr<tgui::Gui> mGui;     // このダイアログ専用のGUIインスタンス
    std::unique_ptr<tgui::Theme> mTheme; // Black.txtテーマ
    UIState mState;                      // UIの状態 (Active, Closing)

    // TGUIウィジェット
    tgui::ChildWindow::Ptr mChildWindow; // すべてのウィジェットを格納する子ウィンドウ
    tgui::EditBox::Ptr mNameBox;         // スナップショット名を入力するボックス
    tgui::TextArea::Ptr mCommentBox;      // コメントを入力するエリア

    // SnapshotScreenから呼び出した場合、ポインタを保持しておく
    class SnapshotScreen* parentManager;
};