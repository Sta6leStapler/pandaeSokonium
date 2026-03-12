#include "EditorScreen.h"
#include "Game.h"
#include "SFML/Graphics.hpp"

// コンストラクタ
EditorScreen::EditorScreen(Game* game, sf::RenderWindow* window, int loadMode)
    : mGame(game),
    mState(IUIScreen::UIState::EActive),
    mGui(std::make_unique<tgui::Gui>(*window)),
    mTheme(std::make_unique<tgui::Theme>("Assets/themes/Black.txt")),
    mEditorCanvasSize(10, 10),
    mCanvasSizeMin(10),
    mCanvasSizeMax(64),
    mCurrentTile(SelectedTile::Wall),
    mSelectedGridCell(-1, -1),
    mHoverGridCell(-1, -1),
    mTileButtonSize(tgui::Layout2d(140, 45)),
    mTileButtonOffset(tgui::Layout2d(20, 20)),
    mTileButtonInterval(tgui::Layout2d(0, mTileButtonSize.y + 20)),
    mSystemButtonSize(tgui::Layout2d(90, 30)),
    mSystemButtonOffset(tgui::Layout2d(20, 20)),
    mSystemButtonInterval(tgui::Layout2d(mSystemButtonSize.x + 20, 0)),
    mIconImageScale(0.75f),
    mIsMouseMode(true),
    mIsSelecting(false),
    mIsValidLevel(false),
    mIsDialogOpen(false)
{
    // 1. ゲームを一時停止状態にする
    mGame->SetState(Game::GameState::EPaused);

    // 2. このUIスクリーンをGameのスタックにプッシュ
    mGame->PushUI(this);

    // フォントを設定
    mGui->setFont(mGame->GetFontTGUI());

    // --- 1. 盤面データの初期化 ---
    // loadMode: 0=Empty, 1=Init, 2=Current
    if (loadMode == 0)
    {
        // 空の盤面 (10x10)
        mEditorCanvasSize = { 10, 10 };
        mEditorBoardData.assign(10, std::string(10, ' '));
    }
    else
    {
        // 現在の盤面データを取得
        std::vector<std::string> sourceData;
        if (loadMode == 1) sourceData = mGame->GetCurrentInitBoardData();
        else sourceData = mGame->GetBoardStateWithObjects();

        // データをコピー
        mEditorBoardData = sourceData;
        mEditorCanvasSize = { static_cast<int>(sourceData[0].size()), static_cast<int>(sourceData.size()) };
    }

    // 初期状態を「セーブ済み」として記録
    mLastSavedData = mEditorBoardData;

    // タイルと文字のマッピング
    mTileToChar[SelectedTile::Floor] = ' ';
    mTileToChar[SelectedTile::Wall] = '#';
    mTileToChar[SelectedTile::Goal] = '.';
    mTileToChar[SelectedTile::Baggage] = '$';
    mTileToChar[SelectedTile::Player] = '@';
    mTileToChar[SelectedTile::Erase] = ' ';

    // --- 描画スプライトとキャンバス設定 ---
    mFloorSprite.setTexture(*mGame->GetTexture("Assets/Floor.png"));
    mWallSprite.setTexture(*mGame->GetTexture("Assets/Wall.png"));
    mGoalSprite.setTexture(*mGame->GetTexture("Assets/Goal.png"));
    mBaggageSprite.setTexture(*mGame->GetTexture("Assets/Box.png"));
    mShinyBaggageSprite.setTexture(*mGame->GetTexture("Assets/ShinyBox.png"));
    mPlayerSprite.setTexture(*mGame->GetTexture("Assets/playerS.png"));

    // 1. エディタ用の子ウィンドウを作成
    mEditorWindow = tgui::ChildWindow::create();
    mEditorWindow->setRenderer(mTheme->getRenderer("ChildWindow"));
    mEditorWindow->setTitle(mGame->GetLoc()->Get("EDITOR_LBL_TITLE"));
    mEditorWindow->setSize(mGame->GetWindowSize().x * 0.95f, mGame->GetWindowSize().y * 0.95f); // 画面いっぱいに
    mEditorWindow->setPosition((mGame->GetWindowSize().x - mEditorWindow->getSize().x) / 2.0f, (mGame->GetWindowSize().y - mEditorWindow->getSize().y) / 2.0f);
    //mEditorWindow->setKeepInParent(true);
    mGui->add(mEditorWindow);

    // Exit Editorボタン以外で閉じられた場合もClose関数を呼び出す
    mEditorWindow->onClose([this]() { this->Close(); });

    // 2. 盤面描画用のキャンバスを作成
    mCanvasWidget = tgui::CanvasSFML::create();
    // キャンバスのサイズ計算
    float windowHeight = mEditorWindow->getSize().y; // 子ウィンドウの高さ概算
    // 上部のタイトルバーや下部のUIスペースを引いた高さを基準にする
    float availableHeight = windowHeight * 0.9f;
    mCanvasTileSize = std::floor(availableHeight / mEditorCanvasSize.y);

    mCanvasWidget->setSize(mCanvasTileSize * mEditorCanvasSize.x, mCanvasTileSize * mEditorCanvasSize.y);
    // ウィンドウ内の中央に配置（レイアウト機能を使用）
    mCanvasWidget->setPosition((mEditorWindow->getSize().x - mCanvasWidget->getSize().x) / 2.0f, (mEditorWindow->getSize().y - mEditorWindow->getChildWidgetsOffset().y - mCanvasWidget->getSize().y) / 2.0f);

    // キャンバスを子ウィンドウに追加
    mEditorWindow->add(mCanvasWidget);

    // スプライトのスケールをタイルサイズに合わせる
    mFloorSprite.setScale(mCanvasTileSize / mFloorSprite.getLocalBounds().width, -mCanvasTileSize / mFloorSprite.getLocalBounds().height);
    // 他のスプライトも同様にsetScaleする
    mWallSprite.setScale(mCanvasTileSize / mWallSprite.getLocalBounds().width, -mCanvasTileSize / mWallSprite.getLocalBounds().height);
    mGoalSprite.setScale(mCanvasTileSize / mGoalSprite.getLocalBounds().width, -mCanvasTileSize / mGoalSprite.getLocalBounds().height);
    mBaggageSprite.setScale(mCanvasTileSize / mBaggageSprite.getLocalBounds().width, -mCanvasTileSize / mBaggageSprite.getLocalBounds().height);
    mShinyBaggageSprite.setScale(mCanvasTileSize / mShinyBaggageSprite.getLocalBounds().width, -mCanvasTileSize / mShinyBaggageSprite.getLocalBounds().height);
    mPlayerSprite.setScale(mCanvasTileSize / mPlayerSprite.getLocalBounds().width, -mCanvasTileSize / mPlayerSprite.getLocalBounds().height);

    // グリッドとハイライトの初期設定
    mGridLines[0].setFillColor(sf::Color(100, 100, 100)); // 縦線
    mGridLines[1].setFillColor(sf::Color(100, 100, 100)); // 横線
    mSelectedCellHighlight.setSize({ mCanvasTileSize, mCanvasTileSize });
    mSelectedCellHighlight.setFillColor(sf::Color(255, 255, 0, 100)); // 黄色半透明

    // TGUIウィジェットの配置 (フェーズ1)
    // 左下のタイルパレット
    mTilePaletteSize = tgui::Layout2d(mCanvasWidget->getPosition().x * 0.9f, (mTileButtonSize.y + mTileButtonOffset.y) * 7 + mTileButtonOffset.y);
    mTilePaletteOffset = tgui::Layout2d((mCanvasWidget->getPosition().x - mTilePaletteSize.x) / 2.0f, mEditorWindow->getSize().y - mEditorWindow->getChildWidgetsOffset().y - mTilePaletteSize.y * 1.05f);
    mTilePalettePanel = tgui::Panel::create();
    mTilePalettePanel->setRenderer(mTheme->getRenderer("Panel"));
    mTilePalettePanel->setSize(mTilePaletteSize);
    mTilePalettePanel->setPosition(mTilePaletteOffset);
    mTilePalettePanel->getRenderer()->setBorders(2);
    mTilePalettePanel->getRenderer()->setBorderColor(sf::Color(100, 100, 100));
    mEditorWindow->add(mTilePalettePanel);

    // ハイライト中のタイルを表示するラジオボタン
    auto highlightedTileGroup = tgui::RadioButtonGroup::create();
    highlightedTileGroup->setRenderer(mTheme->getRenderer("RadioButtonGroup"));
    mTilePalettePanel->add(highlightedTileGroup);

    // パレット内のボタン
    // 壁マス
    // ラジオボタン
    mWallRadioButton = tgui::RadioButton::create();
    mWallRadioButton->setRenderer(mTheme->getRenderer("RadioButton"));
    mWallRadioButton->setSize(mTileButtonSize.y * 0.5f, mTileButtonSize.y * 0.5f);
    mWallRadioButton->setPosition(mTileButtonOffset + mTileButtonInterval * 0 + (tgui::Layout2d(mTileButtonSize.y, mTileButtonSize.y) - mWallRadioButton->getSize()) / 2);
    mWallRadioButton->setChecked(true);
    mWallRadioButton->onCheck([this]() { this->mCurrentTile = SelectedTile::Wall; });
    highlightedTileGroup->add(mWallRadioButton);
    // アイコン付きのボタン
    sf::Image tmpImage = mGame->LoadTexture("Assets/Wall.png")->copyToImage();
    tgui::Texture tmpTGuiTex{};
    tmpTGuiTex.loadFromPixelData(tmpImage.getSize(), tmpImage.getPixelsPtr());
    auto btnWall = tgui::BitmapButton::create("");
    btnWall->setRenderer(mTheme->getRenderer("BitmapButton"));
    btnWall->setImage(tmpTGuiTex);
    btnWall->setImageScaling(mIconImageScale);
    btnWall->setPosition(mTileButtonOffset + mTileButtonInterval * 0 + tgui::Layout2d(mTileButtonSize.y, 0));
    btnWall->setSize(mTileButtonSize - tgui::Layout2d(mTileButtonSize.y, 0));
    btnWall->onPress([this]() { mCurrentTile = SelectedTile::Wall; });
    mTilePalettePanel->add(btnWall);
    // 範囲選択用アクションボタン
    mBtnFillWall = tgui::BitmapButton::create(" " + mGame->GetLoc()->Get("EDITOR_BTN_FILL"));
    mBtnFillWall->setRenderer(mTheme->getRenderer("BitmapButton"));
    mBtnFillWall->setImage(tmpTGuiTex);
    mBtnFillWall->setImageScaling(mIconImageScale);
    mBtnFillWall->setPosition(btnWall->getPosition() - tgui::Layout2d(mTileButtonSize.y, 0) + tgui::Layout2d(mTileButtonSize.x + 20, 0));
    mBtnFillWall->setSize(mTileButtonSize);
    mBtnFillWall->setEnabled(false); // 初期状態は無効
    mBtnFillWall->onPress([this]() { FillSelection(SelectedTile::Wall); });
    mTilePalettePanel->add(mBtnFillWall);

    // 床マス
    // ラジオボタン
    mFloorRadioButton = tgui::RadioButton::create();
    mFloorRadioButton->setRenderer(mTheme->getRenderer("RadioButton"));
    mFloorRadioButton->setSize(mTileButtonSize.y * 0.5f, mTileButtonSize.y * 0.5f);
    mFloorRadioButton->setPosition(mTileButtonOffset + mTileButtonInterval * 1 + (tgui::Layout2d(mTileButtonSize.y, mTileButtonSize.y) - mFloorRadioButton->getSize()) / 2);
    mFloorRadioButton->onCheck([this]() { this->mCurrentTile = SelectedTile::Floor; });
    highlightedTileGroup->add(mFloorRadioButton);
    // アイコン付きのボタン
    tmpImage = mGame->LoadTexture("Assets/Floor.png")->copyToImage();
    tmpTGuiTex.loadFromPixelData(tmpImage.getSize(), tmpImage.getPixelsPtr());
    auto btnFloor = tgui::BitmapButton::create("");
    btnFloor->setRenderer(mTheme->getRenderer("BitmapButton"));
    btnFloor->setImage(tmpTGuiTex);
    btnFloor->setImageScaling(mIconImageScale);
    btnFloor->setPosition(mTileButtonOffset + mTileButtonInterval * 1 + mTileButtonInterval * 0 + tgui::Layout2d(mTileButtonSize.y, 0));
    btnFloor->setSize(mTileButtonSize - tgui::Layout2d(mTileButtonSize.y, 0));
    btnFloor->onPress([this]() { mCurrentTile = SelectedTile::Floor; });
    mTilePalettePanel->add(btnFloor);
    // 範囲選択用アクションボタン
    mBtnFillFloor = tgui::BitmapButton::create(" " + mGame->GetLoc()->Get("EDITOR_BTN_FILL"));
    mBtnFillFloor->setRenderer(mTheme->getRenderer("BitmapButton"));
    mBtnFillFloor->setImage(tmpTGuiTex);
    mBtnFillFloor->setImageScaling(mIconImageScale);
    mBtnFillFloor->setPosition(btnFloor->getPosition() - tgui::Layout2d(mTileButtonSize.y, 0) + tgui::Layout2d(mTileButtonSize.x + 20, 0));
    mBtnFillFloor->setSize(mTileButtonSize);
    mBtnFillFloor->setEnabled(false);
    mBtnFillFloor->onPress([this]() { FillSelection(SelectedTile::Floor); });
    mTilePalettePanel->add(mBtnFillFloor);

    // ゴールマス
    // ラジオボタン
    mGoalRadioButton = tgui::RadioButton::create();
    mGoalRadioButton->setRenderer(mTheme->getRenderer("RadioButton"));
    mGoalRadioButton->setSize(mTileButtonSize.y * 0.5f, mTileButtonSize.y * 0.5f);
    mGoalRadioButton->setPosition(mTileButtonOffset + mTileButtonInterval * 2 + (tgui::Layout2d(mTileButtonSize.y, mTileButtonSize.y) - mGoalRadioButton->getSize()) / 2);
    mGoalRadioButton->onCheck([this]() { this->mCurrentTile = SelectedTile::Goal; });
    highlightedTileGroup->add(mGoalRadioButton);
    // アイコン付きのボタン
    tmpImage = mGame->LoadTexture("Assets/Goal.png")->copyToImage();
    tmpTGuiTex.loadFromPixelData(tmpImage.getSize(), tmpImage.getPixelsPtr());
    auto btnGoal = tgui::BitmapButton::create("");
    btnGoal->setRenderer(mTheme->getRenderer("BitmapButton"));
    btnGoal->setImage(tmpTGuiTex);
    btnGoal->setImageScaling(mIconImageScale);
    btnGoal->setPosition(mTileButtonOffset + mTileButtonInterval * 2 + tgui::Layout2d(mTileButtonSize.y, 0));
    btnGoal->setSize(mTileButtonSize - tgui::Layout2d(mTileButtonSize.y, 0));
    btnGoal->onPress([this]() { mCurrentTile = SelectedTile::Goal; });
    mTilePalettePanel->add(btnGoal);
    // 範囲選択用アクションボタン
    mBtnFillGoal = tgui::BitmapButton::create(" " + mGame->GetLoc()->Get("EDITOR_BTN_FILL"));
    mBtnFillGoal->setRenderer(mTheme->getRenderer("BitmapButton"));
    mBtnFillGoal->setImage(tmpTGuiTex);
    mBtnFillGoal->setImageScaling(mIconImageScale);
    mBtnFillGoal->setPosition(btnGoal->getPosition() - tgui::Layout2d(mTileButtonSize.y, 0) + tgui::Layout2d(mTileButtonSize.x + 20, 0));
    mBtnFillGoal->setSize(mTileButtonSize);
    mBtnFillGoal->setEnabled(false);
    mBtnFillGoal->onPress([this]() { FillSelection(SelectedTile::Goal); });
    mTilePalettePanel->add(mBtnFillGoal);

    // 荷物
    // ラジオボタン
    mBaggageRadioButton = tgui::RadioButton::create();
    mBaggageRadioButton->setRenderer(mTheme->getRenderer("RadioButton"));
    mBaggageRadioButton->setSize(mTileButtonSize.y * 0.5f, mTileButtonSize.y * 0.5f);
    mBaggageRadioButton->setPosition(mTileButtonOffset + mTileButtonInterval * 3 + (tgui::Layout2d(mTileButtonSize.y, mTileButtonSize.y) - mBaggageRadioButton->getSize()) / 2);
    mBaggageRadioButton->onCheck([this]() { this->mCurrentTile = SelectedTile::Baggage; });
    highlightedTileGroup->add(mBaggageRadioButton);
    // アイコン付きのボタン
    tmpImage = mGame->LoadTexture("Assets/Box.png")->copyToImage();
    tmpTGuiTex.loadFromPixelData(tmpImage.getSize(), tmpImage.getPixelsPtr());
    auto btnBaggage = tgui::BitmapButton::create("");
    btnBaggage->setRenderer(mTheme->getRenderer("BitmapButton"));
    btnBaggage->setImage(tmpTGuiTex);
    btnBaggage->setImageScaling(mIconImageScale);
    btnBaggage->setPosition(mTileButtonOffset + mTileButtonInterval * 3 + tgui::Layout2d(mTileButtonSize.y, 0));
    btnBaggage->setSize(mTileButtonSize - tgui::Layout2d(mTileButtonSize.y, 0));
    btnBaggage->onPress([this]() { mCurrentTile = SelectedTile::Baggage; });
    mTilePalettePanel->add(btnBaggage);

    // プレイヤー
    // ラジオボタン
    mPlayerRadioButton = tgui::RadioButton::create();
    mPlayerRadioButton->setRenderer(mTheme->getRenderer("RadioButton"));
    mPlayerRadioButton->setSize(mTileButtonSize.y * 0.5f, mTileButtonSize.y * 0.5f);
    mPlayerRadioButton->setPosition(mTileButtonOffset + mTileButtonInterval * 4 + (tgui::Layout2d(mTileButtonSize.y, mTileButtonSize.y) - mPlayerRadioButton->getSize()) / 2);
    mPlayerRadioButton->onCheck([this]() { this->mCurrentTile = SelectedTile::Player; });
    highlightedTileGroup->add(mPlayerRadioButton);
    // アイコン付きのボタン
    tmpImage = mGame->LoadTexture("Assets/PlayerN.png")->copyToImage();
    tmpTGuiTex.loadFromPixelData(tmpImage.getSize(), tmpImage.getPixelsPtr());
    auto btnPlayer = tgui::BitmapButton::create("");
    btnPlayer->setRenderer(mTheme->getRenderer("BitmapButton"));
    btnPlayer->setImage(tmpTGuiTex);
    btnPlayer->setImageScaling(mIconImageScale);
    btnPlayer->setPosition(mTileButtonOffset + mTileButtonInterval * 4 + tgui::Layout2d(mTileButtonSize.y, 0));
    btnPlayer->setSize(mTileButtonSize - tgui::Layout2d(mTileButtonSize.y, 0));
    btnPlayer->onPress([this]() { mCurrentTile = SelectedTile::Player; });
    mTilePalettePanel->add(btnPlayer);

    // 削除
    // ラジオボタン
    mEraseRadioButton = tgui::RadioButton::create();
    mEraseRadioButton->setRenderer(mTheme->getRenderer("RadioButton"));
    mEraseRadioButton->setSize(mTileButtonSize.y * 0.5f, mTileButtonSize.y * 0.5f);
    mEraseRadioButton->setPosition(mTileButtonOffset + mTileButtonInterval * 5 + (tgui::Layout2d(mTileButtonSize.y, mTileButtonSize.y) - mEraseRadioButton->getSize()) / 2);
    mEraseRadioButton->onCheck([this]() { this->mCurrentTile = SelectedTile::Erase; });
    highlightedTileGroup->add(mEraseRadioButton);
    // ボタン
    auto btnErase = tgui::Button::create(mGame->GetLoc()->Get("EDITOR_BTN_ERASE"));
    btnErase->setRenderer(mTheme->getRenderer("Button"));
    btnErase->setPosition(mTileButtonOffset + mTileButtonInterval * 5 + tgui::Layout2d(mTileButtonSize.y, 0));
    btnErase->setSize(mTileButtonSize - tgui::Layout2d(mTileButtonSize.y, 0));
    btnErase->onPress([this]() { mCurrentTile = SelectedTile::Erase; });
    mTilePalettePanel->add(btnErase);
    // 範囲選択用アクションボタン
    mBtnDeleteArea = tgui::Button::create(mGame->GetLoc()->Get("EDITOR_BTN_DELETE_AREA"));
    mBtnDeleteArea->setRenderer(mTheme->getRenderer("Button"));
    mBtnDeleteArea->setPosition(btnErase->getPosition() + tgui::Layout2d(mTileButtonSize.x + 20, 0) - tgui::Layout2d(mTileButtonSize.y, 0));
    mBtnDeleteArea->setSize(mTileButtonSize);
    mBtnDeleteArea->setEnabled(false);
    mBtnDeleteArea->onPress([this]() { FillSelection(SelectedTile::Erase); });
    mTilePalettePanel->add(mBtnDeleteArea);

    // モード切替
    mModeGroup = tgui::RadioButtonGroup::create();
    mModeGroup->setRenderer(mTheme->getRenderer("RadioButtonGroup"));
    mTilePalettePanel->add(mModeGroup);

    tgui::Layout2d radioButtonOffset(0, 0);
    mRadioDraw = tgui::RadioButton::create();
    mRadioDraw->setRenderer(mTheme->getRenderer("RadioButton"));
    mRadioDraw->setTextSize(16);
    mRadioDraw->setText(mGame->GetLoc()->Get("EDITOR_LBL_DRAW_MODE"));
    radioButtonOffset = tgui::Layout2d((mTileButtonSize - mRadioDraw->getFullSize()) / 2);
    mRadioDraw->setPosition(mTileButtonOffset + mTileButtonInterval * 6 + radioButtonOffset);
    mRadioDraw->setChecked(true);
    mModeGroup->add(mRadioDraw);

    mRadioSelect = tgui::RadioButton::create();
    mRadioSelect->setRenderer(mTheme->getRenderer("RadioButton"));
    mRadioSelect->setTextSize(16);
    mRadioSelect->setText(mGame->GetLoc()->Get("EDITOR_LBL_SELECT_MODE"));
    radioButtonOffset = tgui::Layout2d((mTileButtonSize - mRadioSelect->getFullSize()) / 2);
    mRadioSelect->setPosition(mTileButtonOffset + mTileButtonInterval * 6 + tgui::Layout2d(mTileButtonSize.x + 20, 0) + radioButtonOffset);
    mModeGroup->add(mRadioSelect);

    // モード変更時のイベントハンドラ
    mRadioDraw->onChange([this](bool checked) {
        // こちらのラジオボタンのON/OFFで切り替わるものがあれば処理を追加
        });
    mRadioSelect->onChange([this](bool checked) {
        // ラジオボタンがオンの場合
        if (checked)
        {
            // アクションボタンを有効化
            mBtnFillWall->setEnabled(true);
            mBtnFillFloor->setEnabled(true);
            mBtnFillGoal->setEnabled(true);
            mBtnDeleteArea->setEnabled(true);
        }
        // ラジオボタンがオフの場合
        else
        {
            // アクションボタン無効化
            mIsSelecting = false;
            mDragStartPos = { -1, -1 };
            mBtnFillWall->setEnabled(false);
            mBtnFillFloor->setEnabled(false);
            mBtnFillGoal->setEnabled(false);
            mBtnDeleteArea->setEnabled(false);
        }
        });

    // 右下のボタン
    mSystemButtonPaletteSize = tgui::Layout2d((mSystemButtonSize.x + mSystemButtonOffset.x) * 3 + mSystemButtonOffset.x, mSystemButtonSize.y + mSystemButtonOffset.y * 2);
    mSystemButtonPaletteOffset = tgui::Layout2d(mEditorWindow->getSize().x - mSystemButtonPaletteSize.x, mEditorWindow->getSize().y - mEditorWindow->getChildWidgetsOffset().y - mSystemButtonPaletteSize.y);
    mSystemPalettePanel = tgui::Panel::create();
    mSystemPalettePanel->setRenderer(mTheme->getRenderer("Panel"));
    mSystemPalettePanel->setSize(mSystemButtonPaletteSize);
    mSystemPalettePanel->setPosition(mSystemButtonPaletteOffset);
    mEditorWindow->add(mSystemPalettePanel);

    // レベル適用ボタン
    mApplyButton = tgui::Button::create(mGame->GetLoc()->Get("EDITOR_BTN_APPLY_LEVEL"));
    mApplyButton->setRenderer(mTheme->getRenderer("Button"));
    mApplyButton->setSize(mSystemButtonSize);
    mApplyButton->setPosition(mSystemButtonOffset + mSystemButtonInterval * 0);
    mApplyButton->onPress([this]() {
        PerformApply();
        });
    mSystemPalettePanel->add(mApplyButton);

    // レベルセーブボタン
    mSaveButton = tgui::Button::create(mGame->GetLoc()->Get("EDITOR_BTN_SAVE_LEVEL"));
    mSaveButton->setRenderer(mTheme->getRenderer("Button"));
    mSaveButton->setSize(mSystemButtonSize);
    mSaveButton->setPosition(mSystemButtonOffset + mSystemButtonInterval * 1);
    // セーブ後にエディタ画面に戻るためにfalse
    mSaveButton->onPress([this]() {
        PerformSave(false);
        });
    mSystemPalettePanel->add(mSaveButton);

    // エディタ終了ボタン
    mExitButton = tgui::Button::create(mGame->GetLoc()->Get("EDITOR_BTN_EXIT_EDITOR"));
    mExitButton->setRenderer(mTheme->getRenderer("Button"));
    mExitButton->setSize(mSystemButtonSize);
    mExitButton->setPosition(mSystemButtonOffset + mSystemButtonInterval * 2);
    mExitButton->onPress([this]() { CheckUnsavedChangesAndClose(); });
    mSystemPalettePanel->add(mExitButton);

    // 右の情報パネル
    mInfoPanel = tgui::Panel::create();
    mInfoPanel->setRenderer(mTheme->getRenderer("Panel"));
    mInfoPanel->setSize((mEditorWindow->getSize().x - mCanvasWidget->getSize().x - mCanvasWidget->getPosition().x) * 0.9f, (mEditorWindow->getSize().y - mSystemButtonPaletteSize.y) * 0.95f);
    mInfoPanel->setPosition(mCanvasWidget->getPosition().x + mCanvasWidget->getSize().x + ((mEditorWindow->getSize().x - mCanvasWidget->getSize().x - mCanvasWidget->getPosition().x - mInfoPanel->getSize().x) / 2.0f), (mEditorWindow->getSize().y - mSystemButtonPaletteSize.y - mInfoPanel->getSize().y) / 2.0f);
    mInfoPanel->getRenderer()->setBorders(2);
    mInfoPanel->getRenderer()->setBorderColor(sf::Color(100, 100, 100));
    mEditorWindow->add(mInfoPanel);

    mInfoLabel = tgui::Label::create(mGame->GetLoc()->Get("EDITOR_RIGHT_PANEL_BAGGAGE") + "0 / " + mGame->GetLoc()->Get("EDITOR_RIGHT_PANEL_GOAL") + "0");
    mInfoLabel->setPosition(10, 10);
    mInfoPanel->add(mInfoLabel);

    mInfoBaggageGoaledLabel = tgui::Label::create(mGame->GetLoc()->Get("EDITOR_RIGHT_PANEL_STATUS") + mGame->GetLoc()->Get("EDITOR_RIGHT_PANEL_OK_STATE"));
    mInfoBaggageGoaledLabel->setPosition(10, 40);
    mInfoPanel->add(mInfoBaggageGoaledLabel);

    // mSelectedGridCell の初期位置を (0, 0) に設定しておく
    mSelectedGridCell = sf::Vector2i(0, 0);

    // mSelectedGridCell の初期位置を (0, 0) に設定しておく
    mSelectedGridCell = sf::Vector2i(0, 0);

    // ハイライトシェイプの設定
    // 1. 選択カーソル (濃い枠線)
    mCursorHighlight.setSize({ mCanvasTileSize, mCanvasTileSize });
    mCursorHighlight.setFillColor(sf::Color::Transparent); // 中身は透明
    mCursorHighlight.setOutlineColor(sf::Color::Yellow);   // 枠は黄色
    mCursorHighlight.setOutlineThickness(3.0f);            // 太めの枠

    // 2. マウスホバー (薄い塗りつぶし)
    mHoverHighlight.setSize({ mCanvasTileSize, mCanvasTileSize });
    mHoverHighlight.setFillColor(sf::Color(255, 255, 255, 50)); // 白の半透明
    mHoverHighlight.setOutlineThickness(0.0f);


    // ヘルプ表示エリア
    auto helpArea = tgui::TextArea::create();
    helpArea->setRenderer(mTheme->getRenderer("TextArea"));
    helpArea->setSize(mCanvasWidget->getPosition().x * 0.9f, mTilePaletteOffset.y * 0.95f);
    helpArea->setPosition((mCanvasWidget->getPosition().x - helpArea->getSize().x) / 2.0f, (mTilePaletteOffset.y - helpArea->getSize().y) / 2.0f );
    helpArea->setTextSize(14);
    helpArea->setReadOnly(true); // 編集不可
    helpArea->setText(
        mGame->GetLoc()->Get("EDITOR_LEFT_PANEL_KEY_BIND") + "\n" +
        mGame->GetLoc()->Get("EDITOR_LEFT_PANEL_WASD") + "\n" +
        mGame->GetLoc()->Get("EDITOR_LEFT_PANEL_SPACE") + "\n" +
        mGame->GetLoc()->Get("EDITOR_LEFT_PANEL_TAB") + "\n" +
        mGame->GetLoc()->Get("EDITOR_LEFT_PANEL_DELETE") + "\n" +
        mGame->GetLoc()->Get("EDITOR_LEFT_PANEL_CTRL_S") + "\n\n" +
        mGame->GetLoc()->Get("EDITOR_LEFT_PANEL_MOUSE") + "\n" +
        mGame->GetLoc()->Get("EDITOR_LEFT_PANEL_MOUSE_LEFT") + "\n" +
        mGame->GetLoc()->Get("EDITOR_LEFT_PANEL_MOUSE_RIGHT") + "\n" +
        mGame->GetLoc()->Get("EDITOR_LEFT_PANEL_MOUSE_DRAG")
    );
    mEditorWindow->add(helpArea);

    // 初回のキャンバス調整とバリデーション
    ResizeCanvasIfNeeded();
    UpdateValidationState();
}

