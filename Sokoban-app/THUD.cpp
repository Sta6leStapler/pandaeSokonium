#include "THUD.h"

#include "Game.h"
#include "LocalizationManager.h"
#include "HUDHelper.h"

#include <iostream>

THUD::THUD(Game* game, sf::RenderWindow* window)
	: mGame(game)
	, mGui(std::make_unique<tgui::Gui>(*window))
	, mRollBackLargeButtonSize(sf::Vector2i{ 120 , 50 })
	, mRollBackSmallButtonSize(sf::Vector2i{ 120 , 40 })
	, mGameSystemButtonSize(sf::Vector2i{ 150 , 40 })
	, mButtonInitialPos(sf::Vector2i{ 20, 30 })
	, mButtonMergin(sf::Vector2i{ 20, 20 })
	, mListBoxItemHeight(32)
	, mIconImageScale(0.7f)
	, mState(IUIScreen::UIState::EActive)
	, mIsUndoHeld(false)
	, mIsRedoHeld(false)
	, mUndoRepeatTimer(0.0f)
	, mRedoRepeatTimer(0.0f)
{
	mGame->PushUI(this);

	// フォントを設定
	mGui->setFont(mGame->GetFontTGUI());

	// ブロッカー用のパネル
	mBlocker = tgui::Panel::create();
	mBlocker->setSize("100%", "100%"); // 画面全体を覆う
	mBlocker->getRenderer()->setBackgroundColor(tgui::Color(0, 0, 0, 0)); // 透明

	// 各種ボタンを追加
	// Undo/Redo操作関連のボタン
	// 盤面の描画範囲の下に配置する
	// ボタンの基準位置
	sf::Vector2f rollbackButtonInitialPos{ mGame->GetBoardViewArea().second.x, mGame->GetBoardViewArea().second.y };
	// 盤面の描画範囲とウィンドウの間のマージン
	sf::Vector2f rollbackLargeButtonMargin{ (mGame->GetWindowSize().y - (mGame->GetBoardViewArea().second.y - mGame->GetBoardViewArea().first.y) - static_cast<float>(mRollBackLargeButtonSize.y)) / 2.0f, (mGame->GetWindowSize().y - (mGame->GetBoardViewArea().second.y - mGame->GetBoardViewArea().first.y) - static_cast<float>(mRollBackLargeButtonSize.y)) / 2.0f };
	sf::Vector2f rollbackSmallButtonMargin{ (mGame->GetWindowSize().y - (mGame->GetBoardViewArea().second.y - mGame->GetBoardViewArea().first.y) - static_cast<float>(mRollBackSmallButtonSize.y)) / 2.0f, (mGame->GetWindowSize().y - (mGame->GetBoardViewArea().second.y - mGame->GetBoardViewArea().first.y) - static_cast<float>(mRollBackSmallButtonSize.y)) / 2.0f };
	// Undoボタン
	sf::Image tmpImage = mGame->LoadTexture("Assets/Undo.png")->copyToImage();
	tgui::Texture tmpTGuiTex{};
	tmpTGuiTex.loadFromPixelData(tmpImage.getSize(), tmpImage.getPixelsPtr());
	mUndoButton = tgui::BitmapButton::create(mGame->GetLoc()->Get("BTN_UNDO"));
	mUndoButton->setSize(mRollBackLargeButtonSize.x, mRollBackLargeButtonSize.y);    // ボタンのサイズ
	// ボタンの位置
	mUndoButton->setPosition(
		rollbackButtonInitialPos.x - rollbackLargeButtonMargin.x - mRollBackLargeButtonSize.x,
		rollbackButtonInitialPos.y + rollbackLargeButtonMargin.y);
	mUndoButton->setRenderer(mGame->GetTheme().getRenderer("BitmapButton"));
	mUndoButton->setImage(tmpTGuiTex);
	mUndoButton->setImageScaling(mIconImageScale);
	mUndoButton->setTextSize(16);
	mUndoButton->onMousePress([this]() {
		std::cout << "Undo action triggered!" << std::endl;
		if (mGame->IsAnyMovementAnimating()) return; // アニメーション中は無視
		mIsUndoHeld = true;
		mGame->CallUndo(); // 押した瞬間にまず1回実行
		// 最初の長押し判定までの時間を設定
		mUndoRepeatTimer = mGame->GetHoldThresHold();
		});
	mUndoButton->onMouseRelease([this]() {
		std::cout << "Released undo Button!" << std::endl;
		mIsUndoHeld = false;
		});
	// ボタンの領域外にマウスが出ても停止する
	mUndoButton->onMouseLeave([this]() {
		mIsUndoHeld = false;
		});
	mGui->add(mUndoButton);

	// Redoボタン
	tmpImage = mGame->LoadTexture("Assets/Redo.png")->copyToImage();
	tmpTGuiTex.loadFromPixelData(tmpImage.getSize(), tmpImage.getPixelsPtr());
	mRedoButton = tgui::BitmapButton::create(mGame->GetLoc()->Get("BTN_REDO"));
	mRedoButton->setSize(mRollBackLargeButtonSize.x, mRollBackLargeButtonSize.y);    // ボタンのサイズ
	// ボタンの位置
	mRedoButton->setPosition(
		rollbackButtonInitialPos.x - mRollBackLargeButtonSize.x * 2.0f - rollbackLargeButtonMargin.x * 3.0f,
		rollbackButtonInitialPos.y + rollbackLargeButtonMargin.y);
	mRedoButton->setRenderer(mGame->GetTheme().getRenderer("BitmapButton"));
	mRedoButton->setImage(tmpTGuiTex);
	mRedoButton->setImageScaling(mIconImageScale);
	mRedoButton->setTextSize(16);
	mRedoButton->onMousePress([this]() {
		std::cout << "Redo action triggered!" << std::endl;
		if (mGame->IsAnyMovementAnimating()) return; // アニメーション中は無視
		mIsRedoHeld = true;
		mGame->CallRedo(); // 押した瞬間にまず1回実行
		// 最初の長押し判定までの時間を設定
		mRedoRepeatTimer = mGame->GetHoldThresHold();
		});
	mRedoButton->onMouseRelease([this]() {
		mIsRedoHeld = false;
		});
	mRedoButton->onMouseLeave([this]() {
		mIsRedoHeld = false;
		});
	mGui->add(mRedoButton);

	// Resetボタン
	tmpImage = mGame->LoadTexture("Assets/Reset.png")->copyToImage();
	tmpTGuiTex.loadFromPixelData(tmpImage.getSize(), tmpImage.getPixelsPtr());
	mResetButton = tgui::BitmapButton::create(mGame->GetLoc()->Get("BTN_RESET"));
	mResetButton->setSize(mRollBackSmallButtonSize.x, mRollBackSmallButtonSize.y);    // ボタンのサイズ
	// ボタンの位置
	mResetButton->setPosition(
		rollbackButtonInitialPos.x - mRollBackLargeButtonSize.x * 2.0 - mRollBackSmallButtonSize.x - rollbackLargeButtonMargin.x * 4.0f - rollbackSmallButtonMargin.x,
		rollbackButtonInitialPos.y + rollbackSmallButtonMargin.y);
	mResetButton->setRenderer(mGame->GetTheme().getRenderer("BitmapButton"));
	mResetButton->setImage(tmpTGuiTex);
	mResetButton->setImageScaling(mIconImageScale);
	mResetButton->setTextSize(16);
	mResetButton->onPress([=]() {
		std::cout << "Reset action triggered!" << std::endl;
		if (mGame->IsAnyMovementAnimating()) return; // アニメーション中は無視
		mGame->CallReset();
		});
	mGui->add(mResetButton);

	// Redo Allボタン
	tmpImage = mGame->LoadTexture("Assets/RedoAll.png")->copyToImage();
	tmpTGuiTex.loadFromPixelData(tmpImage.getSize(), tmpImage.getPixelsPtr());
	mRedoAllButton = tgui::BitmapButton::create(mGame->GetLoc()->Get("BTN_REDO_ALL"));
	mRedoAllButton->setSize(mRollBackSmallButtonSize.x, mRollBackSmallButtonSize.y);    // ボタンのサイズ
	// ボタンの位置
	mRedoAllButton->setPosition(
		rollbackButtonInitialPos.x - mRollBackLargeButtonSize.x * 2.0 - mRollBackSmallButtonSize.x * 2.0f - rollbackLargeButtonMargin.x * 4.0f - rollbackSmallButtonMargin.x * 3.0f,
		rollbackButtonInitialPos.y + rollbackSmallButtonMargin.y);
	mRedoAllButton->setRenderer(mGame->GetTheme().getRenderer("BitmapButton"));
	mRedoAllButton->setImage(tmpTGuiTex);
	mRedoAllButton->setImageScaling(mIconImageScale);
	mRedoAllButton->setTextSize(16);
	mRedoAllButton->onPress([=]() {
		std::cout << "Redo all action triggered!" << std::endl;
		if (mGame->IsAnyMovementAnimating()) return; // アニメーション中は無視
		mGame->CallRedoAll();
		});
	mGui->add(mRedoAllButton);

	// Load Snapshotボタン
	mLoadSnapshotButton = tgui::Button::create(mGame->GetLoc()->Get("BTN_QUICK_LOAD"));
	mLoadSnapshotButton->setSize(mRollBackLargeButtonSize.x, mRollBackLargeButtonSize.y);    // ボタンのサイズ
	// ボタンの位置
	mLoadSnapshotButton->setPosition(
		mGame->GetBoardViewArea().first.x + rollbackLargeButtonMargin.x,
		rollbackButtonInitialPos.y + rollbackLargeButtonMargin.y);
	mLoadSnapshotButton->setRenderer(mGame->GetTheme().getRenderer("Button"));
	mLoadSnapshotButton->setTextSize(16);
	mLoadSnapshotButton->onPress([=]() {
		std::cout << "Load Snapshot action triggered!" << std::endl;
		if (mGame->IsAnyMovementAnimating()) return; // アニメーション中は無視
		// スナップショットマネージャーを呼び出す
		mGame->DisplaySnapshot();
		});
	mGui->add(mLoadSnapshotButton);

	// Add Snapshotボタン
	mAddSnapshotButton = tgui::Button::create(mGame->GetLoc()->Get("BTN_QUICK_SAVE"));
	mAddSnapshotButton->setSize(mRollBackLargeButtonSize.x, mRollBackLargeButtonSize.y);    // ボタンのサイズ
	// ボタンの位置
	mAddSnapshotButton->setPosition(
		mGame->GetBoardViewArea().first.x + mRollBackLargeButtonSize.x + rollbackLargeButtonMargin.x * 3.0f,
		rollbackButtonInitialPos.y + rollbackLargeButtonMargin.y);
	mAddSnapshotButton->setRenderer(mGame->GetTheme().getRenderer("Button"));
	mAddSnapshotButton->setTextSize(16);
	mAddSnapshotButton->onPress([=]() {
		std::cout << "Add Snapshot action triggered!" << std::endl;
		if (mGame->IsAnyMovementAnimating()) return; // アニメーション中は無視
		// スナップショットをリストに追加する処理を呼び出す
		mGame->AddSnapshotDialog();
		});
	mGui->add(mAddSnapshotButton);

	// Undo/Redo操作関連のボタンの配置ここまで
	
	// 以下はゲームシステム関連のボタンの配置
	// 各ボタンの基準位置のマージン
	sf::Vector2i listBoxMargin{ static_cast<int>((mGame->GetWindowSize().x - (mGame->GetBoardViewArea().second.x - mGame->GetBoardViewArea().first.x) - (mGameSystemButtonSize.x * 4 + mButtonMergin.x * 3)) / 2.0f), static_cast<int>((mGame->GetWindowSize().x - (mGame->GetBoardViewArea().second.x - mGame->GetBoardViewArea().first.x) - (mGameSystemButtonSize.x * 4 + mButtonMergin.x * 3)) / 4.0f) };

	// Load Boardボタン
	mLoadBoardButton = tgui::Button::create(mGame->GetLoc()->Get("TITLE_SELECT_BOARD"));
	mLoadBoardButton->setPosition(listBoxMargin.x, mButtonInitialPos.y); // ボタンの位置
	mLoadBoardButton->setSize(mGameSystemButtonSize.x, mGameSystemButtonSize.y);    // ボタンのサイズ
	mLoadBoardButton->setRenderer(mGame->GetTheme().getRenderer("Button"));
	mLoadBoardButton->setTextSize(16);
	mLoadBoardButton->onPress([=]() {
		std::cout << "Load board action triggered!" << std::endl;
		if (mGame->IsAnyMovementAnimating()) return; // アニメーション中は無視
		mGame->SelectBoards();
		});
	mGui->add(mLoadBoardButton);

	// Generate Boardボタン
	mGenBoardButton = tgui::Button::create(mGame->GetLoc()->Get("TITLE_BOARD_GENERATION"));
	mGenBoardButton->setPosition(listBoxMargin.x + (mGameSystemButtonSize.x + mButtonMergin.x), mButtonInitialPos.y); // ボタンの位置
	mGenBoardButton->setSize(mGameSystemButtonSize.x, mGameSystemButtonSize.y);    // ボタンのサイズ
	mGenBoardButton->setRenderer(mGame->GetTheme().getRenderer("Button"));
	mGenBoardButton->setTextSize(16);
	mGenBoardButton->onPress([=]() {
		std::cout << "Generate board action triggered!" << std::endl;
		if (mGame->IsAnyMovementAnimating()) return; // アニメーション中は無視
		mGame->CallReload();
		});
	mGui->add(mGenBoardButton);
	
	// Save Boardボタン
	tmpImage = mGame->LoadTexture("Assets/SaveBoard.png")->copyToImage();
	tmpTGuiTex.loadFromPixelData(tmpImage.getSize(), tmpImage.getPixelsPtr());
	mSaveBoardButton = tgui::BitmapButton::create(mGame->GetLoc()->Get("BTN_SAVE"));
	mSaveBoardButton->setPosition(listBoxMargin.x + (mGameSystemButtonSize.x + mButtonMergin.x) * 2, mButtonInitialPos.y); // ボタンの位置
	mSaveBoardButton->setSize(mGameSystemButtonSize.x, mGameSystemButtonSize.y);    // ボタンのサイズ
	mSaveBoardButton->setRenderer(mGame->GetTheme().getRenderer("BitmapButton"));
	mSaveBoardButton->setImage(tmpTGuiTex);
	mSaveBoardButton->setImageScaling(mIconImageScale);
	mSaveBoardButton->setTextSize(16);
	mSaveBoardButton->onPress([=]() {
		std::cout << "Save board action triggered!" << std::endl;
		if (mGame->IsAnyMovementAnimating()) return; // アニメーション中は無視
		mGame->CallSave();
		});
	mGui->add(mSaveBoardButton);

	// Save Logボタン
	tmpImage = mGame->LoadTexture("Assets/SaveLog.png")->copyToImage();
	tmpTGuiTex.loadFromPixelData(tmpImage.getSize(), tmpImage.getPixelsPtr());
	mSaveLogButton = tgui::BitmapButton::create(mGame->GetLoc()->Get("BTN_LOG"));
	mSaveLogButton->setPosition(listBoxMargin.x + (mGameSystemButtonSize.x + mButtonMergin.x) * 3, mButtonInitialPos.y); // ボタンの位置
	mSaveLogButton->setSize(mGameSystemButtonSize.x, mGameSystemButtonSize.y);    // ボタンのサイズ
	mSaveLogButton->setRenderer(mGame->GetTheme().getRenderer("BitmapButton"));
	mSaveLogButton->setImage(tmpTGuiTex);
	mSaveLogButton->setImageScaling(mIconImageScale);
	mSaveLogButton->setTextSize(16);
	mSaveLogButton->onPress([=]() {
		std::cout << "Save log action triggered!" << std::endl;
		if (mGame->IsAnyMovementAnimating()) return; // アニメーション中は無視
		mGame->CallSaveLog();
		});
	mGui->add(mSaveLogButton);

	// Edit Modeボタン
	mEditModeButton = tgui::Button::create(mGame->GetLoc()->Get("TITLE_EDITOR"));
	mEditModeButton->setPosition(listBoxMargin.x, mButtonInitialPos.y + (mGameSystemButtonSize.y + mButtonMergin.y)); // ボタンの位置
	mEditModeButton->setSize(mGameSystemButtonSize.x, mGameSystemButtonSize.y);    // ボタンのサイズ
	mEditModeButton->setRenderer(mGame->GetTheme().getRenderer("Button"));
	mEditModeButton->setTextSize(16);
	mEditModeButton->onPress([=]() {
		std::cout << "Edit Mode action triggered!" << std::endl;
		if (mGame->IsAnyMovementAnimating()) return; // アニメーション中は無視
		mGame->CallEditorSetup();
		});
	mGui->add(mEditModeButton);

	// Helpボタン
	mHelpButton = tgui::Button::create(mGame->GetLoc()->Get("TITLE_HELP"));
	mHelpButton->setPosition(listBoxMargin.x + (mGameSystemButtonSize.x + mButtonMergin.x) * 1, mButtonInitialPos.y + (mGameSystemButtonSize.y + mButtonMergin.y)); // ボタンの位置
	mHelpButton->setSize(mGameSystemButtonSize.x, mGameSystemButtonSize.y);    // ボタンのサイズ
	mHelpButton->setRenderer(mGame->GetTheme().getRenderer("Button"));
	mHelpButton->setTextSize(16);
	mHelpButton->onPress([=]() {
		std::cout << "Help action triggered!" << std::endl;
		if (mGame->IsAnyMovementAnimating()) return; // アニメーション中は無視
		mGame->DisplayHelpWindow();
		});
	mGui->add(mHelpButton);

	// ゲームシステム関連のボタンの配置ここまで
	
	// TODO 各種情報のテキストを表示する矩形
	mListBoxSize = sf::Vector2i{ static_cast<int>(mGame->GetWindowSize().x - (mGame->GetBoardViewArea().second.x - mGame->GetBoardViewArea().first.x) - listBoxMargin.x * 2), static_cast<int>(mGame->GetWindowSize().y) - (mButtonInitialPos.y + (mGameSystemButtonSize.y + mButtonMergin.y) * 2) - 20};
	mListBoxPos = sf::Vector2i{ listBoxMargin.x, mButtonInitialPos.y + (mGameSystemButtonSize.y + mButtonMergin.y) * 2 };

	mTextInfo[TextIndex::EMoveCount] = mGame->GetLoc()->Get("LBL_MOVES") + std::to_string(mGame->GetStep());
	mTextInfo[TextIndex::ETime] = mGame->GetLoc()->Get("LBL_TIME") + "00:00";
	mTextInfo[TextIndex::EBoardState] = mGame->GetLoc()->Get("LBL_STATUS");
	mTextInfo[TextIndex::EMovableBaggages] = mGame->GetLoc()->Get("LBL_BOARD_STATE");
	mTextInfo[TextIndex::EGoaledBaggages] = mGame->GetLoc()->Get("LBL_GOALED_BAGGAGES");
	mTextInfo[TextIndex::EDeadlockedBaggages] = mGame->GetLoc()->Get("LBL_DEADLOCKED_BAGGAGES");

	mListBox = tgui::ListBox::create();
	mListBox->setRenderer(mGame->GetTheme().getRenderer("ListBox"));
	mListBox->setSize(mListBoxSize.x, mListBoxSize.y);
	mListBox->setItemHeight(mListBoxItemHeight);
	mListBox->setPosition(mListBoxPos.x, mListBoxPos.y);
	mListBox->setTextSize(20);
	for (const auto& text : mTextInfo)
	{
		mListBox->addItem(text.second, std::to_string(text.first));
	}
	mGui->add(mListBox);
}

