#include "SnapshotScreen.h"
#include "Game.h"

#include "SnapshotDialog.h"

#include <iostream>

SnapshotScreen::SnapshotScreen(Game* game, sf::RenderWindow* window)
    : mGame(game),
    mWindow(window),
    mState(IUIScreen::UIState::EActive),
    mGui(std::make_unique<tgui::Gui>(*window)),
    mTheme(std::make_unique<tgui::Theme>("Assets/themes/Black.txt")),
    mSelectedIndex(-1),
    mChildSnapshotDialog(nullptr),
    mIsSavingCurrentSnapshot(false)
{
    mGame->SetState(Game::GameState::EPaused);
    mGame->PushUI(this);

    // ウィンドウ全体を覆うモーダルウィンドウを作成
    mChildWindow = tgui::ChildWindow::create();
    mChildWindow->setRenderer(mTheme->getRenderer("ChildWindow"));
    // 画面の90%を使い、中央に配置
    mChildWindow->setSize("90%", "90%");
    mChildWindow->setPosition("(&.width - width) / 2", "(&.height - height) / 2");
    mChildWindow->setTitle("Snapshot Manager");
    mGui->add(mChildWindow);

    // UIレイアウト
    // 子ウィンドウのサイズ
    sf::Vector2f childWindowSize{ mChildWindow->getSize().x, mChildWindow->getSize().y };
    // スナップショットのリストをサムネイル形式で全て表示
    //　スナップショットをゲームクラスから取得
    auto snapshots = mGame->GetSnapshots();

    // サムネイルのリストのウィジェットの設定
    mThumbnailListBox = tgui::PanelListBox::create();
    mThumbnailListBox->setRenderer(mTheme->getRenderer("PanelListBox"));
    mThumbnailListBox->setSize(childWindowSize.x * 0.4f, childWindowSize.y * 0.85f);
    mThumbnailListBox->setPosition(0, 0);
    mThumbnailListBox->setItemsHeight(mThumbnailListBox->getSize().y * 0.3f);

    // 盤面のサムネイルの描画範囲
    std::pair<sf::Vector2f, sf::Vector2f> imageBoundingBox{ sf::Vector2f{ 0.0f, 0.0f }, sf::Vector2f{ mThumbnailListBox->getItemsHeight().getValue(), mThumbnailListBox->getItemsHeight().getValue() } };
    // テキストサイズ
    int textSize(16);

    // スナップショットが1つ以上あるならば、スナップショットのインデックスをもっとも上のものに設定しておく
    if (!snapshots.empty())
    {
        // インデックスは0から始まる
        mSelectedIndex = 0;

        // スナップショットのタイトルとコメントの描画領域のサイズ
        sf::Vector2f thumbnailTitleSize{ (mThumbnailListBox->getItemsWidth().getValue() - (imageBoundingBox.second.x - imageBoundingBox.first.x)) * 0.95f, static_cast<float>(textSize) * 1.25f };
        sf::Vector2f thumbnailCommentSize{ thumbnailTitleSize.x, (imageBoundingBox.second.y - imageBoundingBox.first.y) * 0.95f - thumbnailTitleSize.y };

        // スナップショットのタイトルとコメントの初期位置
        sf::Vector2f thumbnailTitleOffset{ imageBoundingBox.second.x + (mThumbnailListBox->getItemsWidth().getValue() - imageBoundingBox.second.x - thumbnailTitleSize.x) / 2.0f, imageBoundingBox.first.y + (mThumbnailListBox->getItemsHeight().getValue() - thumbnailTitleSize.y - thumbnailCommentSize.y) / 2.0f };
        sf::Vector2f thumbnailCommentOffset{ thumbnailTitleOffset.x, thumbnailTitleOffset.y + thumbnailTitleSize.y };

        // もしスナップショットのリストがPanelListBoxの描画領域に収まらない
        // (スクロールバーが表示される) 場合、タイトルとコメントの描画領域のサイズを調整
        if ((imageBoundingBox.second.y - imageBoundingBox.first.y) * snapshots.size() > mThumbnailListBox->getSize().y)
        {
            float scrollbarWidth(mThumbnailListBox->getHorizontalScrollbar()->getWidth());
            thumbnailTitleSize.x -= scrollbarWidth;
            thumbnailCommentSize.x -= scrollbarWidth;
            thumbnailTitleOffset.x = imageBoundingBox.second.x + (mThumbnailListBox->getItemsWidth().getValue() - imageBoundingBox.second.x - thumbnailTitleSize.x - scrollbarWidth) / 2.0f;
            thumbnailCommentOffset.x = thumbnailTitleOffset.x;
        }

        for (int i = 0, size = static_cast<int>(snapshots.size()); i < size; ++i)
        {
            // sf::Textureからtgui::textureへの変換
            sf::Texture* currentBoardTex = mGame->GenerateThumbnail(snapshots[i]);
            mThumbnailTextures.emplace_back(currentBoardTex);
            sf::Image tmpImage = currentBoardTex->copyToImage();
            tgui::Texture tmpTGuiTex{};
            tmpTGuiTex.loadFromPixelData(tmpImage.getSize(), tmpImage.getPixelsPtr());
            tgui::Picture::Ptr templatePic = tgui::Picture::create(tmpTGuiTex);
            mBoardPictureMap.emplace(i, templatePic);
            // 画像のスケーリングとオフセットの設定
            float scaleFactor(0.0f);
            if ((imageBoundingBox.second.x - imageBoundingBox.first.x) / static_cast<float>(tmpImage.getSize().x) < (imageBoundingBox.second.y - imageBoundingBox.first.y) / static_cast<float>(tmpImage.getSize().y))
            {
                // 盤面が横長の場合
                scaleFactor = (imageBoundingBox.second.x - imageBoundingBox.first.x) / static_cast<float>(tmpImage.getSize().x);
                templatePic->setSize(tgui::Layout2d{ static_cast<float>(tmpImage.getSize().x) * scaleFactor, static_cast<float>(tmpImage.getSize().y) * scaleFactor });
                templatePic->setPosition(0.0f, imageBoundingBox.first.y + ((imageBoundingBox.second.y - imageBoundingBox.first.y) - templatePic->getSize().y) / 2.0f);
            }
            else
            {
                // 盤面が縦長の場合
                scaleFactor = (imageBoundingBox.second.y - imageBoundingBox.first.y) / static_cast<float>(tmpImage.getSize().y);
                templatePic->setSize(tgui::Layout2d{ static_cast<float>(tmpImage.getSize().x) * scaleFactor, static_cast<float>(tmpImage.getSize().y) * scaleFactor });
                templatePic->setPosition(imageBoundingBox.first.x + ((imageBoundingBox.second.x - imageBoundingBox.first.x) - templatePic->getSize().x) / 2.0f, 0.0f);
            }

            // テキストを表示
            // スナップショットの名前
            auto nameLabel = tgui::Label::create("Name: " + snapshots[i].snapshotName);
            nameLabel->setRenderer(mTheme->getRenderer("Label"));
            nameLabel->setSize(thumbnailTitleSize.x, thumbnailTitleSize.y);
            nameLabel->setPosition(thumbnailTitleOffset.x, thumbnailTitleOffset.y);
            nameLabel->setTextSize(textSize);
            nameLabel->getScrollbar()->setPolicy(tgui::Scrollbar::Policy::Never);
            // スナップショットのコメント
            auto commentBox = tgui::TextArea::create();
            mCommentTextAreaMap.emplace(commentBox, snapshots[i].snapshotName);
            commentBox->setRenderer(mTheme->getRenderer("TextArea"));
            commentBox->setSize(thumbnailCommentSize.x, thumbnailCommentSize.y);
            commentBox->setPosition(thumbnailCommentOffset.x, thumbnailCommentOffset.y);
            commentBox->setTextSize(textSize);
            commentBox->setDefaultText("Enter a comment for this snapshot.");
            commentBox->setText(snapshots[i].snapshotComment);
            // テキスト変更時に外部変数を更新
            commentBox->onTextChange([this, commentBox]() {
                mGame->SetSnapshotComment(mCommentTextAreaMap[commentBox], commentBox->getText().toStdString());
                });

            // サムネイルリストに追加
            auto templatePanel = mThumbnailListBox->addItem(snapshots[i].snapshotName, i);
            templatePanel->add(templatePic, snapshots[i].snapshotName);
            std::cout << "SnapshotScreen: Add name label -> " << nameLabel->getText() << std::endl;
            templatePanel->add(nameLabel, snapshots[i].snapshotName);
            std::cout << "SnapshotScreen: Add comment -> " << commentBox->getText() << std::endl;
            templatePanel->add(commentBox, snapshots[i].snapshotName);
        }
    }
    mChildWindow->add(mThumbnailListBox);

    // TODO 選択されたスナップショットの盤面情報を子ウィンドウ右側に表示する
    // スナップショットの情報全体を表示するパネルの設定
    mRightPanel = tgui::Panel::create();
    mRightPanel->setRenderer(mTheme->getRenderer("Panel"));
    mRightPanel->setSize(mChildWindow->getSize().x - mThumbnailListBox->getSize().x, mThumbnailListBox->getSize().y);
    mRightPanel->setPosition(mThumbnailListBox->getPosition().x + mThumbnailListBox->getSize().x, mThumbnailListBox->getPosition().y);
    mChildWindow->add(mRightPanel);

    // 盤面を表示するピクチャウィジェット
    mSelectedPreview = tgui::Picture::create();
    mSelectedPreview->setRenderer(mTheme->getRenderer("Picture"));
    // 画像のスケーリングとオフセットの設定
    float scaleFactor(0.0f);
    sf::Vector2f rightPanelPictureBoundSize{ mRightPanel->getSize().x, mRightPanel->getSize().y * 0.8f };
    auto thumbnailTextureSize(mGame->GetThumbnailSize());
    if (rightPanelPictureBoundSize.x / static_cast<float>(thumbnailTextureSize.x) < rightPanelPictureBoundSize.y / static_cast<float>(thumbnailTextureSize.y))
    {
        // 盤面が横長の場合
        scaleFactor = rightPanelPictureBoundSize.x / static_cast<float>(thumbnailTextureSize.x);
        mSelectedPreview->setSize(tgui::Layout2d{ static_cast<float>(thumbnailTextureSize.x) * scaleFactor, static_cast<float>(thumbnailTextureSize.y) * scaleFactor });
        mSelectedPreview->setPosition(0.0f, (rightPanelPictureBoundSize.y - mSelectedPreview->getSize().y) / 2.0f);
    }
    else
    {
        // 盤面が縦長の場合
        scaleFactor = rightPanelPictureBoundSize.y / static_cast<float>(thumbnailTextureSize.y);
        mSelectedPreview->setSize(tgui::Layout2d{ static_cast<float>(thumbnailTextureSize.x) * scaleFactor, static_cast<float>(thumbnailTextureSize.y) * scaleFactor });
        mSelectedPreview->setPosition((rightPanelPictureBoundSize.x - mSelectedPreview->getSize().x) / 2.0f, 0.0f);
    }
    mRightPanel->add(mSelectedPreview);

    // 情報表示用パネル (盤面プレビューの下に表示)
    mInfoPanel = tgui::Panel::create();
    mInfoPanel->setRenderer(mTheme->getRenderer("Panel"));
    mInfoPanel->setPosition(0.0f, mSelectedPreview->getSize().y);
    mInfoPanel->setSize(mRightPanel->getSize().x, mRightPanel->getSize().y - mSelectedPreview->getSize().y);
    mRightPanel->add(mInfoPanel);

    // mInfoPanel内部のウィジェットを作成
    float infoPanelHeight = mInfoPanel->getSize().y;
    float labelHeight = infoPanelHeight / 3.0f; // パネルの高さを3分割

    mInfoStepsLabel = tgui::Label::create("Steps: ");
    mInfoStepsLabel->setRenderer(mTheme->getRenderer("Label"));
    mInfoStepsLabel->setPosition(mInfoPanel->getSize().x * 0.025f, 0); // 1行目
    mInfoStepsLabel->setSize(mInfoPanel->getSize().x * 0.95, labelHeight);
    mInfoStepsLabel->setTextSize(textSize);
    mInfoStepsLabel->setVerticalAlignment(tgui::VerticalAlignment::Center);
    mInfoPanel->add(mInfoStepsLabel);

    mInfoGoaledLabel = tgui::Label::create("Goaled: ");
    mInfoGoaledLabel->setRenderer(mTheme->getRenderer("Label"));
    mInfoGoaledLabel->setPosition(mInfoPanel->getSize().x * 0.025f, labelHeight); // 2行目
    mInfoGoaledLabel->setSize(mInfoPanel->getSize().x * 0.95f, labelHeight);
    mInfoGoaledLabel->setTextSize(textSize);
    mInfoGoaledLabel->setVerticalAlignment(tgui::VerticalAlignment::Center);
    mInfoPanel->add(mInfoGoaledLabel);

    mInfoTimeLabel = tgui::Label::create("Saved: ");
    mInfoTimeLabel->setRenderer(mTheme->getRenderer("Label"));
    mInfoTimeLabel->setPosition(mInfoPanel->getSize().x * 0.025f, labelHeight * 2); // 3行目
    mInfoTimeLabel->setSize(mInfoPanel->getSize().x * 0.95f, labelHeight);
    mInfoTimeLabel->setTextSize(textSize);
    mInfoTimeLabel->setVerticalAlignment(tgui::VerticalAlignment::Center);
    mInfoPanel->add(mInfoTimeLabel);

    // 各種ボタンの追加
    // ウィンドウを閉じるボタン
    mCloseButton = tgui::Button::create("Close");
    mCloseButton->setRenderer(mTheme->getRenderer("Button"));
    mCloseButton->setSize(120, 30);
    mCloseButton->setPosition(childWindowSize.x - mCloseButton->getSize().x - 20, childWindowSize.y * 0.85f + (childWindowSize.y * 0.15f - mCloseButton->getSize().y) / 2.0f);
    mChildWindow->add(mCloseButton);

    // 選択されたスナップショットを削除するボタン
    mRemoveButton = tgui::Button::create("Remove");
    mRemoveButton->setRenderer(mTheme->getRenderer("Button"));
    mRemoveButton->setSize(120, 30);
    mRemoveButton->setPosition(mCloseButton->getPosition().x - mRemoveButton->getSize().x - 20, mCloseButton->getPosition().y);
    mChildWindow->add(mRemoveButton);

    // 選択されたスナップショットを適用するボタン
    mApplyButton = tgui::Button::create("Apply");
    mApplyButton->setRenderer(mTheme->getRenderer("Button"));
    mApplyButton->setSize(120, 30);
    mApplyButton->setPosition(mRemoveButton->getPosition().x - mApplyButton->getSize().x - 20, mCloseButton->getPosition().y);
    mChildWindow->add(mApplyButton);

    // --- 8. シグナル（イベント）の設定 ---
    // スナップショットのリストからどれか1つが選択された場合はインデックスを切り替え、右側のプレビューを更新
    mThumbnailListBox->onItemSelect([this]() {
        mSelectedIndex = mThumbnailListBox->getSelectedItemIndex();

        if (mSelectedIndex != -1)
        {
            // 1. プレビュー画像を更新 (既存のコード)
            auto originalTexture = mBoardPictureMap.at(mSelectedIndex)->getRenderer()->getTexture();
            mSelectedPreview->getRenderer()->setTexture(originalTexture);
            std::cout << "SnapshotScreen: Apply index[" << mSelectedIndex << "] RightPanel preview!" << std::endl;

            // 2. 該当のスナップショットデータを取得
            const auto& snapshot = mGame->GetSnapshots()[mSelectedIndex];

            // 3. 各ラベルのテキストを設定
            mInfoStepsLabel->setText("Steps: " + std::to_string(snapshot.stepCount));
            mInfoTimeLabel->setText("Saved: " + snapshot.timestamp);

            // 4. ゴール達成数を計算
            int goaledCount = 0;
            const auto& goalPos = mGame->GetGoalPos();
            for (const auto& bPos : snapshot.baggagePositions)
            {
                // ゴール座標リストに荷物の座標が含まれているかチェック
                if (std::find(goalPos.begin(), goalPos.end(), bPos) != goalPos.end())
                {
                    goaledCount++;
                }
            }
            mInfoGoaledLabel->setText("Goaled: " + std::to_string(goaledCount) + " / " + std::to_string(goalPos.size()));
        }
        else
        {
            // 選択が外れたらテクスチャとテキストをリセット
            ReleasePreview();
            std::cout << "SnapshotScreen: Apply empty RightPanel preview!" << std::endl;
        }

        });

    // 子ウィンドウを閉じるボタンが押されるか、子ウィンドウが消された場合は本インスタンスを破棄
    mCloseButton->onPress([this]() { this->Close(); });
    mChildWindow->onClose([this]() { this->Close(); });

    // 選択されたスナップショットを削除するボタンが押された場合、ダイアログでプレイヤーに確認を行ってからスナップショットを削除する
    mRemoveButton->onPress([this]() {
        int mSelectedIndex(mThumbnailListBox->getSelectedItemIndex());
        if (mSelectedIndex == -1) return; // 何も選択されていなければ何もしない

        // --- 1. 確認ダイアログを作成 ---
        auto messageBox = tgui::MessageBox::create();
        messageBox->setRenderer(mTheme->getRenderer("MessageBox"));
        messageBox->setTitle("Confirm Deletion");
        messageBox->setText("Are you sure you want to remove this snapshot?\n\n'" + mGame->GetSnapshots()[mSelectedIndex].snapshotName + "'");
        messageBox->addButton("Yes");
        messageBox->addButton("No");
        messageBox->setPosition("(&.width - width) / 2", "(&.height - height) / 2");

        // ダイアログ以外のウィジェットを無効化する
        mThumbnailListBox->setEnabled(false);
        mCloseButton->setEnabled(false);
        mRemoveButton->setEnabled(false);
        mApplyButton->setEnabled(false);

        // --- 2. ダイアログのボタンが押された時の処理 ---
        messageBox->onButtonPress([this, mSelectedIndex, messageBox](const tgui::String& button) {
            // ウィジェットの無効化を解除
            mThumbnailListBox->setEnabled(true);
            mCloseButton->setEnabled(true);
            mRemoveButton->setEnabled(true);
            mApplyButton->setEnabled(true);

            if (button == "Yes")
            {
                // --- 3. 削除処理の実行 ---
                // a. Game本体からスナップショットデータを削除
                mGame->RemoveSnapshot(mSelectedIndex);
                std::cout << "SnapshotScreen: Removed snapshot in Game class!" << std::endl;

                // b. サムネイルテクスチャを解放 (メモリリーク防止)
                auto tmpTexture = mThumbnailTextures[mSelectedIndex];
                mThumbnailTextures.erase(mThumbnailTextures.begin() + mSelectedIndex);
                delete tmpTexture;
                std::cout << "SnapshotScreen: Removed snapshot texture!" << std::endl;

                // c. TGUIのリストから項目を削除
                mThumbnailListBox->removeItemByIndex(mSelectedIndex);

                // --- 削除された項目より後のスナップショットのインデックスを詰める ---
                std::map<int, tgui::Picture::Ptr> newBoardPictureMap;
                for (auto& [idx, pic] : mBoardPictureMap)
                {
                    if (idx < mSelectedIndex)
                    {
                        newBoardPictureMap[idx] = pic; // そのまま
                    }
                    else if (idx > mSelectedIndex)
                    {
                        newBoardPictureMap[idx - 1] = pic; // 1つ前に詰める
                    }
                }
                mBoardPictureMap.swap(newBoardPictureMap);
                std::cout << "SnapshotScreen: Swaped board picture map!" << std::endl;

                // コメントマップなど、indexと対応する他の構造体も同様に更新
                std::map<tgui::TextArea::Ptr, std::string> newCommentTextAreaMap;
                int newIndex = 0;
                for (int i = 0; i < mThumbnailListBox->getItemCount(); ++i)
                {
                    const auto& snapshotName = mGame->GetSnapshots()[i].snapshotName;
                    for (auto& [commentBox, name] : mCommentTextAreaMap)
                    {
                        if (name == snapshotName)
                        {
                            newCommentTextAreaMap[commentBox] = name;
                            newIndex++;
                            break;
                        }
                    }
                }
                mCommentTextAreaMap.swap(newCommentTextAreaMap);
                std::cout << "SnapshotScreen: Swaped comment text area map!" << std::endl;

                // --- 選択状態をリセット ---
                this->mSelectedIndex = -1;
                mThumbnailListBox->setSelectedItemByIndex(-1);
                std::cout << "SnapshotScreen: Removed snapshot in SnapshotScreen class!" << std::endl;
            }

            // YesボタンでもNoボタンでもダイアログを消去
            mGui->remove(messageBox);
            });

        // --- 4. ダイアログを表示 ---
        // mChildWindowに追加するのではなく、mGuiのトップレベルに追加
        mGui->add(messageBox);
        });

    // 選択されたスナップショットを適用するボタンが押された場合、ダイアログでプレイヤーに現在の盤面のスナップショットを保存するか確認を行ってから適用処理を行う
    mApplyButton->onPress([this]() {
        if (mSelectedIndex == -1) return; // 何も選択されていなければ何もしない

        // --- 1. 確認ダイアログを作成 ---
        auto messageBox = tgui::MessageBox::create();
        messageBox->setRenderer(mTheme->getRenderer("MessageBox"));
        messageBox->setTitle("Confirm Restore");
        messageBox->setText("Save current progress as a new snapshot before restoring?\n\n'" + mGame->GetSnapshots()[mSelectedIndex].snapshotName + "'");
        messageBox->addButton("Save & Restore");
        messageBox->addButton("Restore Only");
        messageBox->addButton("Cancel");
        messageBox->setPosition("(&.width - width) / 2", "(&.height - height) / 2");

        // ダイアログ以外のウィジェットを無効化する
        mThumbnailListBox->setEnabled(false);
        mCloseButton->setEnabled(false);
        mRemoveButton->setEnabled(false);
        mApplyButton->setEnabled(false);

        // --- 2. ダイアログのボタンが押された時の処理 ---
        messageBox->onButtonPress([this, messageBox](const tgui::String& button) {
            if (button == "Save & Restore")
            {
                // --- 3. 現在の盤面のスナップショットを追加する処理の実行 ---
                // a. Game本体からスナップショット追加のダイアログを開く
                mChildSnapshotDialog = new SnapshotDialog(mGame, mWindow);
                mChildSnapshotDialog->SetParentManager(this);

                // b. 現在の盤面のスナップショットを保存中の状態に遷移
                mIsSavingCurrentSnapshot = true;
            }
            else if (button == "Restore Only")
            {
                // 「適用のみ」
                // 1. Gameのスナップショット復元機能を呼び出す
                mGame->RestoreSnapshot(mSelectedIndex);

                // 2. SnapshotScreenを閉じる
                this->Close();
            }
            else if (button == "Cancel")
            {
                // ダイアログを消去
                mGui->remove(messageBox);
                messageBox->close();

                // ウィジェットの無効化を解除
                mThumbnailListBox->setEnabled(true);
                mCloseButton->setEnabled(true);
                mRemoveButton->setEnabled(true);
                mApplyButton->setEnabled(true);
            }
            });

        // --- 4. ダイアログを表示 ---
        // mChildWindowに追加するのではなく、mGuiのトップレベルに追加
        mGui->add(messageBox);
        });
}