// デストラクタ
EditorScreen::~EditorScreen()
{
    mGame->SetState(Game::GameState::EGamePlay);
    // mGuiはunique_ptrが自動で解放
}

void EditorScreen::Close()
{
    mState = UIState::EClosing;
    mGui->removeAllWidgets();
}

// 毎フレームの更新
void EditorScreen::Update(float deltaTime)
{
    // GameクラスのmGameStateがEGamePlayになってしまうので、EPausedに更新し続ける
    mGame->SetState(Game::GameState::EPaused);

    // 情報パネルを更新
    UpdateInfoPanel();

    // タイル選択のラジオボタンをインスタンス内の状態と同期
    mWallRadioButton->setChecked(false);
    mFloorRadioButton->setChecked(false);
    mGoalRadioButton->setChecked(false);
    mBaggageRadioButton->setChecked(false);
    mPlayerRadioButton->setChecked(false);
    mEraseRadioButton->setChecked(false);
    switch (mCurrentTile)
    {
    case SelectedTile::Wall: mWallRadioButton->setChecked(true); break;
    case SelectedTile::Floor: mFloorRadioButton->setChecked(true); break;
    case SelectedTile::Goal: mGoalRadioButton->setChecked(true); break;
    case SelectedTile::Baggage: mBaggageRadioButton->setChecked(true); break;
    case SelectedTile::Player: mPlayerRadioButton->setChecked(true); break;
    case SelectedTile::Erase: mEraseRadioButton->setChecked(true); break;
    default: break;
    }
}

