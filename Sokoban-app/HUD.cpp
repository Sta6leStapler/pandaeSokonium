#include "HUD.h"
#include <iomanip>
#include <sstream>

HUD::HUD(class Game* game)
	: mGame(game)
	, mFont(new Font())
	, mTitle(new sf::Texture())
	, mBackground(new sf::Texture())
	, mButtonOn(new sf::Texture())
	, mButtonOff(new sf::Texture())
	, mUndoTex(new sf::Texture())
	, mRedoTex(new sf::Texture())
	, mResetTex(new sf::Texture())
	, mRedoAllTex(new sf::Texture())
	, mSaveBoardTex(new sf::Texture())
	, mSaveLogTex(new sf::Texture())
	, mLoadBoardTex(new sf::Texture())
	, mGenerateBoardTex(new sf::Texture())
	, mHelpTex(new sf::Texture())
	, mButtons(std::vector<std::pair<Button*, sf::Texture*>>{})
	, mState(IUIScreen::UIState::EActive)
{
	mGame->PushUI(this);

	// TODO �e�N�X�`����Game�N���X�����荞�ނ悤�ɕς���
	// �t�H���g�����[�h���A�摜��ǂݍ���Ńe�N�X�`���ɂ��Ă���
	mFont->Load("Assets/fonts/arial.ttf");
	mButtonOn->loadFromFile("Assets/ButtonYellow.png");
	mButtonOff->loadFromFile("Assets/ButtonBlue.png");
	mUndoTex->loadFromFile("Assets/Undo.png");
	mRedoTex->loadFromFile("Assets/Redo.png");
	mResetTex->loadFromFile("Assets/Reset.png");
	mRedoAllTex->loadFromFile("Assets/RedoAll.png");
	mSaveBoardTex->loadFromFile("Assets/SaveBoard.png");
	mSaveLogTex->loadFromFile("Assets/SaveLog.png");

	// �e��{�^����ǉ�
	// TODO �{�^���̍��W���΍��W���瑊�΍��W�ɕς���
	// Note: SetTitle�֐���mFont->Load���Ăяo������
	SetTitle(" ");
	mTitlePos = sf::Vector2f{ 0.0f, 0.0f };
	mBGPos = sf::Vector2f{ 0.0f, 0.0f };
	// Undo�{�^��
	mUndoButtonPos = sf::Vector2f{ 20.0f, 40.0f };
	AddButton("Undo", mUndoButtonPos, [this]() {
		mGame->CallUndo();
		},
		mUndoTex);
	// Redo�{�^��
	mRedoButtonPos = sf::Vector2f{ mUndoButtonPos.x + static_cast<float>(mButtonOn->getSize().x) + 20.0f, 40.0f };
	AddButton("Redo", mRedoButtonPos, [this]() {
		mGame->CallRedo();
		},
		mRedoTex);
	// Reset�{�^��
	mResetButtonPos = sf::Vector2f{ 20.0f, mUndoButtonPos.y + static_cast<float>(mButtonOn->getSize().y) + 20.0f };
	AddButton("Reset", mResetButtonPos, [this]() {
		mGame->CallReset();
		},
		mResetTex);
	// Redo All�{�^��
	mRedoAllButtonPos = sf::Vector2f{ mResetButtonPos.x + static_cast<float>(mButtonOn->getSize().x) + 20.0f, mRedoButtonPos.y + static_cast<float>(mButtonOn->getSize().y) + 20.0f };
	AddButton("Redo All", mRedoAllButtonPos, [this]() {
		mGame->CallRedoAll();
		},
		mRedoAllTex);
	// Save Board�{�^��
	mSaveBoardButtonPos = sf::Vector2f{ 20.0f, mResetButtonPos.y + static_cast<float>(mButtonOn->getSize().y) + 20.0f };
	AddButton("Save Board", mSaveBoardButtonPos, [this]() {
		mGame->CallSave();
		},
		mSaveBoardTex,
		26);
	// Save Log�{�^��
	mSaveLogButtonPos = sf::Vector2f{ mSaveBoardButtonPos.x + static_cast<float>(mButtonOn->getSize().x) + 20.0f, mRedoAllButtonPos.y + static_cast<float>(mButtonOn->getSize().y) + 20.0f };
	AddButton("Save Log", mSaveLogButtonPos, [this]() {
		mGame->OutputLogs();
		},
		mSaveLogTex,
		26);
	// Load Board�{�^��
	mLoadBoardButtonPos = sf::Vector2f{ mRedoButtonPos.x + static_cast<float>(mButtonOn->getSize().x) + 20.0f, 40.0f };
	AddButton("Load Board", mLoadBoardButtonPos, [this]() {
		// TODO ��蒼��
		},
		mLoadBoardTex,
		26);
	// Generate Board�{�^��
	mGenerateBoardButtonPos = sf::Vector2f{ mRedoAllButtonPos.x + static_cast<float>(mButtonOn->getSize().x) + 20.0f, mLoadBoardButtonPos.y + static_cast<float>(mButtonOn->getSize().y) + 20.0f };
	AddButton("Generate Board", mGenerateBoardButtonPos, [this]() {
		mGame->CallReload();
		},
		mGenerateBoardTex,
		24);
	// Help�{�^��
	mHelpButtonPos = sf::Vector2f{ mSaveLogButtonPos.x + static_cast<float>(mButtonOn->getSize().x) + 20.0f, mGenerateBoardButtonPos.y + static_cast<float>(mButtonOn->getSize().y) + 20.0f };
	AddButton("Help", mHelpButtonPos, [this]() {
		mGame->DisplayHelpWindow();
		},
		mHelpTex);

	// �e����̃e�L�X�g��\�������`
	mTextInfoBox = new sf::RectangleShape(sf::Vector2f(mGame->GetWindowSize().x - mGame->GetWindowSize().y - 40, mGame->GetWindowSize().y - (mSaveBoardButtonPos.y + mButtonOn->getSize().y) - 40 ));
	mTextInfoBox->setPosition(20, mSaveBoardButtonPos.y + mButtonOn->getSize().y + 20); // �����ɔz�u
	mTextInfoBox->setFillColor(sf::Color(200, 200, 200)); // �����O���[

	// �e�L�X�g�̏����ݒ�i���G���A�̍��W����ɑ��΍��W�Őݒ�j
	mMoveCountText = new sf::Text();
	AddText(mMoveCountText, "Moves : " + std::to_string(mGame->GetStep()), mTextInfoBox->getPosition() + sf::Vector2f(10, 10));
	
	mTimeText = new sf::Text();
	AddText(mTimeText, "Time : 0:00", mTextInfoBox->getPosition() + sf::Vector2f(10, 40));

	mStateText = new sf::Text();
	AddText(mStateText, "Status : ", mTextInfoBox->getPosition() + sf::Vector2f(10, 70));
	
	mMovableBaggagesText = new sf::Text();
	AddText(mMovableBaggagesText, "Movable bagggages : ", mTextInfoBox->getPosition() + sf::Vector2f(10, 100));

	mGoaledBaggagesText = new sf::Text();
	AddText(mGoaledBaggagesText, "Goaled baggages : ", mTextInfoBox->getPosition() + sf::Vector2f(10, 130));

	mDeadlockedBaggagesText = new sf::Text();
	AddText(mDeadlockedBaggagesText, "Deadlocked baggages : ", mTextInfoBox->getPosition() + sf::Vector2f(10, 160));
}

