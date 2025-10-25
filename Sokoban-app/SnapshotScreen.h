#pragma once

#include "SFML/Graphics.hpp"
#include "TGUI/TGUI.hpp"
#include "TGUI/Backend/SFML-Graphics.hpp"
#include <memory>
#include <vector>

#include "IUIScreen.h"

class Game;

class SnapshotScreen : public IUIScreen
{
public:
    SnapshotScreen(Game* game, sf::RenderWindow* window);
    virtual ~SnapshotScreen();

    // --- IUIScreenインターフェースのオーバーライド ---
    void Update(float deltaTime) override;
    void Draw(sf::RenderWindow* window) override;
    void ProcessInput(const sf::Event* event, const sf::Vector2i& mousePos) override;
    UIState GetState() override { return mState; }

    void Close();

    // スナップショットを追加するメンバ関数
    void AddSnapshot(const std::string& name, const std::string& comment);

    // スナップショットダイアログのポインタを初期化する
    void RemoveChildSnapshotDialog() { mChildSnapshotDialog = nullptr; }

private:
    // このクラス専用のヘルパー関数
    // 
    void ReleasePreview();

    // メンバ変数
    Game* mGame;
    sf::RenderWindow* mWindow;
    std::unique_ptr<tgui::Gui> mGui;
    std::unique_ptr<tgui::Theme> mTheme;
    UIState mState;

    // サムネイルテクスチャの管理
    // 動的に生成されたサムネイルテクスチャのポインタを保持し、デストラクタで解放する
    std::vector<sf::Texture*> mThumbnailTextures;

    // TGUIウィジェット
    tgui::ChildWindow::Ptr mChildWindow;
    tgui::PanelListBox::Ptr mThumbnailListBox;
    tgui::Panel::Ptr mRightPanel;
    tgui::Picture::Ptr mSelectedPreview;
    tgui::Panel::Ptr mInfoPanel;
    tgui::Button::Ptr mCloseButton;
    tgui::Button::Ptr mRemoveButton;
    tgui::Button::Ptr mApplyButton;
    tgui::Label::Ptr mInfoStepsLabel;
    tgui::Label::Ptr mInfoGoaledLabel;
    tgui::Label::Ptr mInfoTimeLabel;

    // 現在選択されているスナップショットのインデックス
    int mSelectedIndex;

    // スナップショットのインデックスと盤面状況の画像のマップ
    std::map<int, tgui::Picture::Ptr> mBoardPictureMap;

    // コメントのテキスト更新のためにスナップショットのコメントのテキストオブジェクトとタイトルの対応付けをしておく
    std::map<tgui::TextArea::Ptr, std::string> mCommentTextAreaMap;

    // このクラスからスナップショットダイアログを追加した場合に保持するポインタ
    class SnapshotDialog* mChildSnapshotDialog;

    // 現在の盤面のスナップショットをセーブ中か
    bool mIsSavingCurrentSnapshot;
};