// 描画処理
void EditorScreen::Draw(sf::RenderWindow* window)
{
    // 1. 最初に自前のキャンバスを描画する
    DrawCanvas();

    // 2. その上にTGUIのUIを描画する
    mGui->draw();
}

// 入力処理
void EditorScreen::ProcessInput(const sf::Event* event, const sf::Vector2i& mousePos)
{
    // TGUIにイベントを渡す
    bool eventConsumed = mGui->handleEvent(*event);

    // ダイアログ表示中はキャンバス操作をブロック
    if (mIsDialogOpen) return;

    // エディットウィンドウのタイトルバーを除いて左上を原点としたマウスの座標
    sf::Vector2f childWindowMousePos( (float)mousePos.x - mEditorWindow->getPosition().x, (float)mousePos.y - mEditorWindow->getPosition().y);
    // エディットウィンドウのタイトルバーを加味して左上を原点としたマウスの座標
    sf::Vector2f canvasMousePos( childWindowMousePos.x - mEditorWindow->getChildWidgetsOffset().x, childWindowMousePos.y - mEditorWindow->getChildWidgetsOffset().y);

    // キャンバス上でマウスが動いたりクリックされた場合、TGUIは「消費された」とみなすが、
    // エディタとしては盤面操作を行いたいので、例外的に処理を続行させる。
    bool isMouseOnCanvas = mCanvasWidget->isMouseOnWidget(canvasMousePos);

    if (eventConsumed && !isMouseOnCanvas)
    {
        // キャンバス以外のUI（ボタンなど）が操作された場合はここで終了
        return;
    }

    // マウス位置の計算とホバー更新
    sf::Vector2i gridPos = ScreenToGrid(sf::Vector2f(mousePos));

    // マウスが動いたら「マウスモード」にする
    if (event->type == sf::Event::MouseMoved)
    {
        mIsMouseMode = true;
        mHoverGridCell = gridPos; // ホバー位置更新

        // 左クリックが押されたまま移動した場合
        if (mRadioDraw->isChecked() && sf::Mouse::isButtonPressed(sf::Mouse::Left))
        {
            if (gridPos.x != -1)
            {
                mSelectedGridCell = gridPos; // カーソルも追従
                // プレイヤーと荷物は連続配置しない（トグル動作との競合を防ぐ）
                PlaceTile(gridPos, true);
            }
        }

        // Selectモード時のドラッグ範囲更新
        if (mRadioSelect->isChecked() && mIsSelecting)
        {
            // ドラッグ中は終点を更新（範囲外でもクランプして更新するなど工夫可）
            // 左クリックが押されている間だけ終点を更新する
            if (sf::Mouse::isButtonPressed(sf::Mouse::Left))
            {
                if (gridPos.x >= 0 && gridPos.x < mEditorCanvasSize.x && gridPos.y >= 0 && gridPos.y < mEditorCanvasSize.y) mDragEndPos = gridPos;
            }
        }
    }

    // マウス操作
    if (gridPos.x != -1)
    {
        // クリックされたら、選択カーソルもそこに移動させる
        if (event->type == sf::Event::MouseButtonPressed && event->mouseButton.button == sf::Mouse::Left)
        {
            mSelectedGridCell = gridPos; // カーソル移動
            mIsMouseMode = true;

            if (mRadioDraw->isChecked())
            {
                // Drawモード: 即座に配置
                PlaceTile(gridPos, false);
            }
            else if (mRadioSelect->isChecked())
            {
                // Selectモード: 範囲選択開始
                mIsSelecting = true;
                mDragStartPos = gridPos;
                mDragEndPos = gridPos;

                // 選択開始時にボタン類は一度無効化（ドラッグ完了まで）
                mBtnFillWall->setEnabled(false);
                mBtnFillFloor->setEnabled(false);
                mBtnFillGoal->setEnabled(false);
                mBtnDeleteArea->setEnabled(false);
            }
        }
        // 右クリック (Drawモードのみ有効、Erase)
        else if (event->type == sf::Event::MouseButtonPressed && event->mouseButton.button == sf::Mouse::Right)
        {
            // 配置モードなら選択されたタイルを解除 (床タイルに戻る)
            if (mRadioDraw->isChecked())
            {
                mSelectedGridCell = gridPos;
                SelectedTile prevTile = mCurrentTile;
                mCurrentTile = SelectedTile::Erase;
                PlaceTile(gridPos);
                mCurrentTile = prevTile;
            }
        }
        // 左クリック離上 (Selectモード用)
        else if (event->type == sf::Event::MouseButtonReleased && event->mouseButton.button == sf::Mouse::Left)
        {
            if (mRadioSelect->isChecked() && mIsSelecting)
            {
                // ドラッグ終了。範囲が確定したのでボタンを有効化
                mBtnFillWall->setEnabled(true);
                mBtnFillFloor->setEnabled(true);
                mBtnFillGoal->setEnabled(true);
                mBtnDeleteArea->setEnabled(true);

                // mIsSelectingはtrueのまま維持（範囲を表示し続けるため）
                // 解除は右クリックやモード切替で行う
            }
        }
    }
    else
    {
        mHoverGridCell = { -1, -1 };
    }

    // キーボード操作
    if (event->type == sf::Event::KeyPressed)
    {
        // キー入力があったら「キーボードモード」にする (ホバーを隠す)
        mIsMouseMode = false;

        // 範囲選択モードかつShiftキーが押された瞬間 -> 選択開始
        if (mRadioSelect->isChecked() &&
            (event->key.code == sf::Keyboard::LShift || event->key.code == sf::Keyboard::RShift))
        {
            mIsSelecting = true;
            mDragStartPos = mSelectedGridCell;
            mDragEndPos = mSelectedGridCell;
            // ボタン無効化
            mBtnFillWall->setEnabled(false); mBtnFillFloor->setEnabled(false);
            mBtnFillGoal->setEnabled(false); mBtnDeleteArea->setEnabled(false);
        }

        // カーソル移動 (WASD / 矢印)
        // 範囲チェックを行って移動
        sf::Vector2i newPos = mSelectedGridCell;
        if (event->key.code == sf::Keyboard::W || event->key.code == sf::Keyboard::Up)    newPos.y--;
        if (event->key.code == sf::Keyboard::S || event->key.code == sf::Keyboard::Down)  newPos.y++;
        if (event->key.code == sf::Keyboard::A || event->key.code == sf::Keyboard::Left)  newPos.x--;
        if (event->key.code == sf::Keyboard::D || event->key.code == sf::Keyboard::Right) newPos.x++;

        // 範囲内なら適用
        if (newPos.x >= 0 && newPos.x < mEditorCanvasSize.x &&
            newPos.y >= 0 && newPos.y < mEditorCanvasSize.y)
        {
            mSelectedGridCell = newPos;

            // 範囲選択中の場合、終点を更新
            if (mRadioSelect->isChecked() && mIsSelecting)
            {
                mDragEndPos = mSelectedGridCell;
            }

            // DrawモードでEnterキーを押しながら移動 -> 連続配置
            if (mRadioDraw->isChecked() && sf::Keyboard::isKeyPressed(sf::Keyboard::Enter))
            {
                PlaceTile(mSelectedGridCell, true);
            }
        }

        // パネル配置 (Space / Enter)
        if (event->key.code == sf::Keyboard::Space || event->key.code == sf::Keyboard::Enter)
        {
            // Enter単押しの場合は単発配置(false)
            // (移動しながらのEnterは上の移動ブロックで処理される)
            PlaceTile(mSelectedGridCell, false);
        }
        // 削除 (Delete / BackSpace)
        if (event->key.code == sf::Keyboard::Delete || event->key.code == sf::Keyboard::BackSpace)
        {
            SelectedTile prevTile = mCurrentTile;
            mCurrentTile = SelectedTile::Erase;
            PlaceTile(mSelectedGridCell);
            mCurrentTile = prevTile;
        }

        // パネル切り替え (Tab)
        if (event->key.code == sf::Keyboard::Tab)
        {
            int tileVal = static_cast<int>(mCurrentTile);

            if (event->key.shift) // Shift + Tab : 逆順
            {
                if (0 < tileVal) tileVal--;
            }
            else // Tab : 正順
            {
                if (tileVal < static_cast<int>(SelectedTile::Erase)) tileVal++;
            }

            mCurrentTile = static_cast<SelectedTile>(tileVal);
        }

        // タイル配置モードと範囲選択モードの切替 (M)
        if (event->key.code == sf::Keyboard::M)
        {
            if (mRadioDraw->isChecked())
            {
                mRadioSelect->setChecked(true); // Draw -> Select
            }
            else
            {
                mRadioDraw->setChecked(true);   // Select -> Draw
            }
        }

        // クイックセーブ (Ctrl + S)
        if (event->key.control && event->key.code == sf::Keyboard::S)
        {
            // セーブ後にエディタ画面に戻るためにfalse
            PerformSave(false);
        }
    }

    // Shiftキー離上イベント (範囲選択の終了)
    if (event->type == sf::Event::KeyReleased)
    {
        if (mRadioSelect->isChecked() &&
            (event->key.code == sf::Keyboard::LShift || event->key.code == sf::Keyboard::RShift))
        {
            if (mIsSelecting)
            {
                // 選択完了、ボタン有効化
                mBtnFillWall->setEnabled(true); mBtnFillFloor->setEnabled(true);
                mBtnFillGoal->setEnabled(true); mBtnDeleteArea->setEnabled(true);
            }
        }
    }
}