HUD::~HUD()
{
	// Button�N���X�̃����o��Font�N���X�̃����o�ϐ����܂܂�邽�߁A��������ɏ���
	while (!mButtons.empty())
	{
		delete mButtons.back().first;
		delete mButtons.back().second;
		mButtons.pop_back();
	}
	delete mFont;
	delete mButtonOn;
	delete mButtonOff;
	while (!mTextInfoes.empty())
	{
		delete mTextInfoes.back();
		mTextInfoes.pop_back();
	}
	mGame->SetState(Game::GameState::EGamePlay);
}

void HUD::Update(float deltaTime)
{
	// �e�L�X�g�\������e������X�V
	mMoveCountText->setString("Moves : " + std::to_string(mGame->GetStep()));
	long time = static_cast<long>(mGame->GetSecTime());
	std::stringstream m, s;
	m << std::setw(2) << std::setfill('0') << time / 60;
	s << std::setw(2) << std::setfill('0') << time % 60;
	mTimeText->setString("Time : " + m.str() + ":" + s.str());

	std::unordered_map<int, sf::Vector2i> deadlockedBaggages{ mGame->GetHUDHelper()->GetDeadlockedBaggages() },
		goaledBaggages{ mGame->GetHUDHelper()->GetGoaledBaggages() };
	std::vector<sf::Vector2i> movableBaggages{ mGame->GetHUDHelper()->GetCandidates() };
	if (!deadlockedBaggages.empty())
	{
		mStateText->setString("Status : Deadlocked!");
		mStateText->setFillColor(sf::Color::Red);
	}
	else
	{
		mStateText->setString("Status : ");
		mStateText->setFillColor(sf::Color::Black);
	}

	std::string tmpStr{};
	for (const auto& baggage : goaledBaggages)
	{
		tmpStr += "( " + std::to_string(baggage.second.x) + ", " + std::to_string(baggage.second.y) + ") ";
	}
	mGoaledBaggagesText->setString("Goaled baggages : " + tmpStr);
	tmpStr = "";

	for (const auto& baggage : movableBaggages)
	{
		tmpStr += "( " + std::to_string(baggage.x) + ", " + std::to_string(baggage.y) + ") ";
	}
	mMovableBaggagesText->setString("Movable baggages : " + tmpStr);
	tmpStr = "";

	for (const auto& baggage : deadlockedBaggages)
	{
		tmpStr += "( " + std::to_string(baggage.second.x) + ", " + std::to_string(baggage.second.y) + ") ";
	}
	mDeadlockedBaggagesText->setString("Deadlocked baggages : " + tmpStr);
	tmpStr = "";
}

