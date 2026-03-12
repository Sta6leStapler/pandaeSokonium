#pragma once

#include "SFML/Graphics.hpp"
#include "TGUI/TGUI.hpp"
#include "TGUI/Backend/SFML-Graphics.hpp"

#include "IUIScreen.h"

class PauseMenu : public IUIScreen
{
public:
    PauseMenu(class Game* game, class sf::RenderWindow* window);
    ~PauseMenu();

    void Update(float deltaTime) override;
    void Draw(sf::RenderWindow* window) override;
    void ProcessInput(const sf::Event* event, const sf::Vector2i& mousePos) override;
    void Close() override;
    UIState GetState() override { return mState; }
    void OnLanguageChanged() override;

private:
    class Game* mGame;
    std::unique_ptr<tgui::Gui> mGui;
    tgui::ChildWindow::Ptr mWindow;
    UIState mState;
};