// プライベートヘルパー関数

void EditorScreen::DrawCanvas()
{
    // キャンバスをクリア (背景色を指定)
    mCanvasWidget->clear(sf::Color(30, 30, 30));

    // スプライトの描画 (座標はキャンバス左上 (0,0) からの相対座標)
    sf::Vector2f currentPos(0.0f, mCanvasTileSize * static_cast<float>(mEditorCanvasSize.y));

    // 盤面タイルを描画
    for (int y = 0; y < mEditorCanvasSize.y; ++y)
    {
        currentPos.x = 0.0f;
        for (int x = 0; x < mEditorCanvasSize.x; ++x)
        {
            char tile = mEditorBoardData[y][x];
            sf::Sprite* spriteToDraw = &mFloorSprite; // デフォルトは床

            // 床以外の地形マスの場合の設定
            switch (tile)
            {
            case '#': spriteToDraw = &mWallSprite; break;
            case '.': spriteToDraw = &mGoalSprite; break;
            case '*': spriteToDraw = &mGoalSprite; break;
            case '+': spriteToDraw = &mGoalSprite; break;
            }

            // 地形部分を描画
            spriteToDraw->setPosition(currentPos);
            mCanvasWidget->draw(*spriteToDraw);

            // 荷物とプレイヤーの描画
            if (tile == '$')
            {
                mBaggageSprite.setPosition(currentPos);
                mCanvasWidget->draw(mBaggageSprite);
            }
            else if (tile == '*')
            {
                mShinyBaggageSprite.setPosition(currentPos);
                mCanvasWidget->draw(mShinyBaggageSprite);
            }
            else if (tile == '@' || tile == '+')
            {
                mPlayerSprite.setPosition(currentPos);
                mCanvasWidget->draw(mPlayerSprite);
            }

            // グリッド線 (縦)
            mGridLines[0].setSize({ 1.f, mCanvasTileSize });
            mGridLines[0].setPosition(currentPos - sf::Vector2f{ 0.0f, mCanvasTileSize });
            mCanvasWidget->draw(mGridLines[0]);

            currentPos.x += mCanvasTileSize;
        }
        // グリッド線 (横)
        mGridLines[1].setSize({ mCanvasTileSize * mEditorCanvasSize.x, 1.f });
        mGridLines[1].setPosition(0.0f, currentPos.y);
        mCanvasWidget->draw(mGridLines[1]);

        currentPos.y -= mCanvasTileSize;
    }

    // マウスホバーのハイライト (マウスモード時のみ表示)
    if (mIsMouseMode && mHoverGridCell.x != -1)
    {
        mHoverHighlight.setPosition(
            mHoverGridCell.x * mCanvasTileSize,
            mCanvasTileSize * static_cast<float>(mEditorCanvasSize.y - 1) - mHoverGridCell.y * mCanvasTileSize
        );
        mCanvasWidget->draw(mHoverHighlight);
    }

    // 選択中マスのハイライトを描画 (フェーズ2)
    if (mSelectedGridCell.x != -1)
    {
        mSelectedCellHighlight.setPosition(
            mSelectedGridCell.x * mCanvasTileSize,
            mCanvasTileSize * static_cast<float>(mEditorCanvasSize.y - 1) - mSelectedGridCell.y * mCanvasTileSize
        );
        mCanvasWidget->draw(mSelectedCellHighlight);
    }

    if (mIsSelecting)
    {
        // 矩形範囲を計算
        int startX = std::min(mDragStartPos.x, mDragEndPos.x);
        int startY = std::min(mDragStartPos.y, mDragEndPos.y);
        int endX = std::max(mDragStartPos.x, mDragEndPos.x);
        int endY = std::max(mDragStartPos.y, mDragEndPos.y);

        mSelectionRect.setPosition(startX * mCanvasTileSize, (mEditorCanvasSize.y - startY) * mCanvasTileSize);
        mSelectionRect.setSize({ (endX - startX + 1) * mCanvasTileSize, -(endY - startY + 1) * mCanvasTileSize });
        mSelectionRect.setFillColor(sf::Color(0, 0, 255, 50)); // 青の半透明
        mCanvasWidget->draw(mSelectionRect);
    }
}

