#include "BoardParamManual.h"
#include "Game.h"
#include <fstream>
#include <sstream>

void BoardParamManual::display(Game* game, tgui::Gui& gui)
{
    auto theme = game->GetTheme();

    auto window = tgui::ChildWindow::create(game->GetLoc()->Get("BORAD_PARAM_MANUAL_TITLE"));
    window->setRenderer(theme.getRenderer("ChildWindow"));
    window->setSize(800, 600);
    window->setPosition("center", "center");

    // テキスト表示用のテキストボックス（スクロール可能・編集不可）
    auto textArea = tgui::TextArea::create();
    textArea->setRenderer(theme.getRenderer("TextArea"));
    textArea->setSize("100%", "100% - 60");
    textArea->setTextSize(18);
    textArea->setReadOnly(true);

    // ファイルからテキストを読み込んでセット
    std::string content = loadManualText(game->GetLoc()->Get("BORAD_PARAM_MANUAL_PATH"));
    textArea->setText(content);
    window->add(textArea);

    // 閉じるボタン
    auto closeBtn = tgui::Button::create(game->GetLoc()->Get("BTN_CLOSE"));
    closeBtn->setRenderer(game->GetTheme().getRenderer("Button"));
    closeBtn->setSize(100, 30);
    closeBtn->setPosition("100% - 120", "100% - 45");
    closeBtn->onPress([&gui, window] { gui.remove(window); });
    window->add(closeBtn);

    gui.add(window);
}

std::string BoardParamManual::loadManualText(const std::string& filename)
{
    std::ifstream file(filename);
    if (!file.is_open()) return "Error: Could not open manual file.";

    std::stringstream ss;
    ss << file.rdbuf();
    return ss.str();
}