THUD::~THUD()
{

}

void THUD::Update(float deltaTime)
{
	// テキスト表示する各種情報を更新
	mTextInfo[TextIndex::EMoveCount] = mGame->GetLoc()->Get("LBL_MOVES") + std::to_string(mGame->GetStep());
	long time = static_cast<long>(mGame->GetSecTime());
	std::stringstream m, s;
	m << std::setw(2) << std::setfill('0') << time / 60;
	s << std::setw(2) << std::setfill('0') << time % 60;
	mTextInfo[TextIndex::ETime] = mGame->GetLoc()->Get("LBL_TIME") + m.str() + ":" + s.str();
	std::unordered_map<int, sf::Vector2i> deadlockedBaggages{ mGame->GetHUDHelper()->GetDeadlockedBaggages() },
		goaledBaggages{ mGame->GetHUDHelper()->GetGoaledBaggages() };
	std::vector<sf::Vector2i> movableBaggages{ mGame->GetHUDHelper()->GetCandidates() };
	if (!deadlockedBaggages.empty())
	{
		mTextInfo[TextIndex::EBoardState] = mGame->GetLoc()->Get("LBL_STATUS") + mGame->GetLoc()->Get("LBL_DEADLOCKED_MESSAGE");
	}
	else
	{
		mTextInfo[TextIndex::EBoardState] = mGame->GetLoc()->Get("LBL_STATUS");
	}
	std::string tmpStr{};
	for (const auto& baggage : goaledBaggages)
	{
		tmpStr += "( " + std::to_string(baggage.second.x) + ", " + std::to_string(baggage.second.y) + ") ";
	}
	mTextInfo[TextIndex::EGoaledBaggages] = mGame->GetLoc()->Get("LBL_GOALED_BAGGAGES") + tmpStr;
	tmpStr = "";
	for (const auto& baggage : movableBaggages)
	{
		tmpStr += "( " + std::to_string(baggage.x) + ", " + std::to_string(baggage.y) + ") ";
	}
	mTextInfo[TextIndex::EMovableBaggages] = mGame->GetLoc()->Get("LBL_MOVABLE_BAGGAGES") + tmpStr;
	tmpStr = "";
	for (const auto& baggage : deadlockedBaggages)
	{
		tmpStr += "( " + std::to_string(baggage.second.x) + ", " + std::to_string(baggage.second.y) + ") ";
	}
	mTextInfo[TextIndex::EDeadlockedBaggages] = mGame->GetLoc()->Get("LBL_DEADLOCKED_BAGGAGES") + tmpStr;
	tmpStr = "";

	for (const auto& text : mTextInfo)
	{
		mListBox->changeItemById(std::to_string(text.first), text.second);
	}
	// テキストの更新はここまで

	// --- UIボタンの長押しリピート処理 ---
	// 長押し処理：アニメーション中は何もしない
	if (mGame->IsAnyMovementAnimating()) {
		CancelButtonHolds(); // アニメーションが始まったら長押し状態を強制解除する
		return;
	}

	// Undoボタンの長押し処理
	if (mIsUndoHeld)
	{
		mUndoRepeatTimer -= deltaTime;
		if (mUndoRepeatTimer <= 0.0f)
		{
			mGame->CallUndo();
			// 次のリピート間隔を設定
			mUndoRepeatTimer = mGame->GetAutoRepeatInterval();
		}
	}

	// Redoボタンの長押し処理
	if (mIsRedoHeld)
	{
		mRedoRepeatTimer -= deltaTime;
		if (mRedoRepeatTimer <= 0.0f)
		{
			mGame->CallRedo();
			// 次のリピート間隔を設定
			mRedoRepeatTimer = mGame->GetAutoRepeatInterval();
		}
	}
}

