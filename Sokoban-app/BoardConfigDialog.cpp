#include "BoardConfigDialog.h"

#include <iomanip>
#include <sstream>

#include "Game.h"
#include "BoardParamManual.h"

BoardConfigDialog::BoardConfigDialog(Game* game, tgui::Gui& gui, tgui::Theme& theme, const GenerationParameters& currentParams)
    : mGame(game)
    , mGui(gui)
    , mTheme(theme)
    , mParams(currentParams)
{
    // メタデータの定義
    mMeta = {
        {mGame->GetLoc()->Get("GENERATE_BOARD_META_TITLE_WIDTH"), mGame->GetLoc()->Get("GENERATE_BOARD_META_TEXT_WIDTH"), mGame->GetLoc()->Get("GENERATE_BOARD_META_POP_WIDTH"), 5, 48, true},
        {mGame->GetLoc()->Get("GENERATE_BOARD_META_TITLE_HEIGHT"), mGame->GetLoc()->Get("GENERATE_BOARD_META_TEXT_HEIGHT"), mGame->GetLoc()->Get("GENERATE_BOARD_META_POP_HEIGHT"), 5, 48, true},
        {mGame->GetLoc()->Get("GENERATE_BOARD_META_TITLE_BAGGAGE"), mGame->GetLoc()->Get("GENERATE_BOARD_META_TEXT_BAGGAGE"), mGame->GetLoc()->Get("GENERATE_BOARD_META_POP_BAGGAGE"), 1, 50, true},
        {mGame->GetLoc()->Get("GENERATE_BOARD_META_TITLE_RESET"), mGame->GetLoc()->Get("GENERATE_BOARD_META_TEXT_RESET"), mGame->GetLoc()->Get("GENERATE_BOARD_META_POP_RESET"), 0, 32, true},
        {mGame->GetLoc()->Get("GENERATE_BOARD_META_TITLE_TRANSPORT"), mGame->GetLoc()->Get("GENERATE_BOARD_META_TEXT_TRANSPORT"), mGame->GetLoc()->Get("GENERATE_BOARD_META_POP_TRANSPORT"), 1, 256, true},
        {mGame->GetLoc()->Get("GENERATE_BOARD_META_TITLE_WALL"), mGame->GetLoc()->Get("GENERATE_BOARD_META_TEXT_WALL"), mGame->GetLoc()->Get("GENERATE_BOARD_META_POP_WALL"), 0.0, 0.5, false},
        {mGame->GetLoc()->Get("GENERATE_BOARD_META_TITLE_VISITED"), mGame->GetLoc()->Get("GENERATE_BOARD_META_TEXT_VISITED"), mGame->GetLoc()->Get("GENERATE_BOARD_META_POP_VISITED"), 0.0, 1.0, false},
        {mGame->GetLoc()->Get("GENERATE_BOARD_META_TITLE_EVAL"), mGame->GetLoc()->Get("GENERATE_BOARD_META_TEXT_EVAL"), mGame->GetLoc()->Get("GENERATE_BOARD_META_POP_EVAL"), 0, 6, true}
    };

    setupUI();
}