sf::Vector2i EditorScreen::ScreenToGrid(const sf::Vector2f& screenPos) const
{
    // screenPosはメインウィンドウ上の絶対座標
    auto canvasPos = mCanvasWidget->getPosition() + mEditorWindow->getChildWidgetsOffset();

    // エディットウィンドウのタイトルバーを除いて左上を原点としたマウスの座標
    sf::Vector2f childWindowMousePos((float)screenPos.x - mEditorWindow->getPosition().x, (float)screenPos.y - mEditorWindow->getPosition().y);

    // ローカル座標へ変換
    sf::Vector2f localPos = childWindowMousePos - sf::Vector2f{ canvasPos.x, canvasPos.y };

    // グリッド座標へ変換
    int gridX = static_cast<int>(std::floor(localPos.x / mCanvasTileSize));
    int gridY = static_cast<int>(std::floor(localPos.y / mCanvasTileSize));

    // 範囲チェック
    if (gridX >= 0 && gridX < mEditorCanvasSize.x && gridY >= 0 && gridY < mEditorCanvasSize.y)
    {
        return { gridX, gridY };
    }
    return { -1, -1 };
}

void EditorScreen::PlaceTile(const sf::Vector2i& gridPos, bool isContinuous)
{
    char currentChar = mEditorBoardData[gridPos.y][gridPos.x];
    SelectedTile targetType = mCurrentTile;
    char newChar = currentChar;

    // プレイヤー配置の特別ルール
    if (targetType == SelectedTile::Player)
    {
        // 連続配置（ドラッグ/キー長押し）の場合、
        // 「古い位置のプレイヤーを消して、新しい位置に置く」ことで「追従」を表現する
        if (isContinuous)
        {
            // 既にそこにプレイヤーがいたら何もしない（無駄な更新を防ぐ）
            if (currentChar == '@' || currentChar == '+') return;

            // 全盤面からプレイヤーを消去（移動前の残像を消す）
            for (int y = 0; y < mEditorCanvasSize.y; ++y)
            {
                for (int x = 0; x < mEditorCanvasSize.x; ++x)
                {
                    if (mEditorBoardData[y][x] == '@') mEditorBoardData[y][x] = ' ';
                    if (mEditorBoardData[y][x] == '+') mEditorBoardData[y][x] = '.';
                }
            }
            // 新しい文字を決定するための準備（currentCharは古いままなので注意）
            // 消去処理でcurrentCharの場所が変わった可能性があるので再取得
            currentChar = mEditorBoardData[gridPos.y][gridPos.x];
        }
        else
        {
            // 単発モード: トグル動作
            // (既存ロジック: 既にプレイヤーがいれば消すだけの操作になる)
            if (currentChar == '@' || currentChar == '+')
            {
                // 既にいる場所をクリック -> 消去のみ
                // (前処理で消すので、newCharを床/ゴールに戻す)
                if (currentChar == '@') newChar = ' ';
                if (currentChar == '+') newChar = '.';
                
                // ※重要: ここでreturnせず、下部の盤面更新まで通す
            }
            else
            {
                // いない場所にクリック -> 他を消してここに置く
                for (int y = 0; y < mEditorCanvasSize.y; ++y)
                {
                    for (int x = 0; x < mEditorCanvasSize.x; ++x)
                    {
                        if (mEditorBoardData[y][x] == '@') mEditorBoardData[y][x] = ' ';
                        if (mEditorBoardData[y][x] == '+') mEditorBoardData[y][x] = '.';
                    }
                }
                currentChar = mEditorBoardData[gridPos.y][gridPos.x];
            }
        }
    }

    // 配置ロジック
    switch (targetType)
    {
    case SelectedTile::Wall: newChar = '#'; break;
    case SelectedTile::Floor:
        if (currentChar == '#' || currentChar == '.') newChar = ' ';
        else if (currentChar == '*') newChar = '$';
        else if (currentChar == '+') newChar = '@';
        break;
    case SelectedTile::Goal:
        if (currentChar == '#' || currentChar == ' ') newChar = '.';
        else if (currentChar == '$') newChar = '*';
        else if (currentChar == '@') newChar = '+';
        break;

    case SelectedTile::Baggage:
        // 連続配置の場合、「既に荷物があるなら何もしない（解除しない）」
        if (isContinuous)
        {
            if (currentChar == '$' || currentChar == '*') return; // 点滅防止
        }
        else
        {
            // 単発クリックならトグル動作 (既存ロジック)
            if (currentChar == '$') newChar = ' ';
            else if (currentChar == '*') newChar = '.';
        }

        // 配置処理 (ここに来る = 置く または トグルで消す対象でない)
        // 単発で消す処理は上のifでnewChar設定済みなので、ここでは「置く」ケースを記述
        if (newChar == currentChar) // 上のトグルで変わってなければ「置く」
        {
            if (currentChar == '@') newChar = '$';
            else if (currentChar == '+') newChar = '*';
            else if (currentChar == ' ') newChar = '$';
            else if (currentChar == '.') newChar = '*';
            else if (currentChar == '#') newChar = '$';
        }
        break;

    case SelectedTile::Player:
        // 上部で処理済みだが、newCharの設定が必要
        if (newChar == currentChar) // 変更がまだなら「置く」
        {
            if (currentChar == ' ' || currentChar == '$') newChar = '@';
            else if (currentChar == '.' || currentChar == '*') newChar = '+';
            else if (currentChar == '#') newChar = '@';
        }
        break;

    case SelectedTile::Erase:
        if (currentChar == '#' || currentChar == '.') newChar = ' ';
        else if (currentChar == '*') newChar = '$';
        else if (currentChar == '+') newChar = '@';
        break;
    }

    if (mEditorBoardData[gridPos.y][gridPos.x] != newChar)
    {
        mEditorBoardData[gridPos.y][gridPos.x] = newChar;
        ResizeCanvasIfNeeded();
        UpdateValidationState();
    }
}

