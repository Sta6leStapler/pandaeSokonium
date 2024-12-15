#include "THUD.h"

THUD::THUD(class Game* game, sf::RenderWindow* window)
	: mGame(game)
	, mGui(std::make_unique<tgui::Gui>(*window))
	, mTheme(std::make_unique<tgui::Theme>("Assets/themes/Black.txt"))
	, mButtonSize(sf::Vector2i{ 120 , 40 })
	, mButtonInitialPos(sf::Vector2i{ 20, 30 })
	, mButtonMergin(sf::Vector2i{ 20, 20 })
	, mState(IUIScreen::UIState::EActive)
{
	mGame->PushUI(this);

	// �e�[�}��ݒ�
	tgui::Theme::setDefault();

	// �e��{�^����ǉ�
	// Undo�{�^��
	auto undoButton = tgui::Button::create("Undo");
	undoButton->setPosition(mButtonInitialPos.x, mButtonInitialPos.y); // �{�^���̈ʒu
	undoButton->setSize(mButtonSize.x, mButtonSize.y);    // �{�^���̃T�C�Y
	undoButton->setRenderer(mTheme->getRenderer("Button"));
	undoButton->setTextSize(16);
	undoButton->onPress([=]() {
		std::cout << "Undo action triggered!" << std::endl;
		mGame->CallUndo();
		});
	mGui->add(undoButton);

	// Redo�{�^��
	auto redoButton = tgui::Button::create("Redo");
	redoButton->setPosition(mButtonInitialPos.x + mButtonSize.x + mButtonMergin.x, mButtonInitialPos.y); // �{�^���̈ʒu
	redoButton->setSize(mButtonSize.x, mButtonSize.y);    // �{�^���̃T�C�Y
	redoButton->setRenderer(mTheme->getRenderer("Button"));
	redoButton->setTextSize(16);
	redoButton->onPress([=]() {
		std::cout << "Redo action triggered!" << std::endl;
		mGame->CallRedo();
		});
	mGui->add(redoButton);

	// Reset�{�^��
	auto resetButton = tgui::Button::create("Reset");
	resetButton->setPosition(mButtonInitialPos.x, mButtonInitialPos.y + mButtonSize.y + mButtonMergin.y); // �{�^���̈ʒu
	resetButton->setSize(mButtonSize.x, mButtonSize.y);    // �{�^���̃T�C�Y
	resetButton->setRenderer(mTheme->getRenderer("Button"));
	resetButton->setTextSize(16);
	resetButton->onPress([=]() {
		std::cout << "Reset action triggered!" << std::endl;
		mGame->CallReset();
		});
	mGui->add(resetButton);

	// Redo All�{�^��
	auto redoAllButton = tgui::Button::create("Redo All");
	redoAllButton->setPosition(mButtonInitialPos.x + mButtonSize.x + mButtonMergin.x, mButtonInitialPos.y + mButtonSize.y + mButtonMergin.y); // �{�^���̈ʒu
	redoAllButton->setSize(mButtonSize.x, mButtonSize.y);    // �{�^���̃T�C�Y
	redoAllButton->setRenderer(mTheme->getRenderer("Button"));
	redoAllButton->setTextSize(16);
	redoAllButton->onPress([=]() {
		std::cout << "Redo all action triggered!" << std::endl;
		mGame->CallRedoAll();
		});
	mGui->add(redoAllButton);

	// Save Board�{�^��
	auto saveBoardButton = tgui::Button::create("Save Board");
	saveBoardButton->setPosition(mButtonInitialPos.x, mButtonInitialPos.y + (mButtonSize.y + mButtonMergin.y) * 2); // �{�^���̈ʒu
	saveBoardButton->setSize(mButtonSize.x, mButtonSize.y);    // �{�^���̃T�C�Y
	saveBoardButton->setRenderer(mTheme->getRenderer("Button"));
	saveBoardButton->setTextSize(16);
	saveBoardButton->onPress([=]() {
		std::cout << "Save board action triggered!" << std::endl;
		mGame->CallSave();
		});
	mGui->add(saveBoardButton);

	// Save Log�{�^��
	auto saveLogButton = tgui::Button::create("Save Log");
	saveLogButton->setPosition(mButtonInitialPos.x + mButtonSize.x + mButtonMergin.x, mButtonInitialPos.y + (mButtonSize.y + mButtonMergin.y) * 2); // �{�^���̈ʒu
	saveLogButton->setSize(mButtonSize.x, mButtonSize.y);    // �{�^���̃T�C�Y
	saveLogButton->setRenderer(mTheme->getRenderer("Button"));
	saveLogButton->setTextSize(16);
	saveLogButton->onPress([=]() {
		std::cout << "Save log action triggered!" << std::endl;
		mGame->OutputLogs();
		});
	mGui->add(saveLogButton);

	// Load Board�{�^��
	auto loadBoardButton = tgui::Button::create("Load Board");
	loadBoardButton->setPosition(mButtonInitialPos.x + (mButtonSize.x + mButtonMergin.x) * 2, mButtonInitialPos.y); // �{�^���̈ʒu
	loadBoardButton->setSize(mButtonSize.x, mButtonSize.y);    // �{�^���̃T�C�Y
	loadBoardButton->setRenderer(mTheme->getRenderer("Button"));
	loadBoardButton->setTextSize(16);
	loadBoardButton->onPress([=]() {
		std::cout << "Load board action triggered!" << std::endl;
		mGame->SelectBoards();
		});
	mGui->add(loadBoardButton);
	
	// Generate Board�{�^��
	auto genBoardButton = tgui::Button::create("Generate Board");
	genBoardButton->setPosition(mButtonInitialPos.x + (mButtonSize.x + mButtonMergin.x) * 2, mButtonInitialPos.y + mButtonSize.y + mButtonMergin.y); // �{�^���̈ʒu
	genBoardButton->setSize(mButtonSize.x, mButtonSize.y);    // �{�^���̃T�C�Y
	genBoardButton->setRenderer(mTheme->getRenderer("Button"));
	genBoardButton->setTextSize(13);
	genBoardButton->onPress([=]() {
		std::cout << "Generate board action triggered!" << std::endl;
		mGame->CallReload();
		});
	mGui->add(genBoardButton);

	// Help�{�^��
	auto helpButton = tgui::Button::create("Help");
	helpButton->setPosition(mButtonInitialPos.x + (mButtonSize.x + mButtonMergin.x) * 2, mButtonInitialPos.y + (mButtonSize.y + mButtonMergin.y) * 2); // �{�^���̈ʒu
	helpButton->setSize(mButtonSize.x, mButtonSize.y);    // �{�^���̃T�C�Y
	helpButton->setRenderer(mTheme->getRenderer("Button"));
	helpButton->setTextSize(16);
	helpButton->onPress([=]() {
		std::cout << "Help action triggered!" << std::endl;
		mGame->DisplayHelpWindow();
		});
	mGui->add(helpButton);

	// TODO �e����̃e�L�X�g��\�������`
	mListBoxSize = sf::Vector2i{ static_cast<int>(mGame->GetWindowSize().x - mGame->GetWindowSize().y - 40.0), static_cast<int>(mGame->GetWindowSize().y) - (mButtonInitialPos.y + (mButtonSize.y + mButtonMergin.y) * 3) - 20 };
	mListBoxPos = sf::Vector2i{ 20, mButtonInitialPos.y + (mButtonSize.y + mButtonMergin.y) * 3 };
	mListBoxItemHeight = 32;

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
	// �e�L�X�g�\������e������X�V
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
	// GUI�̓��͏������s��
	mGui->handleEvent(*event);

}

void THUD::Draw(sf::RenderWindow* mWindow)
{
	// GUI�̕`��
	mGui->draw();
}