SnapshotScreen::~SnapshotScreen()
{
    // ゲームのUIスタックから自分を削除する処理はGame::UpdateGame()で行われる

    // ゲームの状態をプレイ中に戻す
    mGame->SetState(Game::GameState::EGamePlay);

    // 保持していたサムネイルテクスチャをすべて解放
    for (sf::Texture* tex : mThumbnailTextures)
    {
        delete tex;
    }
    mThumbnailTextures.clear();

    std::cout << "SnapshotScreen destroyed. Thumbnails cleared." << std::endl;
}

void SnapshotScreen::Update(float deltaTime)
{
    // 現在の盤面のスナップショットを保存中で、スナップショットダイアログがnullptrになったら、
    // 保存を終了したと判断してクローズする
    if (mIsSavingCurrentSnapshot && mChildSnapshotDialog == nullptr)
    {
        // Gameのスナップショット復元機能を呼び出す
        mGame->RestoreSnapshot(mSelectedIndex);
        Close();
    }

    // サムネイルが選択されていない場合はプレビュー画面を消しておく
    if (mThumbnailListBox->getSelectedItemIndex() == -1)
    {
        mSelectedIndex = -1;
        ReleasePreview();
    }
}

void SnapshotScreen::Draw(sf::RenderWindow* window)
{
    mGui->draw(); // このUIが持つGUIを描画
}

