#pragma once

#include "SFML/Graphics.hpp"
#include "TGUI/TGUI.hpp"
#include "TGUI/Backend/SFML-Graphics.hpp"

#include <map>
#include <string>

class Game;

class BoardParamManual {
public:
    // 解説ウィンドウを表示する
    static void display(Game* game, tgui::Gui& gui);

private:
    // ファイルからデータを読み込むヘルパー
    static std::string loadManualText(const std::string& filename);
};