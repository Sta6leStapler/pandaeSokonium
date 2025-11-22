#pragma once

#include "SFML/Graphics.hpp"
#include "TGUI/TGUI.hpp"
#include "TGUI/Backend/SFML-Graphics.hpp"
#include <memory>

#include "IUIScreen.h"

class Game;

class EditorSetupDialog : public IUIScreen
{
public:
    // コンストラクタ
    EditorSetupDialog(Game* game, sf::RenderWindow* window);

    // デストラクタ
    // ゲームの一時停止を解除する
    virtual ~EditorSetupDialog();

    // IUIScreenインターフェースのオーバーライド
    void Update(float deltaTime) override;
    void Draw(sf::RenderWindow* window) override;
    void ProcessInput(const sf::Event* event, const sf::Vector2i& mousePos) override;
    UIState GetState() override { return mState; }

    // このUIスクリーンを閉じる
    void Close();

private:
    Game* mGame;
    std::unique_ptr<tgui::Gui> mGui;     // このダイアログ専用のGUIインスタンス
    std::unique_ptr<tgui::Theme> mTheme; // Black.txtテーマ
    UIState mState;                      // UIの状態 (Active, Closing)

    // メンバ変数として保持しておくTGUIウィジェットは特になし

    // EditorScreenに渡すモードの種類
    // 0. 空の盤面
    // 1. 現在の盤面の初期状態
    // 2. 現在の盤面の現時点の状態
    int mMode;
};