void HUD::Draw(sf::RenderWindow* mWindow)
{
	// �w�i��ǉ�����ꍇ�͂����`��
	if (mBackground)
	{
		DrawTexture(mWindow, mBackground, mBGPos);
	}
	// �^�C�g�������݂���ꍇ�͂����`��
	if (mTitle)
	{
		DrawTexture(mWindow, mTitle, mTitlePos);
	}
	// �{�^���{�̂̕`��
	for (auto b : mButtons)
	{
		// �{�^���̔w�i��`��
		sf::Texture* tex = b.first->GetHighlighted() ? mButtonOn : mButtonOff;
		DrawTexture(mWindow, tex, b.first->GetPosition());
		// �{�^���̃A�C�R���̕`��
		DrawTexture(mWindow, b.second, sf::Vector2f{ b.first->GetPosition().x + static_cast<float>(tex->getSize().x - b.second->getSize().x), b.first->GetPosition().y});
		// �{�^���̃e�L�X�g��`��
		DrawTexture(mWindow, b.first->GetNameTex(), b.first->GetPosition());
	}

	// �e�L�X�g�E�B���h�E�̕\��
	mWindow->draw(*mTextInfoBox);
	for (auto t : mTextInfoes)
	{
		mWindow->draw(*t);
	}
}

void HUD::ProcessInput(const sf::Event* event, const sf::Vector2i& mousePos)
{
	// �{�^�������邩�H
	if (!mButtons.empty())
	{
		// �}�E�X�̈ʒu���擾
		sf::Vector2f mouse_pos{ static_cast<float>(mousePos.x), static_cast<float>(mousePos.y) };

		// �{�^���̋���
		for (auto b : mButtons)
		{
			if (b.first->ContainsPoint(mouse_pos))
			{
				b.first->SetHighlighted(true);
			}
			else
			{
				b.first->SetHighlighted(false);
			}
		}
	}

	// �{�^����Ń}�E�X���{�^���������ꂽ��A���̃{�^���̏������s��
	switch (event->mouseButton.button)
	{
	case sf::Mouse::Left:
		// �A�N�e�B�u�ȃ{�^�������邩���ׂ�
		if (!mButtons.empty())
		{
			for (auto b : mButtons)
			{
				if (b.first->GetHighlighted())
				{
					b.first->OnClick();
					break;
				}
			}
		}
		break;
	default:
		break;
	}
}

void HUD::SetTitle(const std::string& text, const sf::Color& color /*= sf::Color::White*/, int pointSize /*= 40*/)
{
	mTitle = mFont->RenderText(text, color, pointSize);
}

void HUD::AddButton(const std::string& name, const sf::Vector2f& button_pos, std::function<void()> onClick, sf::Texture* iconTexture, int pointSize, const sf::Color& color)
{
	// TODO �{�^���̍��W���΍��W���瑊�΍��W�ɕς���
	sf::Vector2f dims{ static_cast<float>(mButtonOn->getSize().x), static_cast<float>(mButtonOn->getSize().y) };
	Button* b = new Button(name, mFont, onClick, button_pos, dims);
	b->SetNameCenter(name, dims, color, pointSize);
	mButtons.emplace_back(std::pair<Button*, sf::Texture*>{ b, iconTexture });
}

void HUD::AddText(sf::Text* text, const std::string& textStr, const sf::Vector2f& pos, const unsigned int& charSize, const sf::Color& color)
{
	text->setFont(*mFont->GetFont());
	text->setString(textStr);
	text->setCharacterSize(charSize);
	text->setPosition(pos);
	text->setFillColor(color);
	mTextInfoes.emplace_back(text);
}

void HUD::DrawTexture(sf::RenderWindow* mWindow, class sf::Texture* texture, const sf::Vector2f& offset /*= sf::Vector2f{ 0.0f, 0.0f }*/, float scale /*= 1.0f*/)
{
	sf::Sprite spr;
	spr.setTexture(*texture);

	// �X�v���C�g�̃X�P�[�����O�ƈړ����s��
	spr.setScale(scale, scale);
	spr.setPosition(offset);

	mWindow->draw(spr);
}