// 右上の情報パネルの更新
void EditorScreen::UpdateInfoPanel()
{
    // (フェーズ1)
    int baggageCount = 0;
    int goalCount = 0;
    int playerCount = 0;

    for (const auto& row : mEditorBoardData)
    {
        for (char c : row)
        {
            if (c == '$' || c == '*') baggageCount++;
            if (c == '.' || c == '*' || c == '+') goalCount++;
            if (c == '@' || c == '+') playerCount++;
        }
    }

    mInfoLabel->setText(mGame->GetLoc()->Get("EDITOR_RIGHT_PANEL_BAGGAGE") + std::to_string(baggageCount) + " / " + mGame->GetLoc()->Get("EDITOR_RIGHT_PANEL_GOAL") + std::to_string(goalCount));
}

void EditorScreen::ResizeCanvasIfNeeded()
{
    // 1. 現在配置されているオブジェクトの最小・最大座標を見つける
    int minX = mEditorCanvasSize.x;
    int minY = mEditorCanvasSize.y;
    int maxX = -1;
    int maxY = -1;

    bool isEmpty = true;

    for (int y = 0; y < mEditorCanvasSize.y; ++y)
    {
        for (int x = 0; x < mEditorCanvasSize.x; ++x)
        {
            char tile = mEditorBoardData[y][x];
            // 床以外（壁、ゴール、荷物、プレイヤー）がある場所を探す
            if (tile != ' ')
            {
                if (x < minX) minX = x;
                if (x > maxX) maxX = x;
                if (y < minY) minY = y;
                if (y > maxY) maxY = y;
                isEmpty = false;
            }
        }
    }

    // 何も置かれていない場合はデフォルトサイズ (例: 10x10) に戻す
    if (isEmpty)
    {
        // 変化がなければ何もしない
        if (mEditorCanvasSize.x == mCanvasSizeMin && mEditorCanvasSize.y == mCanvasSizeMin) return;

        mEditorCanvasSize = { mCanvasSizeMin, mCanvasSizeMin };
        mEditorBoardData.assign(static_cast<size_t>(mCanvasSizeMin), std::string(static_cast<size_t>(mCanvasSizeMin), ' '));
        // 描画情報の更新へ
    }
    else
    {
        // 2. 必要なサイズを計算 (外周 +1 マス)
        // 現在のオブジェクト範囲の幅と高さ
        int contentWidth = maxX - minX + 1;
        int contentHeight = maxY - minY + 1;

        // 必要なキャンバスサイズ (左右に +1 ずつで +2、上下に +1 ずつで +2)
        // かつ、縦横比を維持するため大きい方に合わせる（仕様）
        int newSize = std::max(contentWidth, contentHeight) + 2;

        // 最小サイズ制限 (例: 10未満にはしない)
        newSize = std::max(newSize, mCanvasSizeMin);

        // サイズ変更が必要か？
        if (newSize != mEditorCanvasSize.x || newSize != mEditorCanvasSize.y)
        {
            // 3. 新しい盤面データを作成して移行
            std::vector<std::string> newBoard(newSize, std::string(newSize, ' '));

            // オブジェクトを新しい盤面の中央に配置するためのオフセット
            // 新しい盤面の中心 - オブジェクト群の中心
            int offsetX = (newSize - contentWidth) / 2 - minX;
            int offsetY = (newSize - contentHeight) / 2 - minY;

            for (int y = minY; y <= maxY; ++y)
            {
                for (int x = minX; x <= maxX; ++x)
                {
                    newBoard[y + offsetY][x + offsetX] = mEditorBoardData[y][x];
                }
            }

            // データを更新
            mEditorBoardData = newBoard;
            mEditorCanvasSize = { newSize, newSize };

            // カーソル位置などがずれないように調整（必要なら）
            // 今回はカーソル位置のリセットが無難かもしれない
            // mSelectedGridCell = {-1, -1};
        }
    }

    // 4. ビューとウィジェットのサイズ・位置更新
    // 描画に使える最大領域
    float maxAvailableHeight = mEditorWindow->getSize().y * 0.9f;

    // 縦横、どちらの倍率が制限になるか計算
    float scaleX = maxAvailableHeight / static_cast<float>(mEditorCanvasSize.x);
    float scaleY = maxAvailableHeight / static_cast<float>(mEditorCanvasSize.y);

    // 小さい方の倍率に合わせてタイルサイズを決定
    mCanvasTileSize = std::min(scaleX, scaleY);

    // キャンバスウィジェットのサイズを「タイルサイズ × 個数」に設定
    mCanvasWidget->setSize(mCanvasTileSize * static_cast<float>(mEditorCanvasSize.x), mCanvasTileSize * static_cast<float>(mEditorCanvasSize.y));

    // 5. スプライトとハイライトのスケール更新
    float texW = static_cast<float>(mFloorSprite.getTexture()->getSize().x);
    float texH = static_cast<float>(mFloorSprite.getTexture()->getSize().y);

    // Y軸反転の対応はお好みで (正の値なら上から下、負なら下から上)
    // ここでは一般的な「上から下」描画に合わせて正の値にします
    float scale(mCanvasTileSize / texW);
    mFloorSprite.setScale(scale, -scale);
    mWallSprite.setScale(scale, -scale);
    mGoalSprite.setScale(scale, -scale);
    mBaggageSprite.setScale(scale, -scale);
    mShinyBaggageSprite.setScale(scale, -scale);
    mPlayerSprite.setScale(scale, -scale);

    mCursorHighlight.setSize({ mCanvasTileSize, mCanvasTileSize });
    mHoverHighlight.setSize({ mCanvasTileSize, mCanvasTileSize });
    mSelectedCellHighlight.setSize({ mCanvasTileSize, mCanvasTileSize });

    // グリッド線の太さがタイルサイズに対して太すぎないように調整
    float gridThickness = std::max(1.0f, mCanvasTileSize / 40.0f);
    mGridLines[0].setSize({ gridThickness, mCanvasTileSize });
    mGridLines[1].setSize({ mCanvasTileSize * mEditorCanvasSize.x, gridThickness });
}

