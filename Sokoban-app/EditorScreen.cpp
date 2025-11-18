#include "EditorScreen.h"
#include "Game.h"
#include "SFML/Graphics.hpp"

// コンストラクタ
EditorScreen::EditorScreen(Game* game, sf::RenderWindow* window, const bool editCurrentBoard)
    : mGame(game),
    mState(IUIScreen::UIState::EActive),
    mGui(std::make_unique<tgui::Gui>(*window)),
    mTheme(std::make_unique<tgui::Theme>("Assets/themes/Black.txt")),
    mEditorCanvasSize(10, 10), // 30x30 の固定サイズキャンバス
    mCurrentTile(SelectedTile::Wall),
    mSelectedGridCell(-1, -1),
    mHoverGridCell(-1, -1),
    mIsMouseMode(true)
{
    mGame->SetState(Game::GameState::EEditMode);
    mGame->PushUI(this);

    // タイルと文字のマッピング
    mTileToChar[SelectedTile::Floor] = ' ';
    mTileToChar[SelectedTile::Wall] = '#';
    mTileToChar[SelectedTile::Goal] = '.';
    mTileToChar[SelectedTile::Baggage] = '$';
    mTileToChar[SelectedTile::Player] = '@';
    mTileToChar[SelectedTile::Erase] = ' ';

    // 盤面データを初期化 (すべて床 ' ' で埋める)
    mEditorBoardData.assign(mEditorCanvasSize.y, std::string(mEditorCanvasSize.x, ' '));

    if (editCurrentBoard)
    {
        // TODO: 現在の盤面を mEditorBoardData にロードする処理
        // (Gameから mInitBoardData を取得し、mEditorBoardData にコピーする)
    }

    // --- 描画スプライトとキャンバス設定 ---
    mFloorSprite.setTexture(*mGame->GetTexture("Assets/Floor.png"));
    mWallSprite.setTexture(*mGame->GetTexture("Assets/Wall.png"));
    mGoalSprite.setTexture(*mGame->GetTexture("Assets/Goal.png"));
    mBaggageSprite.setTexture(*mGame->GetTexture("Assets/Box.png")); // Box.png?
    mPlayerSprite.setTexture(*mGame->GetTexture("Assets/playerS.png")); // playerS.png?

    // 1. エディタ用の子ウィンドウを作成
    mEditorWindow = tgui::ChildWindow::create();
    mEditorWindow->setRenderer(mTheme->getRenderer("ChildWindow"));
    mEditorWindow->setTitle("Level Editor");
    mEditorWindow->setSize(mGame->GetWindowSize().x * 0.95f, mGame->GetWindowSize().y * 0.95f); // 画面いっぱいに
    mEditorWindow->setPosition((mGame->GetWindowSize().x - mEditorWindow->getSize().x) / 2.0f, (mGame->GetWindowSize().y - mEditorWindow->getSize().y) / 2.0f);
    //mEditorWindow->setKeepInParent(true);
    mGui->add(mEditorWindow);

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
    mPlayerSprite.setScale(mCanvasTileSize / mPlayerSprite.getLocalBounds().width, -mCanvasTileSize / mPlayerSprite.getLocalBounds().height);

    // グリッドとハイライトの初期設定
    mGridLines[0].setFillColor(sf::Color(100, 100, 100)); // 縦線
    mGridLines[1].setFillColor(sf::Color(100, 100, 100)); // 横線
    mSelectedCellHighlight.setSize({ mCanvasTileSize, mCanvasTileSize });
    mSelectedCellHighlight.setFillColor(sf::Color(255, 255, 0, 100)); // 黄色半透明

    // TGUIウィジェットの配置 (フェーズ1)
    // 左下のタイルパレット
    mTilePalettePanel = tgui::Panel::create();
    mTilePalettePanel->setRenderer(mTheme->getRenderer("Panel"));
    mTilePalettePanel->setSize(200, 300);
    mTilePalettePanel->setPosition(10, 10);
    mEditorWindow->add(mTilePalettePanel);

    // パレット内のボタン
    auto btnWall = tgui::Button::create("Wall");
    btnWall->setPosition(10, 10);
    btnWall->onPress([this]() { mCurrentTile = SelectedTile::Wall; });
    mTilePalettePanel->add(btnWall);

    auto btnFloor = tgui::Button::create("Floor");
    btnFloor->setPosition(10, 50);
    btnFloor->onPress([this]() { mCurrentTile = SelectedTile::Floor; });
    mTilePalettePanel->add(btnFloor);

    // ... (Goal, Baggage, Player, Erase ボタンも同様に追加) ...
    auto btnErase = tgui::Button::create("Erase");
    btnErase->setPosition(10, 250);
    btnErase->onPress([this]() { mCurrentTile = SelectedTile::Erase; });
    mTilePalettePanel->add(btnErase);

    // 右下の情報パネル
    mInfoPanel = tgui::Panel::create();
    mInfoPanel->setRenderer(mTheme->getRenderer("Panel"));
    mInfoPanel->setSize(300, 150);
    mInfoPanel->setPosition(mEditorWindow->getSize().x - 310, mEditorWindow->getSize().y - 160);
    mEditorWindow->add(mInfoPanel);

    mInfoLabel = tgui::Label::create("Baggage: 0 / Goal: 0");
    mInfoLabel->setPosition(10, 10);
    mInfoPanel->add(mInfoLabel);

    mInfoBaggageGoaledLabel = tgui::Label::create("Status: OK");
    mInfoBaggageGoaledLabel->setPosition(10, 40);
    mInfoPanel->add(mInfoBaggageGoaledLabel);

    // 右下のボタン
    mSaveButton = tgui::Button::create("Save Level");
    mSaveButton->setPosition(mEditorWindow->getSize().x - 310, mEditorWindow->getSize().y - 60);
    mSaveButton->onPress([this]() { TrySaveLevel(); });
    mEditorWindow->add(mSaveButton);

    mExitButton = tgui::Button::create("Exit Editor");
    mExitButton->setPosition(mEditorWindow->getSize().x - 160, mEditorWindow->getSize().y - 60);
    mExitButton->onPress([this]() { Close(); });
    mEditorWindow->add(mExitButton);

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


    // --- ヘルプボタンの追加 ---
    mHelpButton = tgui::Button::create("Help");
    mHelpButton->setPosition(mEditorWindow->getSize().x - 160, mEditorWindow->getSize().x - 110); // Exitボタンの上あたり
    mHelpButton->setSize(140, 30);
    mHelpButton->onPress([this]() {
        // メッセージボックスで操作説明を表示
        auto msg = tgui::MessageBox::create("Editor Controls",
            "Mouse:\n"
            "  Left Click: Place Tile\n"
            "  Right Click: Erase Tile\n\n"
            "Keyboard:\n"
            "  WASD / Arrows: Move Cursor\n"
            "  Space / Enter: Place Tile\n"
            "  Tab: Switch Tile Type",
            { "OK" });
        msg->setPosition("(&.width - width) / 2", "(&.height - height) / 2");
        mEditorWindow->add(msg);
        });
    mEditorWindow->add(mHelpButton);
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
    // 情報パネルを更新
    UpdateInfoPanel();
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
    }

    // マウス操作 (配置)
    if (gridPos.x != -1)
    {
        // クリックされたら、選択カーソルもそこに移動させる
        if (event->type == sf::Event::MouseButtonPressed)
        {
            mSelectedGridCell = gridPos; // カーソル移動
            mIsMouseMode = true;         // マウスモードへ

            if (event->mouseButton.button == sf::Mouse::Left)
            {
                PlaceTile(gridPos);
            }
            else if (event->mouseButton.button == sf::Mouse::Right)
            {
                // 一時的に消去モードで配置（元のmCurrentTileは変えない）
                SelectedTile prevTile = mCurrentTile;
                mCurrentTile = SelectedTile::Erase;
                PlaceTile(gridPos);
                mCurrentTile = prevTile;
            }
        }
    }
    else
    {
        mHoverGridCell = { -1, -1 };
    }

    // --- キーボード操作 ---
    if (event->type == sf::Event::KeyPressed)
    {
        // キー入力があったら「キーボードモード」にする (ホバーを隠す)
        mIsMouseMode = false;

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
        }

        // パネル配置 (Space / Enter)
        if (event->key.code == sf::Keyboard::Space || event->key.code == sf::Keyboard::Enter)
        {
            PlaceTile(mSelectedGridCell);
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
            // Enumをintにキャストしてインクリメントし、循環させる
            int nextTileVal = static_cast<int>(mCurrentTile) + 1;
            if (nextTileVal > static_cast<int>(SelectedTile::Erase)) // Eraseの次はFloor(0)へ
            {
                nextTileVal = 0;
            }
            mCurrentTile = static_cast<SelectedTile>(nextTileVal);

            // TODO: パレットUIの見た目も同期させるなら、ここでボタンの状態更新が必要
            // (フェーズ2の仕上げとして実装すると良いです)
        }

        // クイックセーブ (Ctrl + S)
        if (event->key.control && event->key.code == sf::Keyboard::S)
        {
            TrySaveLevel();
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

            switch (tile)
            {
            case '#': spriteToDraw = &mWallSprite; break;
            case '.': spriteToDraw = &mGoalSprite; break;
            case '$': spriteToDraw = &mBaggageSprite; break;
            case '@': spriteToDraw = &mPlayerSprite; break;
                // '+' や '*' (荷物+ゴールなど) の描画も必要
            }

            // 床を描画
            mFloorSprite.setPosition(currentPos);
            mCanvasWidget->draw(mFloorSprite);

            if (tile != ' ')
            {
                spriteToDraw->setPosition(currentPos);
                mCanvasWidget->draw(*spriteToDraw);
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

void EditorScreen::PlaceTile(const sf::Vector2i& gridPos)
{
    char newChar = mTileToChar[mCurrentTile];

    // TODO: プレイヤー('@') や荷物('$') を置く前に、
    // 既に盤面にあるものを ' ' (床) に変える処理が必要

    mEditorBoardData[gridPos.y][gridPos.x] = newChar;
}

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

    mInfoLabel->setText("Baggage: " + std::to_string(baggageCount) + " / Goal: " + std::to_string(goalCount));

    // バリデーション
    std::string status = "Status: ";
    if (baggageCount != goalCount) status += "Baggage/Goal count mismatch! ";
    if (playerCount != 1) status += "Player count must be exactly 1! ";

    if (status == "Status: ")
    {
        status += "OK";
        mSaveButton->setEnabled(true);
    }
    else
    {
        mSaveButton->setEnabled(false);
    }
    mInfoBaggageGoaledLabel->setText(status);
}

void EditorScreen::TrySaveLevel()
{
    if (!mSaveButton->isEnabled())
    {
        // 多重チェック (基本的には UpdateInfoPanel で無効化されているはず)
        return;
    }

    // TODO: バリデーション (フェーズ3: 壁で囲まれているかチェック)

    // Gameクラスに新しい盤面として追加
    std::string newLevelName = "Edited_Level_" + mGame->GetDateTime();
    mGame->AddBoard(newLevelName, mEditorBoardData);

    // メッセージボックスで保存完了を通知
    auto msgBox = tgui::MessageBox::create("Save Complete", "Level saved as: \n" + newLevelName, { "OK" });
    msgBox->setPosition("(&.width - width) / 2", "(&.height - height) / 2");
    mGui->add(msgBox);

    // エディタを閉じる
    Close();
}