void SnapshotScreen::ProcessInput(const sf::Event* event, const sf::Vector2i& mousePos)
{
    mGui->handleEvent(*event); // このUIが持つGUIにイベントを渡す
}

void SnapshotScreen::Close()
{
    mState = IUIScreen::UIState::EClosing;
    mGui->remove(mChildWindow);
}

void SnapshotScreen::AddSnapshot(const std::string& name, const std::string& comment)
{
    mGame->AddSnapshot(name, comment);
    auto snapshot = mGame->GetSnapshots().back();

    // 盤面のサムネイルの描画範囲
    std::pair<sf::Vector2f, sf::Vector2f> imageBoundingBox{ sf::Vector2f{ 0.0f, 0.0f }, sf::Vector2f{ mThumbnailListBox->getItemsHeight().getValue(), mThumbnailListBox->getItemsHeight().getValue() } };
    // テキストサイズ
    int textSize(16);
    // スナップショットのタイトルとコメントの描画領域のサイズ
    sf::Vector2f thumbnailTitleSize{ (mThumbnailListBox->getItemsWidth().getValue() - (imageBoundingBox.second.x - imageBoundingBox.first.x)) * 0.95f, static_cast<float>(textSize) * 1.25f };
    sf::Vector2f thumbnailCommentSize{ thumbnailTitleSize.x, (imageBoundingBox.second.y - imageBoundingBox.first.y) * 0.95f - thumbnailTitleSize.y };
    // スナップショットのタイトルとコメントの初期位置
    sf::Vector2f thumbnailTitleOffset{ imageBoundingBox.second.x + (mThumbnailListBox->getItemsWidth().getValue() - imageBoundingBox.second.x - thumbnailTitleSize.x) / 2.0f, imageBoundingBox.first.y + (mThumbnailListBox->getItemsHeight().getValue() - thumbnailTitleSize.y - thumbnailCommentSize.y) / 2.0f };
    sf::Vector2f thumbnailCommentOffset{ thumbnailTitleOffset.x, thumbnailTitleOffset.y + thumbnailTitleSize.y };

    // sf::Textureからtgui::textureへの変換
    sf::Texture* currentBoardTex = mGame->GenerateThumbnail(snapshot);
    mThumbnailTextures.emplace_back(currentBoardTex);
    sf::Image tmpImage = currentBoardTex->copyToImage();
    tgui::Texture tmpTGuiTex{};
    tmpTGuiTex.loadFromPixelData(tmpImage.getSize(), tmpImage.getPixelsPtr());
    tgui::Picture::Ptr templatePic = tgui::Picture::create(tmpTGuiTex);
    // 画像のスケーリングとオフセットの設定
    float scaleFactor(0.0f);
    if ((imageBoundingBox.second.x - imageBoundingBox.first.x) / static_cast<float>(tmpImage.getSize().x) < (imageBoundingBox.second.y - imageBoundingBox.first.y) / static_cast<float>(tmpImage.getSize().y))
    {
        scaleFactor = (imageBoundingBox.second.x - imageBoundingBox.first.x) / static_cast<float>(tmpImage.getSize().x);
        templatePic->setSize(tgui::Layout2d{ static_cast<float>(tmpImage.getSize().x) * scaleFactor, static_cast<float>(tmpImage.getSize().y) * scaleFactor });
        templatePic->setPosition(0.0f, imageBoundingBox.first.y + ((imageBoundingBox.second.y - imageBoundingBox.first.y) - templatePic->getSize().y) / 2.0f);
    }
    else
    {
        scaleFactor = (imageBoundingBox.second.y - imageBoundingBox.first.y) / static_cast<float>(tmpImage.getSize().y);
        templatePic->setSize(tgui::Layout2d{ static_cast<float>(tmpImage.getSize().x) * scaleFactor, static_cast<float>(tmpImage.getSize().y) * scaleFactor });
        templatePic->setPosition(imageBoundingBox.first.x + ((imageBoundingBox.second.x - imageBoundingBox.first.x) - templatePic->getSize().x) / 2.0f, 0.0f);
    }

    // テキストを表示
    // スナップショットの名前
    auto nameLabel = tgui::Label::create("Name: " + snapshot.snapshotName);
    nameLabel->setRenderer(mTheme->getRenderer("Label"));
    nameLabel->setSize(thumbnailTitleSize.x, thumbnailTitleSize.y);
    nameLabel->setPosition(thumbnailTitleOffset.x, thumbnailTitleOffset.y);
    nameLabel->setTextSize(textSize);
    nameLabel->getScrollbar()->setPolicy(tgui::Scrollbar::Policy::Never);
    // スナップショットのコメント
    auto commentBox = tgui::TextArea::create();
    mCommentTextAreaMap.emplace(commentBox, snapshot.snapshotName);
    commentBox->setRenderer(mTheme->getRenderer("TextArea"));
    commentBox->setSize(thumbnailCommentSize.x, thumbnailCommentSize.y);
    commentBox->setPosition(thumbnailCommentOffset.x, thumbnailCommentOffset.y);
    commentBox->setTextSize(textSize);
    commentBox->setDefaultText("Enter a comment for this snapshot.");
    commentBox->setText(snapshot.snapshotComment);
    // テキスト変更時に外部変数を更新
    commentBox->onTextChange([this, commentBox]() {
        mGame->SetSnapshotComment(mCommentTextAreaMap[commentBox], commentBox->getText().toStdString());
        });

    // サムネイルリストに追加
    auto templatePanel = mThumbnailListBox->addItem(snapshot.snapshotName, static_cast<int>(mThumbnailListBox->getItemCount()));
    templatePanel->add(templatePic, snapshot.snapshotName);
    std::cout << "SnapshotScreen: Add name label -> " << nameLabel->getText() << std::endl;
    templatePanel->add(nameLabel, snapshot.snapshotName);
    std::cout << "SnapshotScreen: Add comment -> " << commentBox->getText() << std::endl;
    templatePanel->add(commentBox, snapshot.snapshotName);
}

void SnapshotScreen::ReleasePreview()
{
    // テクスチャを解除
    mSelectedPreview->getRenderer()->setTexture({});

    // ラベルもクリア
    mInfoStepsLabel->setText("Steps: ");
    mInfoGoaledLabel->setText("Goaled: ");
    mInfoTimeLabel->setText("Saved: ");
}