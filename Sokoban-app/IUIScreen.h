#pragma once

#include "SFML/Window/Event.hpp"

// UI画面共通のインターフェースクラス
class IUIScreen {
public:
    virtual ~IUIScreen() = default;

    // UI画面の更新処理（時間経過に応じた動作）
    virtual void Update(float deltaTime) = 0;

    // UI画面の描画処理
    virtual void Draw(sf::RenderWindow* mWindow) = 0;

    // ユーザー入力の処理
    virtual void ProcessInput(const sf::Event* event, const sf::Vector2i& mousePos) = 0;

    // 状態がアクティブかクロージングか待機中か管理
    enum UIState { EActive, EClosing, EPending };

    // 状態をクロージングにする
    virtual void Close() = 0;

    // 状態を取得
    virtual UIState GetState() = 0;
};