#include "THUD.h"

#include "Game.h"
#include "HUDHelper.h"

#include <iostream>

THUD::THUD(Game* game, sf::RenderWindow* window)
	: mGame(game)
	, mGui(std::make_unique<tgui::Gui>(*window))
	, mTheme(std::make_unique<tgui::Theme>("Assets/themes/Black.txt"))
	, mButtonSize(sf::Vector2i{ 150 , 40 })
	, mButtonInitialPos(sf::Vector2i{ 20, 30 })
	, mButtonMergin(sf::Vector2i{ 20, 20 })
	, mListBoxItemHeight(32)
	, mIconImageScale(0.7f)
	, mState(IUIScreen::UIState::EActive)
{
	mGame->PushUI(this);

	// テーマを設定
	tgui::Theme::setDefault();

	// 各種ボタンを追加
	// Undoボタン;
	sf::Image tmpImage = mGame->LoadTexture("Assets/Undo.png")->copyToImage();
	tgui::Texture tmpTGuiTex{};
	tmpTGuiTex.loadFromPixelData(tmpImage.getSize(), tmpImage.getPixelsPtr());
	auto undoButton = tgui::BitmapButton::create("Undo");
	undoButton->setPosition(mButtonInitialPos.x, mButtonInitialPos.y); // ボタンの位置
	undoButton->setSize(mButtonSize.x, mButtonSize.y);    // ボタンのサイズ
	undoButton->setRenderer(mTheme->getRenderer("BitmapButton"));
	undoButton->setImage(tmpTGuiTex);
	undoButton->setImageScaling(mIconImageScale);
	undoButton->setTextSize(16);
	undoButton->onPress([=]() {
		std::cout << "Undo action triggered!" << std::endl;
		mGame->CallUndo();
		});
	mGui->add(undoButton);

	// Redoボタン
	tmpImage = mGame->LoadTexture("Assets/Redo.png")->copyToImage();
	tmpTGuiTex.loadFromPixelData(tmpImage.getSize(), tmpImage.getPixelsPtr());
	auto redoButton = tgui::BitmapButton::create("Redo");
	redoButton->setPosition(mButtonInitialPos.x + mButtonSize.x + mButtonMergin.x, mButtonInitialPos.y); // ボタンの位置
	redoButton->setSize(mButtonSize.x, mButtonSize.y);    // ボタンのサイズ
	redoButton->setRenderer(mTheme->getRenderer("BitmapButton"));
	redoButton->setImage(tmpTGuiTex);
	redoButton->setImageScaling(mIconImageScale);
	redoButton->setTextSize(16);
	redoButton->onPress([=]() {
		std::cout << "Redo action triggered!" << std::endl;
		mGame->CallRedo();
		});
	mGui->add(redoButton);

	// Resetボタン
	tmpImage = mGame->LoadTexture("Assets/Reset.png")->copyToImage();
	tmpTGuiTex.loadFromPixelData(tmpImage.getSize(), tmpImage.getPixelsPtr());
	auto resetButton = tgui::BitmapButton::create("Reset");
	resetButton->setPosition(mButtonInitialPos.x, mButtonInitialPos.y + mButtonSize.y + mButtonMergin.y); // ボタンの位置
	resetButton->setSize(mButtonSize.x, mButtonSize.y);    // ボタンのサイズ
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
	redoAllButton->setPosition(mButtonInitialPos.x + mButtonSize.x + mButtonMergin.x, mButtonInitialPos.y + mButtonSize.y + mButtonMergin.y); // ボタンの位置
	redoAllButton->setSize(mButtonSize.x, mButtonSize.y);    // ボタンのサイズ
	redoAllButton->setRenderer(mTheme->getRenderer("BitmapButton"));
	redoAllButton->setImage(tmpTGuiTex);
	redoAllButton->setImageScaling(mIconImageScale);
	redoAllButton->setTextSize(16);
	redoAllButton->onPress([=]() {
		std::cout << "Redo all action triggered!" << std::endl;
		mGame->CallRedoAll();
		});
	mGui->add(redoAllButton);

	// Save Boardボタン
	tmpImage = mGame->LoadTexture("Assets/SaveBoard.png")->copyToImage();
	tmpTGuiTex.loadFromPixelData(tmpImage.getSize(), tmpImage.getPixelsPtr());
	auto saveBoardButton = tgui::BitmapButton::create("Save Board");
	saveBoardButton->setPosition(mButtonInitialPos.x, mButtonInitialPos.y + (mButtonSize.y + mButtonMergin.y) * 2); // ボタンの位置
	saveBoardButton->setSize(mButtonSize.x, mButtonSize.y);    // ボタンのサイズ
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
	saveLogButton->setPosition(mButtonInitialPos.x + mButtonSize.x + mButtonMergin.x, mButtonInitialPos.y + (mButtonSize.y + mButtonMergin.y) * 2); // ボタンの位置
	saveLogButton->setSize(mButtonSize.x, mButtonSize.y);    // ボタンのサイズ
	saveLogButton->setRenderer(mTheme->getRenderer("BitmapButton"));
	saveLogButton->setImage(tmpTGuiTex);
	saveLogButton->setImageScaling(mIconImageScale);
	saveLogButton->setTextSize(16);
	saveLogButton->onPress([=]() {
		std::cout << "Save log action triggered!" << std::endl;
		mGame->OutputLogs();
		});
	mGui->add(saveLogButton);

	// Load Boardボタン
	auto loadBoardButton = tgui::Button::create("Load Board");
	loadBoardButton->setPosition(mButtonInitialPos.x + (mButtonSize.x + mButtonMergin.x) * 2, mButtonInitialPos.y); // ボタンの位置
	loadBoardButton->setSize(mButtonSize.x, mButtonSize.y);    // ボタンのサイズ
	loadBoardButton->setRenderer(mTheme->getRenderer("Button"));
	loadBoardButton->setTextSize(16);
	loadBoardButton->onPress([=]() {
		std::cout << "Load board action triggered!" << std::endl;
		mGame->SelectBoards();
		});
	mGui->add(loadBoardButton);
	
	// Generate Boardボタン
	auto genBoardButton = tgui::Button::create("Generate Board");
	genBoardButton->setPosition(mButtonInitialPos.x + (mButtonSize.x + mButtonMergin.x) * 2, mButtonInitialPos.y + mButtonSize.y + mButtonMergin.y); // ボタンの位置
	genBoardButton->setSize(mButtonSize.x, mButtonSize.y);    // ボタンのサイズ
	genBoardButton->setRenderer(mTheme->getRenderer("Button"));
	genBoardButton->setTextSize(16);
	genBoardButton->onPress([=]() {
		std::cout << "Generate board action triggered!" << std::endl;
		mGame->CallReload();
		});
	mGui->add(genBoardButton);

	// Helpボタン
	auto helpButton = tgui::Button::create("Help");
	helpButton->setPosition(mButtonInitialPos.x + (mButtonSize.x + mButtonMergin.x) * 2, mButtonInitialPos.y + (mButtonSize.y + mButtonMergin.y) * 2); // ボタンの位置
	helpButton->setSize(mButtonSize.x, mButtonSize.y);    // ボタンのサイズ
	helpButton->setRenderer(mTheme->getRenderer("Button"));
	helpButton->setTextSize(16);
	helpButton->onPress([=]() {
		std::cout << "Help action triggered!" << std::endl;
		mGame->DisplayHelpWindow();
		});
	mGui->add(helpButton);

	// TODO 各種情報のテキストを表示する矩形
	mListBoxSize = sf::Vector2i{ static_cast<int>(mGame->GetWindowSize().x - mGame->GetWindowSize().y - 40.0), static_cast<int>(mGame->GetWindowSize().y) - (mButtonInitialPos.y + (mButtonSize.y + mButtonMergin.y) * 3) - 20 };
	mListBoxPos = sf::Vector2i{ 20, mButtonInitialPos.y + (mButtonSize.y + mButtonMergin.y) * 3 };

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