void BoardConfigDialog::setupUI()
{
    // 子ウィンドウの作成
    mWindow = tgui::ChildWindow::create(mGame->GetLoc()->Get("GENERATE_BOARD_WINDOW_TITLE"));
    mWindow->setRenderer(mTheme.getRenderer("ChildWindow"));
    mWindow->setSize(800, 560);
    // 画面中央に配置
    mWindow->setPosition("420", "80");
    // ウィンドウを直接閉じた場合
    mWindow->onClose([this]() { mResult = Result::Cancel; mIsActive = false; });

    float yPos = 20;
    for (const auto& meta : mMeta)
    {
        // 1. ラベルとツールチップの作成
        auto label = tgui::Label::create(meta.name + " :");
        label->setRenderer(mTheme.getRenderer("Label"));
        label->setPosition(30, yPos);
        label->setTextSize(18);

        // ツールチップの設定
        // ホバー時に補足説明を出す
        auto toolTip = tgui::Label::create(meta.description);
        toolTip->setRenderer(mTheme.getRenderer("ToolTip"));
        //toolTip->setPadding(5);
        label->setToolTip(toolTip);
        mWindow->add(label);

        // 2. エディットボックス（数値直接入力用）
        auto editBox = tgui::EditBox::create();
        editBox->setRenderer(mTheme.getRenderer("EditBox"));
        editBox->setSize(80, 25);
        editBox->setPosition(200, yPos);

        // 3. スライダー（マウス操作用）
        auto slider = tgui::Slider::create();
        slider->setRenderer(mTheme.getRenderer("Slider"));
        slider->setPosition(300, yPos + 4);
        slider->setSize(400, 18);
        slider->setMinimum(meta.min);
        slider->setMaximum(meta.max);

        // パラメータの種類によってステップ値を切り替える
        if (meta.isInteger)
        {
            slider->setStep(1.0f);
        }
        else
        {
            slider->setStep(0.01f); // 実数値は細かく調整可能にする
        }

        // 初期値の反映
        float initialValue = 0.0f;
        if (meta.key == "width") initialValue = static_cast<float>(mParams.boardSize.x);
        else if (meta.key == "height") initialValue = static_cast<float>(mParams.boardSize.y);
        else if (meta.key == "baggage") initialValue = static_cast<float>(mParams.baggageNum);
        else if (meta.key == "reset") initialValue = static_cast<float>(mParams.resetCount);
        else if (meta.key == "transport") initialValue = static_cast<float>(mParams.transportCount);
        else if (meta.key == "wall") initialValue = mParams.wallRate;
        else if (meta.key == "visited") initialValue = mParams.visitedRate;
        else if (meta.key == "eval") initialValue = (float)mParams.evalFuncIndex;

        slider->setValue(initialValue);
        editBox->setText(meta.isInteger ? std::to_string((int)initialValue) : std::to_string(initialValue).substr(0, 4));

        // イベントのバインド
        // syncValuesという共通関数に飛ばし、スライダー操作時とキーボード入力時でロジックが重複するのを避ける
        slider->onValueChange([this, meta](float val) { syncValues(meta.key, val, true); });
        editBox->onReturnKeyPress([this, meta, editBox]() {
            try
            {
                float val = std::stof(editBox->getText().toStdString());
                syncValues(meta.key, val, false);
            }
            catch (...) {}
            });

        mWindow->add(editBox);
        mWindow->add(slider);

        // マップに保持（後で値を読み出すため）
        mUIMap[meta.key] = { slider, editBox };

        yPos += 45;
    }

    // 計算負荷の表示ラベル
    mComplexityLabel = tgui::Label::create("");
    mComplexityLabel->setPosition(30, yPos + 20);
    mComplexityLabel->setTextSize(20);
    mWindow->add(mComplexityLabel);
    updateComplexityEstimation();

    // 決定ボタン
    auto applyBtn = tgui::Button::create(mGame->GetLoc()->Get("GENERATE_BOARD_WINDOW_BTN_APPLY_GENERATE"));
    applyBtn->setRenderer(mTheme.getRenderer("Button"));
    applyBtn->setSize(160, 30);
    applyBtn->setPosition("100% - 200", "100% - 50");
    applyBtn->onPress([this]() { mResult = Result::Apply; mIsActive = false; });
    mWindow->add(applyBtn);

    // キャンセルボタン
    auto cancelBtn = tgui::Button::create(mGame->GetLoc()->Get("BTN_CANCEL"));
    cancelBtn->setRenderer(mTheme.getRenderer("Button"));
    cancelBtn->setSize(100, 30);
    cancelBtn->setPosition("100% - 320", "100% - 50");
    cancelBtn->onPress([this]() { mResult = Result::Cancel; mIsActive = false; });
    mWindow->add(cancelBtn);

    // パラメータのマニュアルを表示するボタン
    auto helpBtn = tgui::Button::create("?");
    helpBtn->setRenderer(mTheme.getRenderer("Button"));
    helpBtn->setSize(30, 30);
    helpBtn->setPosition("100% - 370", "100% - 50"); // Apply/Cancelの左側
    helpBtn->onPress([this] {
        // マニュアルウィンドウを重畳表示
        BoardParamManual::display(mGame, mGui);
        });
    mWindow->add(helpBtn);

    mGui.add(mWindow);
}