void THUD::ProcessInput(const sf::Event* event, const sf::Vector2i& mousePos)
{
	// GUIの入力処理を行う
	mGui->handleEvent(*event);

}

void THUD::Draw(sf::RenderWindow* mWindow)
{
	// GUIの描画
	mGui->draw();
}

void THUD::CancelButtonHolds()
{
	mIsUndoHeld = false;
	mIsRedoHeld = false;

	// タイマーもリセットしておく
	mUndoRepeatTimer = 0.0f;
	mRedoRepeatTimer = 0.0f;
}

void THUD::OnLanguageChanged()
{
	// ボタンテキストの更新
	mUndoButton->setText(mGame->GetLoc()->Get("BTN_UNDO"));
	mRedoButton->setText(mGame->GetLoc()->Get("BTN_REDO"));
	mResetButton->setText(mGame->GetLoc()->Get("BTN_RESET"));
	mRedoAllButton->setText(mGame->GetLoc()->Get("BTN_REDO_ALL"));
	mLoadSnapshotButton->setText(mGame->GetLoc()->Get("TITLE_QUICK_LOAD"));
	mAddSnapshotButton->setText(mGame->GetLoc()->Get("TITLE_QUICK_SAVE"));
	mLoadBoardButton->setText(mGame->GetLoc()->Get("TITLE_SELECT_BOARD"));
	mGenBoardButton->setText(mGame->GetLoc()->Get("TITLE_BOARD_GENERATION"));
	mSaveBoardButton->setText(mGame->GetLoc()->Get("BTN_SAVE"));
	mSaveLogButton->setText(mGame->GetLoc()->Get("BTN_LOG"));
	mEditModeButton->setText(mGame->GetLoc()->Get("TITLE_EDITOR"));
	mHelpButton->setText(mGame->GetLoc()->Get("BTN_HELP"));

	// ListBox内の静的なラベル（Movesなど）の更新
	// Update()内で毎フレーム changeItemById されているため、
	// 次のフレームで自動的に辞書から最新の文字が引かれます。

}