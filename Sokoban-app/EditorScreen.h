#pragma once

#include "SFML/Graphics.hpp"
#include "TGUI/TGUI.hpp"
#include "TGUI/Backend/SFML-Graphics.hpp"
#include <memory>
#include <vector>
#include <string>
#include <map>

#include "IUIScreen.h"

class Game;

class EditorScreen : public IUIScreen
{
public:
	EditorScreen(Game* game, sf::RenderWindow* window, const bool editCurrentBoard);
	virtual ~EditorScreen();

	// IUIScreenインターフェースのオーバーライド
	void Update(float deltaTime) override;
	void Draw(sf::RenderWindow* window) override;
	void ProcessInput(const sf::Event* event, const sf::Vector2i& mousePos) override;
	UIState GetState() override { return mState; }

	void Close();

private:
	// このクラス特有のヘルパー関数
	// ウィンドウのピクセル座標をグリッド座標に変換
	sf::Vector2i ScreenToGrid(const sf::Vector2f& screenPos) const;
	// グリッドキャンパスとグリッド線の描画
	void DrawCanvas();
	// 指定したグリッド座標にタイルを配置する
	void PlaceTile(const sf::Vector2i& gridPos);
	// 現在の盤面の妥当性を検証し、保存を試みる
	void TrySaveLevel();
	// 情報パネルのテキストを更新
	void UpdateInfoPanel();

	// システム関連のメンバ変数
	Game* mGame;
	UIState mState;
	std::unique_ptr<tgui::Gui> mGui;
	std::unique_ptr<tgui::Theme> mTheme;

	// このクラス特有のメンバ変数
	// タイルパレット
	enum class SelectedTile
	{
		Floor,
		Wall,
		Goal,
		Baggage,
		Player,
		Erase	// 床に戻す
	};
	SelectedTile mCurrentTile;	// 現在選択中のタイル
	std::map<SelectedTile, char> mTileToChar;	// タイルEnumと保存用文字のマッピング

	// 盤面関連
	const sf::Vector2i mEditorCanvasSize; // 盤面の固定サイズ (例: 30x30)
	float mCanvasTileSize;        // 1タイルのピクセルサイズ
	sf::RectangleShape mGridLines[2]; // グリッド描画用 (縦/横)
	sf::RectangleShape mSelectedCellHighlight; // 選択中マスのハイライト

	// タイル描画用のスプライト (テクスチャはGameから拝借)
	sf::Sprite mFloorSprite;
	sf::Sprite mWallSprite;
	sf::Sprite mGoalSprite;
	sf::Sprite mBaggageSprite;
	sf::Sprite mPlayerSprite;

	// 盤面データ
	std::vector<std::string> mEditorBoardData; // 編集中の盤面データ
	sf::Vector2i mSelectedGridCell; // キーボード/マウスで選択中のマス

	// TGUIウィジェット
	tgui::ChildWindow::Ptr mEditorWindow;	// エディタ全体を収める親ウィンドウ
	tgui::CanvasSFML::Ptr mCanvasWidget;	// 盤面を描画するためのTGUIキャンバス
	tgui::Panel::Ptr mTilePalettePanel; // 左下のタイルパレット
	tgui::Panel::Ptr mInfoPanel;        // 右下の情報パネル
	tgui::Label::Ptr mInfoLabel;        // 荷物/ゴール数表示用
	tgui::Label::Ptr mInfoBaggageGoaledLabel;
	tgui::Button::Ptr mSaveButton;
	tgui::Button::Ptr mExitButton;

	// UI周りのメンバ変数
	// マウス用：現在マウスが乗っているグリッド座標
	sf::Vector2i mHoverGridCell;
	// UX用：現在マウス操作モードかどうか（薄いハイライトの表示制御用）
	bool mIsMouseMode;
	// 選択カーソルのハイライト用シェイプ（濃い枠）
	sf::RectangleShape mCursorHighlight;
	// マウスホバーのハイライト用シェイプ（薄い色）
	sf::RectangleShape mHoverHighlight;
	// ヘルプボタン
	tgui::Button::Ptr mHelpButton;
};