void BoardConfigDialog::syncValues(const std::string& key, float value, bool fromSlider)
{
    // 1. パラメータ構造体の更新
    if (key == "width") mParams.boardSize.x = (int)value;
    else if (key == "height") mParams.boardSize.y = (int)value;
    else if (key == "baggage") mParams.baggageNum = (int)value;
    else if (key == "reset") mParams.resetCount = (int)value;
    else if (key == "transport") mParams.transportCount = (int)value;
    else if (key == "wall") mParams.wallRate = value;
    else if (key == "visited") mParams.visitedRate = value;
    else if (key == "eval") mParams.evalFuncIndex = (int)value;

    // 2. UIの同期
    // スライダーが動いたらエディットボックスを書き換え、その逆も行う。
    
    // 対象のメタ情報を探す
    auto it = std::find_if(mMeta.begin(), mMeta.end(), [&](const ParameterMeta& m) { return m.key == key; });

    auto& ui = mUIMap[key];
    if (fromSlider) {
        if (it != mMeta.end() && it->isInteger) {
            // 自然数の場合は小数点以下を表示しない
            ui.editBox->setText(std::to_string(static_cast<int>(value))); //
        }
        else {
            // 実数の場合は小数点以下を表示
            std::stringstream ss;
            ss << std::fixed << std::setprecision(2) << value;
            ui.editBox->setText(ss.str()); //
        }
    }
    else {
        ui.slider->setValue(value);
    }

    // 3. 負荷予測の更新
    updateComplexityEstimation();
}

void BoardConfigDialog::updateComplexityEstimation()
{
    float score = mParams.calculateComplexityScore();
    std::string level;
    sf::Color color;

    // スコアに応じた評価（数値は実行環境に合わせて要調整）
    // 現在は体感による適当な値を入れている
    if (score < 1e6) { level = mGame->GetLoc()->Get("GENERATE_BOARD_LBL_VERY_FAST"); color = sf::Color::Cyan; }
    else if (score < 1e8) { level = mGame->GetLoc()->Get("GENERATE_BOARD_LBL_FAST"); color = sf::Color::Green; }
    else if (score < 1e11) { level = mGame->GetLoc()->Get("GENERATE_BOARD_LBL_MODERATE"); color = sf::Color::Yellow; }
    else { level = mGame->GetLoc()->Get("GENERATE_BOARD_LBL_SLOW"); color = sf::Color::Red; }

    mComplexityLabel->setText(mGame->GetLoc()->Get("GENERATE_BOARD_LBL_ESTIMATED_LOAD") + level);
    mComplexityLabel->getRenderer()->setTextColor(color);
}

std::pair<BoardConfigDialog::Result, GenerationParameters> BoardConfigDialog::run(sf::RenderWindow& window, sf::Clock& clock)
{
    // メインのゲームループと同様のブロッキングループ
    sf::Time lastTicks = clock.getElapsedTime();

    while (mIsActive && window.isOpen())
    {
        sf::Event event;
        while (window.pollEvent(event))
        {
            if (event.type == sf::Event::Closed) {
                mIsActive = false;
                break;
            }
            mGui.handleEvent(event);
        }

        window.clear();
        mGame->DrawSprites();   // 背面のゲーム画面を描画する
        mGame->DrawUI();        // 背面のUIを描画する (表示するだけで反応はしない)
        mGui.draw();
        window.display();
    }

    mGui.remove(mWindow);
    return { mResult, mParams };
}