void EditorScreen::FillSelection(SelectedTile tile)
{
    if (!mIsSelecting) return;

    // 範囲の正規化
    int startX = std::min(mDragStartPos.x, mDragEndPos.x);
    int startY = std::min(mDragStartPos.y, mDragEndPos.y);
    int endX = std::max(mDragStartPos.x, mDragEndPos.x);
    int endY = std::max(mDragStartPos.y, mDragEndPos.y);

    // 一時的に現在のタイルを保存して変更
    SelectedTile prevTile = mCurrentTile;
    mCurrentTile = tile;

    bool changed = false;
    for (int y = startY; y <= endY; ++y)
    {
        for (int x = startX; x <= endX; ++x)
        {
            // グリッド範囲内チェック
            if (x >= 0 && x < mEditorCanvasSize.x && y >= 0 && y < mEditorCanvasSize.y)
            {
                // PlaceTileを呼ぶと1マスごとにリサイズ判定が走って重くなる可能性があるため
                // ここでは直接データを書き換え、最後にまとめてリサイズ判定を行うのが効率的です
                char newChar = mTileToChar[mCurrentTile];
                if (mEditorBoardData[y][x] != newChar)
                {
                    mEditorBoardData[y][x] = newChar;
                    changed = true;
                }
            }
        }
    }

    mCurrentTile = prevTile; // 元に戻す

    if (changed)
    {
        ResizeCanvasIfNeeded(); // 最後に1回だけリサイズ判定
        UpdateInfoPanel();      // 情報更新
        UpdateValidationState();
    }
}

bool EditorScreen::CheckReachability()
{
    sf::Vector2i playerPos = { -1, -1 };
    int baggageCount = 0;

    // 1. プレイヤー位置と荷物数を取得
    for (int y = 0; y < mEditorCanvasSize.y; ++y)
    {
        for (int x = 0; x < mEditorCanvasSize.x; ++x)
        {
            char c = mEditorBoardData[y][x];
            if (c == '@' || c == '+') playerPos = { x, y };
            if (c == '$' || c == '*') baggageCount++;
        }
    }

    // プレイヤーがいない場合はNG
    if (playerPos.x == -1) return false;

    // 2. Flood Fill (幅優先探索) で到達可能領域を調べる
    // 同時に「外周（盤面外）へ到達可能か」も調べる
    std::vector<std::vector<bool>> visited(mEditorCanvasSize.y, std::vector<bool>(mEditorCanvasSize.x, false));
    std::vector<sf::Vector2i> queue;
    queue.push_back(playerPos);
    visited[playerPos.y][playerPos.x] = true;

    bool isEnclosed = true; // 壁で囲まれているか
    int reachableBaggages = 0;

    int head = 0;
    while (head < queue.size())
    {
        sf::Vector2i curr = queue[head++];

        // キャンバスの端に到達したら「囲まれていない」
        if (curr.x == 0 || curr.x == mEditorCanvasSize.x - 1 ||
            curr.y == 0 || curr.y == mEditorCanvasSize.y - 1)
        {
            isEnclosed = false;
        }

        // 4方向をチェック
        int dx[] = { 0, 0, 1, -1 };
        int dy[] = { 1, -1, 0, 0 };

        for (int i = 0; i < 4; ++i)
        {
            int nx = curr.x + dx[i];
            int ny = curr.y + dy[i];

            // 範囲内チェック
            if (nx >= 0 && nx < mEditorCanvasSize.x && ny >= 0 && ny < mEditorCanvasSize.y)
            {
                if (!visited[ny][nx])
                {
                    char c = mEditorBoardData[ny][nx];
                    // 壁以外なら移動可能とみなす（荷物も押せるので移動可能経路に含める）
                    // ※厳密には荷物は障害物だが、ここでは「荷物に触れるか」を見たいので通れるものとする
                    if (c != '#')
                    {
                        visited[ny][nx] = true;
                        queue.push_back({ nx, ny });

                        // 到達したマスに荷物があるかカウント
                        if (c == '$' || c == '*') reachableBaggages++;
                    }
                }
            }
        }
    }

    // 3. 結果の判定
    std::string statusMsg = "";
    bool isValid = true;

    // 条件1: 完全に壁で囲まれているか
    if (!isEnclosed)
    {
        statusMsg += mGame->GetLoc()->Get("EDITOR_RIGHT_PANEL_NOT_ENCLOSED_STATE") + "\n";
        isValid = false;
    }

    // 条件2: すべての荷物が到達可能か
    // ※ゴール上の荷物('*')も到達不能なら動かせないのでNGとする（詰み防止のため）
    // ただし仕様では「ゴールに配置されていない荷物が到達不能」とあるので調整
    int unreachableNonGoaled = 0;
    for (int y = 0; y < mEditorCanvasSize.y; ++y)
    {
        for (int x = 0; x < mEditorCanvasSize.x; ++x)
        {
            char c = mEditorBoardData[y][x];
            if (c == '$' && !visited[y][x]) // ゴールに乗っていない荷物、かつ未訪問
            {
                unreachableNonGoaled++;
            }
        }
    }

    if (unreachableNonGoaled > 0)
    {
        statusMsg += mGame->GetLoc()->Get("EDITOR_RIGHT_PANEL_ERROR") + std::to_string(unreachableNonGoaled) + " " + mGame->GetLoc()->Get("EDITOR_RIGHT_PANEL_UNREACHABLE_STATE") + "\n";
        isValid = false;
    }

    return isValid;
}

