#include "THUD.h"

#include "Game.h"
#include "HUDHelper.h"

#include <iostream>

THUD::THUD(Game* game, sf::RenderWindow* window)
	: mGame(game)
	, mGui(std::make_unique<tgui::Gui>(*window))
	, mTheme(std::make_unique<tgui::Theme>("Assets/themes/Black.txt"))
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

	// テーマを設定
	tgui::Theme::setDefault();

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
	auto undoButton = tgui::BitmapButton::create("Undo");
	undoButton->setSize(mRollBackLargeButtonSize.x, mRollBackLargeButtonSize.y);    // ボタンのサイズ
	// ボタンの位置
	undoButton->setPosition(
		rollbackButtonInitialPos.x - rollbackLargeButtonMargin.x - mRollBackLargeButtonSize.x,
		rollbackButtonInitialPos.y + rollbackLargeButtonMargin.y);
	undoButton->setRenderer(mTheme->getRenderer("BitmapButton"));
	undoButton->setImage(tmpTGuiTex);
	undoButton->setImageScaling(mIconImageScale);
	undoButton->setTextSize(16);
	undoButton->onMousePress([this]() {
		std::cout << "Undo action triggered!" << std::endl;
		mIsUndoHeld = true;
		mGame->CallUndo(); // 押した瞬間にまず1回実行
		// 最初の長押し判定までの時間を設定
		mUndoRepeatTimer = mGame->GetHoldThresHold();
		});
	undoButton->onMouseRelease([this]() {
		std::cout << "Released undo Button!" << std::endl;
		mIsUndoHeld = false;
		});
	// ボタンの領域外にマウスが出ても停止する
	undoButton->onMouseLeave([this]() {
		mIsUndoHeld = false;
		});
	mGui->add(undoButton);

	// Redoボタン
	tmpImage = mGame->LoadTexture("Assets/Redo.png")->copyToImage();
	tmpTGuiTex.loadFromPixelData(tmpImage.getSize(), tmpImage.getPixelsPtr());
	auto redoButton = tgui::BitmapButton::create("Redo");
	redoButton->setSize(mRollBackLargeButtonSize.x, mRollBackLargeButtonSize.y);    // ボタンのサイズ
	// ボタンの位置
	redoButton->setPosition(
		rollbackButtonInitialPos.x - mRollBackLargeButtonSize.x * 2.0f - rollbackLargeButtonMargin.x * 3.0f,
		rollbackButtonInitialPos.y + rollbackLargeButtonMargin.y);
	redoButton->setRenderer(mTheme->getRenderer("BitmapButton"));
	redoButton->setImage(tmpTGuiTex);
	redoButton->setImageScaling(mIconImageScale);
	redoButton->setTextSize(16);
	redoButton->onMousePress([this]() {
		std::cout << "Redo action triggered!" << std::endl;
		mIsRedoHeld = true;
		mGame->CallRedo(); // 押した瞬間にまず1回実行
		// 最初の長押し判定までの時間を設定
		mRedoRepeatTimer = mGame->GetHoldThresHold();
		});
	redoButton->onMouseRelease([this]() {
		mIsRedoHeld = false;
		});
	redoButton->onMouseLeave([this]() {
		mIsRedoHeld = false;
		});
	mGui->add(redoButton);

	// Resetボタン
	tmpImage = mGame->LoadTexture("Assets/Reset.png")->copyToImage();
	tmpTGuiTex.loadFromPixelData(tmpImage.getSize(), tmpImage.getPixelsPtr());
	auto resetButton = tgui::BitmapButton::create("Reset");
	resetButton->setSize(mRollBackSmallButtonSize.x, mRollBackSmallButtonSize.y);    // ボタンのサイズ
	// ボタンの位置
	resetButton->setPosition(
		rollbackButtonInitialPos.x - mRollBackLargeButtonSize.x * 2.0 - mRollBackSmallButtonSize.x - rollbackLargeButtonMargin.x * 4.0f - rollbackSmallButtonMargin.x,
		rollbackButtonInitialPos.y + rollbackSmallButtonMargin.y);
	resetButton->setRenderer(mTheme->getRenderer("BitmapButton"));
	resetButton->setImage(tmpTGuiTex);
	resetButton->setImageScaling(mIconImageScale);
	resetButton->setTextSize(16);
	resetButton->onPress([=]() {
		std::cout << "Reset action triggered!" << std::endl;
		mGame->CallReset();
		});
	mGui->add(resetButton);

	// Redo Allボタン
	tmpImage = mGame->LoadTexture("Assets/RedoAll.png")->copyToImage();
	tmpTGuiTex.loadFromPixelData(tmpImage.getSize(), tmpImage.getPixelsPtr());
	auto redoAllButton = tgui::BitmapButton::create("Redo All");
	redoAllButton->setSize(mRollBackSmallButtonSize.x, mRollBackSmallButtonSize.y);    // ボタンのサイズ
	// ボタンの位置
	redoAllButton->setPosition(
		rollbackButtonInitialPos.x - mRollBackLargeButtonSize.x * 2.0 - mRollBackSmallButtonSize.x * 2.0f - rollbackLargeButtonMargin.x * 4.0f - rollbackSmallButtonMargin.x * 3.0f,
		rollbackButtonInitialPos.y + rollbackSmallButtonMargin.y);
	redoAllButton->setRenderer(mTheme->getRenderer("BitmapButton"));
	redoAllButton->setImage(tmpTGuiTex);
	redoAllButton->setImageScaling(mIconImageScale);
	redoAllButton->setTextSize(16);
	redoAllButton->onPress([=]() {
		std::cout << "Redo all action triggered!" << std::endl;
		mGame->CallRedoAll();
		});
	mGui->add(redoAllButton);

	// Load Snapshotボタン
	auto loadSnapshotButton = tgui::Button::create("Load Snapshot");
	loadSnapshotButton->setSize(mRollBackLargeButtonSize.x, mRollBackLargeButtonSize.y);    // ボタンのサイズ
	// ボタンの位置
	loadSnapshotButton->setPosition(
		mGame->GetBoardViewArea().first.x + rollbackLargeButtonMargin.x,
		rollbackButtonInitialPos.y + rollbackLargeButtonMargin.y);
	loadSnapshotButton->setRenderer(mTheme->getRenderer("Button"));
	loadSnapshotButton->setTextSize(16);
	loadSnapshotButton->onPress([=]() {
		std::cout << "Load Snapshot action triggered!" << std::endl;
		// スナップショットマネージャーを呼び出す
		mGame->DisplaySnapshot();
		});
	mGui->add(loadSnapshotButton);

	// Add Snapshotボタン
	auto addSnapshotButton = tgui::Button::create("Add Snapshot");
	addSnapshotButton->setSize(mRollBackLargeButtonSize.x, mRollBackLargeButtonSize.y);    // ボタンのサイズ
	// ボタンの位置
	addSnapshotButton->setPosition(
		mGame->GetBoardViewArea().first.x + mRollBackLargeButtonSize.x + rollbackLargeButtonMargin.x * 3.0f,
		rollbackButtonInitialPos.y + rollbackLargeButtonMargin.y);
	addSnapshotButton->setRenderer(mTheme->getRenderer("Button"));
	addSnapshotButton->setTextSize(16);
	addSnapshotButton->onPress([=]() {
		std::cout << "Add Snapshot action triggered!" << std::endl;
		// スナップショットをリストに追加する処理を呼び出す
		mGame->AddSnapshotDialog();
		});
	mGui->add(addSnapshotButton);

	// Undo/Redo操作関連のボタンの配置ここまで
	
	// 以下はゲームシステム関連のボタンの配置
	// 各ボタンの基準位置のマージン
	sf::Vector2i listBoxMargin{ static_cast<int>((mGame->GetWindowSize().x - (mGame->GetBoardViewArea().second.x - mGame->GetBoardViewArea().first.x) - (mGameSystemButtonSize.x * 4 + mButtonMergin.x * 3)) / 2.0f), static_cast<int>((mGame->GetWindowSize().x - (mGame->GetBoardViewArea().second.x - mGame->GetBoardViewArea().first.x) - (mGameSystemButtonSize.x * 4 + mButtonMergin.x * 3)) / 4.0f) };

	// Load Boardボタン
	auto loadBoardButton = tgui::Button::create("Load Board");
	loadBoardButton->setPosition(listBoxMargin.x, mButtonInitialPos.y); // ボタンの位置
	loadBoardButton->setSize(mGameSystemButtonSize.x, mGameSystemButtonSize.y);    // ボタンのサイズ
	loadBoardButton->setRenderer(mTheme->getRenderer("Button"));
	loadBoardButton->setTextSize(16);
	loadBoardButton->onPress([=]() {
		std::cout << "Load board action triggered!" << std::endl;
		mGame->SelectBoards();
		});
	mGui->add(loadBoardButton);

	// Generate Boardボタン
	auto genBoardButton = tgui::Button::create("Generate Board");
	genBoardButton->setPosition(listBoxMargin.x + (mGameSystemButtonSize.x + mButtonMergin.x), mButtonInitialPos.y); // ボタンの位置
	genBoardButton->setSize(mGameSystemButtonSize.x, mGameSystemButtonSize.y);    // ボタンのサイズ
	genBoardButton->setRenderer(mTheme->getRenderer("Button"));
	genBoardButton->setTextSize(16);
	genBoardButton->onPress([=]() {
		std::cout << "Generate board action triggered!" << std::endl;
		mGame->CallReload();
		});
	mGui->add(genBoardButton);
	
	// Save Boardボタン
	tmpImage = mGame->LoadTexture("Assets/SaveBoard.png")->copyToImage();
	tmpTGuiTex.loadFromPixelData(tmpImage.getSize(), tmpImage.getPixelsPtr());
	auto saveBoardButton = tgui::BitmapButton::create("Save Board");
	saveBoardButton->setPosition(listBoxMargin.x + (mGameSystemButtonSize.x + mButtonMergin.x) * 2, mButtonInitialPos.y); // ボタンの位置
	saveBoardButton->setSize(mGameSystemButtonSize.x, mGameSystemButtonSize.y);    // ボタンのサイズ
	saveBoardButton->setRenderer(mTheme->getRenderer("BitmapButton"));
	saveBoardButton->setImage(tmpTGuiTex);
	saveBoardButton->setImageScaling(mIconImageScale);
	saveBoardButton->setTextSize(16);
	saveBoardButton->onPress([=]() {
		std::cout << "Save board action triggered!" << std::endl;
		mGame->CallSave();
		});
	mGui->add(saveBoardButton);

	// Save Logボタン
	tmpImage = mGame->LoadTexture("Assets/SaveLog.png")->copyToImage();
	tmpTGuiTex.loadFromPixelData(tmpImage.getSize(), tmpImage.getPixelsPtr());
	auto saveLogButton = tgui::BitmapButton::create("Save Log");
	saveLogButton->setPosition(listBoxMargin.x + (mGameSystemButtonSize.x + mButtonMergin.x) * 3, mButtonInitialPos.y); // ボタンの位置
	saveLogButton->setSize(mGameSystemButtonSize.x, mGameSystemButtonSize.y);    // ボタンのサイズ
	saveLogButton->setRenderer(mTheme->getRenderer("BitmapButton"));
	saveLogButton->setImage(tmpTGuiTex);
	saveLogButton->setImageScaling(mIconImageScale);
	saveLogButton->setTextSize(16);
	saveLogButton->onPress([=]() {
		std::cout << "Save log action triggered!" << std::endl;
		mGame->OutputLogs();
		});
	mGui->add(saveLogButton);

	// Edit Modeボタン
	auto editModeButton = tgui::Button::create("Edit Mode");
	editModeButton->setPosition(listBoxMargin.x, mButtonInitialPos.y + (mGameSystemButtonSize.y + mButtonMergin.y)); // ボタンの位置
	editModeButton->setSize(mGameSystemButtonSize.x, mGameSystemButtonSize.y);    // ボタンのサイズ
	editModeButton->setRenderer(mTheme->getRenderer("Button"));
	editModeButton->setTextSize(16);
	editModeButton->onPress([=]() {
		std::cout << "Edit Mode action triggered!" << std::endl;
		mGame->DisplayEditorScreen();
		});
	mGui->add(editModeButton);

	// Helpボタン
	auto helpButton = tgui::Button::create("Help");
	helpButton->setPosition(listBoxMargin.x + (mGameSystemButtonSize.x + mButtonMergin.x) * 1, mButtonInitialPos.y + (mGameSystemButtonSize.y + mButtonMergin.y)); // ボタンの位置
	helpButton->setSize(mGameSystemButtonSize.x, mGameSystemButtonSize.y);    // ボタンのサイズ
	helpButton->setRenderer(mTheme->getRenderer("Button"));
	helpButton->setTextSize(16);
	helpButton->onPress([=]() {
		std::cout << "Help action triggered!" << std::endl;
		mGame->DisplayHelpWindow();
		});
	mGui->add(helpButton);

	// ゲームシステム関連のボタンの配置ここまで
	
	// TODO 各種情報のテキストを表示する矩形
	mListBoxSize = sf::Vector2i{ static_cast<int>(mGame->GetWindowSize().x - (mGame->GetBoardViewArea().second.x - mGame->GetBoardViewArea().first.x) - listBoxMargin.x * 2), static_cast<int>(mGame->GetWindowSize().y) - (mButtonInitialPos.y + (mGameSystemButtonSize.y + mButtonMergin.y) * 2) - 20};
	mListBoxPos = sf::Vector2i{ listBoxMargin.x, mButtonInitialPos.y + (mGameSystemButtonSize.y + mButtonMergin.y) * 2 };

	mTextInfo[TextIndex::EMoveCount] = "Moves : " + std::to_string(mGame->GetStep());
	mTextInfo[TextIndex::ETime] = "Time : 00:00";
	mTextInfo[TextIndex::EBoardState] = "Status : ";
	mTextInfo[TextIndex::EMovableBaggages] = "Movable bagggages : ";
	mTextInfo[TextIndex::EGoaledBaggages] = "Goaled baggages : ";
	mTextInfo[TextIndex::EDeadlockedBaggages] = "Deadlocked baggages : ";

	mListBox = tgui::ListBox::create();
	mListBox->setRenderer(mTheme->getRenderer("ListBox"));
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
	mTextInfo[TextIndex::EMoveCount] = "Moves : " + std::to_string(mGame->GetStep());
	long time = static_cast<long>(mGame->GetSecTime());
	std::stringstream m, s;
	m << std::setw(2) << std::setfill('0') << time / 60;
	s << std::setw(2) << std::setfill('0') << time % 60;
	mTextInfo[TextIndex::ETime] = "Time : " + m.str() + ":" + s.str();
	std::unordered_map<int, sf::Vector2i> deadlockedBaggages{ mGame->GetHUDHelper()->GetDeadlockedBaggages() },
		goaledBaggages{ mGame->GetHUDHelper()->GetGoaledBaggages() };
	std::vector<sf::Vector2i> movableBaggages{ mGame->GetHUDHelper()->GetCandidates() };
	if (!deadlockedBaggages.empty())
	{
		mTextInfo[TextIndex::EBoardState] = "Status : Deadlocked!";
	}
	else
	{
		mTextInfo[TextIndex::EBoardState] = "Status : ";
	}
	std::string tmpStr{};
	for (const auto& baggage : goaledBaggages)
	{
		tmpStr += "( " + std::to_string(baggage.second.x) + ", " + std::to_string(baggage.second.y) + ") ";
	}
	mTextInfo[TextIndex::EGoaledBaggages] = "Goaled baggages : " + tmpStr;
	tmpStr = "";
	for (const auto& baggage : movableBaggages)
	{
		tmpStr += "( " + std::to_string(baggage.x) + ", " + std::to_string(baggage.y) + ") ";
	}
	mTextInfo[TextIndex::EMovableBaggages] = "Movable bagggages : " + tmpStr;
	tmpStr = "";
	for (const auto& baggage : deadlockedBaggages)
	{
		tmpStr += "( " + std::to_string(baggage.second.x) + ", " + std::to_string(baggage.second.y) + ") ";
	}
	mTextInfo[TextIndex::EDeadlockedBaggages] = "Deadlocked baggages : " + tmpStr;
	tmpStr = "";

	for (const auto& text : mTextInfo)
	{
		mListBox->changeItemById(std::to_string(text.first), text.second);
	}
	// テキストの更新はここまで

	// --- UIボタンの長押しリピート処理 ---
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