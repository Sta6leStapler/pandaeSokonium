#pragma once

#include "SFML/Graphics.hpp"
#include "TGUI/TGUI.hpp"
#include "TGUI/Backend/SFML-Graphics.hpp"

#include <memory>
#include <vector>
#include <map>
#include "ParamStruct.h"

class Game;

// 盤面生成パラメータ設定用ダイアログクラス
class BoardConfigDialog
{
public:
    // 実行結果のステータス
    enum class Result { Apply, Cancel };

    /*
     * @param gui メインウィンドウのGUIオブジェクトへの参照
     * @param theme UIのスキン（Black.txt等）
     * @param currentParams 現在のGameクラスが保持しているパラメータ
     */
    BoardConfigDialog(class Game* game, tgui::Gui& gui, tgui::Theme& theme, const GenerationParameters& currentParams);

    /*
     * @brief ダイアログを表示し、ユーザーが決定またはキャンセルするまで待機する
     * @return 実行結果(Result)と、設定されたパラメータのペア
     * ダイアログが開いている間はGame側の更新を止める
     */
    std::pair<Result, GenerationParameters> run(sf::RenderWindow& window, sf::Clock& clock);

private:
    // UIパーツの初期構築
    void setupUI();
    // 複雑度スコアから「予想時間」ラベルを更新
    void updateComplexityEstimation();

    // スライダーとエディットボックスの値を同期させる
    void syncValues(const std::string& key, float value, bool fromSlider);

    class Game* mGame;
    tgui::Gui& mGui;
    tgui::Theme& mTheme;
    tgui::ChildWindow::Ptr mWindow;
    tgui::Label::Ptr mComplexityLabel;

    GenerationParameters mParams;
    std::vector<ParameterMeta> mMeta;

    // UI要素を動的に参照するためのマップ
    struct UIElementGroup {
        tgui::Slider::Ptr slider;
        tgui::EditBox::Ptr editBox;
    };
    std::map<std::string, UIElementGroup> mUIMap;

    bool mIsActive = true;
    Result mResult = Result::Cancel;
};