void EditorScreen::UpdateValidationState()
{
    // 基本的な数のチェック
    int baggageCount = 0;
    int goalCount = 0;
    int playerCount = 0;
    for (const auto& row : mEditorBoardData) {
        for (char c : row) {
            if (c == '$' || c == '*') baggageCount++;
            if (c == '.' || c == '*' || c == '+') goalCount++;
            if (c == '@' || c == '+') playerCount++;
        }
    }
    std::string errorMsg = "";
    mIsValidLevel = true;

    // 2. 基本ルールのチェック
    if (baggageCount != goalCount)
    {
        errorMsg += mGame->GetLoc()->Get("EDITOR_RIGHT_PANEL_COUNT_MISMATCH_STATE") + "\n";
        mIsValidLevel = false;
    }
    if (playerCount != 1)
    {
        errorMsg += mGame->GetLoc()->Get("EDITOR_RIGHT_PANEL_NON_PLAYER_STATE") + "\n";
        mIsValidLevel = false;
    }
    if (baggageCount == 0)
    {
        errorMsg += mGame->GetLoc()->Get("EDITOR_RIGHT_PANEL_NON_BAGGAGE_STATE") + "\n";
        mIsValidLevel = false;
    }

    // 3. 基本ルールが通っている場合のみ、高度な到達判定を行う
    if (mIsValidLevel)
    {
        // ※CheckReachabilityの中でisEnclosed判定も行っているため呼び出す
        if (!CheckReachability())
        {
            // CheckReachability内で発生したエラーメッセージを取得する仕組みが必要
            // 現状はCheckReachabilityの戻り値だけで判定
            errorMsg += mGame->GetLoc()->Get("EDITOR_RIGHT_PANEL_NOT_ENCLOSED_STATE") + "\n";
            mIsValidLevel = false;
        }
    }

    // 全ての判定をクリア(isValid == true)した場合のみボタンを有効化
    mSaveButton->setEnabled(mIsValidLevel);
    mApplyButton->setEnabled(mIsValidLevel);

    // 4. 最終的な表示更新
    if (mIsValidLevel)
    {
        mInfoBaggageGoaledLabel->setText(mGame->GetLoc()->Get("EDITOR_RIGHT_PANEL_STATUS") + mGame->GetLoc()->Get("EDITOR_RIGHT_PANEL_OK_STATE"));
        mInfoBaggageGoaledLabel->getRenderer()->setTextColor(sf::Color::Green);
    }
    else
    {
        mInfoBaggageGoaledLabel->setText(mGame->GetLoc()->Get("EDITOR_RIGHT_PANEL_STATUS") + "\n" + errorMsg);
        mInfoBaggageGoaledLabel->getRenderer()->setTextColor(sf::Color::Red);
    }
}

void EditorScreen::PerformSave(bool closeAfterSave)
{
    if (!mIsValidLevel) return;

    // 余計な床パネルを切り詰めた盤面データを取得
    std::vector<std::string> trimmedBoard = GetTrimmedBoardData();

    // メモリ上のリストに追加（既存処理）
    std::string newLevelName = "Edited_" + mGame->GetDateTime();
    mGame->AddBoard(newLevelName, trimmedBoard);
    mLastSavedData = mEditorBoardData; // セーブ済み状態に更新

    // ファイルダイアログを開いて保存
    mGame->CallSave(trimmedBoard);

    if (closeAfterSave)
    {
        Close();
    }
    else
    {
        // メッセージボックスで保存完了通知（CallSaveがキャンセルされた場合でも出るが、仕様上許容）
        // CallSave内でキャンセルされたかを判別したい場合はCallSaveの戻り値をboolにする必要がありますが
        // ここでは「保存フローが終わった」ことの通知とします。
        mIsDialogOpen = true;
        auto msg = tgui::MessageBox::create(mGame->GetLoc()->Get("EDITOR_DIALOG_LBL_INFO"), mGame->GetLoc()->Get("EDITOR_DIALOG_LBL_TEXT"), { mGame->GetLoc()->Get("BTN_OK") });
        msg->setRenderer(mTheme->getRenderer("MessageBox"));
        msg->setPosition("(&.width - width) / 2", "(&.height - height) / 2");
        msg->onButtonPress([this, msg]() {
            mEditorWindow->remove(msg);
            mIsDialogOpen = false;
            });
        mEditorWindow->add(msg);
    }
}

void EditorScreen::PerformApply()
{
    if (!mIsValidLevel) return;

    // 余計な床パネルを切り詰めた盤面データを取得
    std::vector<std::string> trimmedBoard = GetTrimmedBoardData();

    // 現在の盤面を適用してゲーム再開
    mGame->ApplyEditedBoard(trimmedBoard);

    // エディタを閉じる
    Close();
}

void EditorScreen::CheckUnsavedChangesAndClose()
{
    // 変更があるかチェック
    if (mEditorBoardData == mLastSavedData)
    {
        Close(); // 変更なしなら即終了
        return;
    }

    // 確認ダイアログ表示
    mIsDialogOpen = true;
    auto msgBox = tgui::MessageBox::create(mGame->GetLoc()->Get("EDITOR_EXIT_DIALOG_TITLE"),
        mGame->GetLoc()->Get("EDITOR_EXIT_DIALOG_TEXT"),
        { mGame->GetLoc()->Get("EDITOR_EXIT_DIALOG_BTN_SAVE_EXIT"), mGame->GetLoc()->Get("EDITOR_EXIT_DIALOG_BTN_DISCARD"), mGame->GetLoc()->Get("BTN_CANCEL") });
    msgBox->setRenderer(mTheme->getRenderer("MessageBox"));
    msgBox->setPosition("(&.width - width) / 2", "(&.height - height) / 2");

    msgBox->onButtonPress([this, msgBox](const tgui::String& button) {
        if (button == mGame->GetLoc()->Get("EDITOR_EXIT_DIALOG_BTN_SAVE_EXIT"))
        {
            if (mIsValidLevel)
            {
                // セーブ後はエディット画面に戻るのでfalse
                PerformSave(false);
            }
            else
            {
                // セーブできない状態なら警告
                mIsDialogOpen = true;
                auto errBox = tgui::MessageBox::create("Error", "Cannot save invalid level!", { "OK" });
                errBox->setRenderer(mTheme->getRenderer("MessageBox"));
                errBox->setPosition("(&.width - width) / 2", "(&.height - height) / 2");
                mEditorWindow->add(errBox);
                mEditorWindow->remove(msgBox); // 元のダイアログは消す
                errBox->onButtonPress([this, errBox]() {
                    mEditorWindow->remove(errBox);
                    mIsDialogOpen = false;
                    });
            }
        }
        else if (button == mGame->GetLoc()->Get("EDITOR_EXIT_DIALOG_BTN_DISCARD"))
        {
            Close(); // 破棄して終了
        }
        else
        {
            mEditorWindow->remove(msgBox); // キャンセル
            mIsDialogOpen = false;
        }
        });

    mEditorWindow->add(msgBox);
}