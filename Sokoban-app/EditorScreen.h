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
	EditorScreen(Game* game, sf::RenderWindow* window, int loadMode);
	virtual ~EditorScreen();

	// IUIScreenインターフェースのオーバーライド
	void Update(float deltaTime) override;
	void Draw(sf::RenderWindow* window) override;
	void ProcessInput(const sf::Event* event, const sf::Vector2i& mousePos) override;
	UIState GetState() override { return mState; }

	void Close();

private:
	// タイルパレット
	enum class SelectedTile
	{
		Wall,
		Floor,
		Goal,
		Baggage,
		Player,
		Erase	// 床に戻す
	};

	// このクラス特有のヘルパー関数
	// ウィンドウのピクセル座標をグリッド座標に変換
	sf::Vector2i ScreenToGrid(const sf::Vector2f& screenPos) const;
	// グリッドキャンパスとグリッド線の描画
	void DrawCanvas();
	// 指定したグリッド座標にタイルを配置する
	void PlaceTile(const sf::Vector2i& gridPos, bool isContinuous = false);
	// 情報パネルのテキストを更新
	void UpdateInfoPanel();
	// キャンバスサイズを動的に変更
	void ResizeCanvasIfNeeded();
	// 現在選択されている範囲を指定したタイルで塗りつぶす
	void FillSelection(SelectedTile tile);
	// バリデーション用関数
	bool CheckReachability(); // 到達可能性と閉塞性のチェック
	void UpdateValidationState(); // 全バリデーションを実行しUI更新
	// 終了確認とセーブ
	void CheckUnsavedChangesAndClose(); // 終了時のチェック
	void PerformSave(bool closeAfterSave); // セーブ実行
	void PerformApply(); // 適用実行

	// システム関連のメンバ変数
	Game* mGame;
	UIState mState;
	std::unique_ptr<tgui::Gui> mGui;
	std::unique_ptr<tgui::Theme> mTheme;

	// このクラス特有のメンバ変数
	SelectedTile mCurrentTile;	// 現在選択中のタイル
	std::map<SelectedTile, char> mTileToChar;	// タイルEnumと保存用文字のマッピング

	// 盤面関連
	sf::Vector2i mEditorCanvasSize; // 盤面のサイズ
	const int mCanvasSizeMin;	// 盤面の最小サイズ
	const int mCanvasSizeMax;	// 盤面の最大サイズ
	float mCanvasTileSize;        // 1タイルのピクセルサイズ
	sf::RectangleShape mGridLines[2]; // グリッド描画用 (縦/横)
	sf::RectangleShape mSelectedCellHighlight; // 選択中マスのハイライト

	// タイル描画用のスプライト (テクスチャはGameから拝借)
	sf::Sprite mFloorSprite;
	sf::Sprite mWallSprite;
	sf::Sprite mGoalSprite;
	sf::Sprite mBaggageSprite;
	sf::Sprite mShinyBaggageSprite;
	sf::Sprite mPlayerSprite;

	// 盤面データ
	std::vector<std::string> mEditorBoardData; // 編集中の盤面データ
	sf::Vector2i mSelectedGridCell; // キーボード/マウスで選択中のマス
	std::vector<std::string> mLastSavedData; // 最後にセーブした状態（変更検知用）
	bool mIsValidLevel; // 現在の盤面が有効かどうか

	// TGUIウィジェット
	tgui::ChildWindow::Ptr mEditorWindow;	// エディタ全体を収める親ウィンドウ
	tgui::CanvasSFML::Ptr mCanvasWidget;	// 盤面を描画するためのTGUIキャンバス
	tgui::Panel::Ptr mTilePalettePanel; // 左下のタイルパレット
	tgui::Panel::Ptr mSystemPalettePanel; // 右下のタイルパレット
	tgui::Panel::Ptr mInfoPanel;        // 右の情報パネル
	tgui::Label::Ptr mInfoLabel;        // 荷物/ゴール数表示用
	tgui::Label::Ptr mInfoBaggageGoaledLabel;
	tgui::Button::Ptr mApplyButton; // 適用ボタン
	tgui::Button::Ptr mSaveButton;
	tgui::Button::Ptr mExitButton;
	// 選択中のタイルを示すラジオボタン
	tgui::RadioButton::Ptr mWallRadioButton;
	tgui::RadioButton::Ptr mFloorRadioButton;
	tgui::RadioButton::Ptr mGoalRadioButton;
	tgui::RadioButton::Ptr mBaggageRadioButton;
	tgui::RadioButton::Ptr mPlayerRadioButton;
	tgui::RadioButton::Ptr mEraseRadioButton;
	// モード切替用ラジオボタン
	tgui::RadioButtonGroup::Ptr mModeGroup;
	tgui::RadioButton::Ptr mRadioDraw;
	tgui::RadioButton::Ptr mRadioSelect;
	// 範囲選択時用のアクションボタン
	tgui::BitmapButton::Ptr mBtnFillWall;
	tgui::BitmapButton::Ptr mBtnFillFloor;
	tgui::BitmapButton::Ptr mBtnFillGoal;
	tgui::Button::Ptr mBtnDeleteArea;

	// UI周りのメンバ変数
	// タイルパレットのサイズとオフセット
	tgui::Layout2d mTilePaletteSize;
	tgui::Layout2d mTilePaletteOffset;
	// タイル選択ボタンのサイズとオフセットと配置間隔
	tgui::Layout2d mTileButtonSize;
	tgui::Layout2d mTileButtonOffset;
	tgui::Layout2d mTileButtonInterval;
	// タイル選択ボタンのアイコンのスケーリング
	float mIconImageScale;
	// システム関連のボタンのパレットのサイズとオフセット
	tgui::Layout2d mSystemButtonPaletteSize;
	tgui::Layout2d mSystemButtonPaletteOffset;
	// セーブボタンとエディタ終了ボタンのサイズとオフセットと配置間隔
	tgui::Layout2d mSystemButtonSize;
	tgui::Layout2d mSystemButtonOffset;
	tgui::Layout2d mSystemButtonInterval;
	// マウス用：現在マウスが乗っているグリッド座標
	sf::Vector2i mHoverGridCell;
	// UX用：現在マウス操作モードかどうか（薄いハイライトの表示制御用）
	bool mIsMouseMode;
	// 選択カーソルのハイライト用シェイプ（濃い枠）
	sf::RectangleShape mCursorHighlight;
	// マウスホバーのハイライト用シェイプ（薄い色）
	sf::RectangleShape mHoverHighlight;
	// モーダルダイアログが表示中かどうか
	bool mIsDialogOpen;

	// 矩形選択用
	bool mIsSelecting;          // ドラッグ中か
	sf::Vector2i mDragStartPos; // ドラッグ開始位置
	sf::Vector2i mDragEndPos;   // ドラッグ終了位置（現在のマウス位置）
	sf::RectangleShape mSelectionRect; // 選択範囲の描画用
};

