#include "Game.h"
#include <iostream>
#include <fstream>
#include <chrono>
#include <ctime>

Game::Game()
	: mWindow(nullptr)
	, mIsRunning(true)
	, mIsComplete(false)
	, mUpdatingActors(false)
	, mWindowSize(1600.0, 900.0)
	, mInputCooldown(0.0f)
	, mBoardSize(sf::Vector2i{ 9, 9 })
	, mBaggageNum(6)
	, mRepetition01(1)
	, mRepetition02(10)
	, mRepetition03(0.0)
	, mRepetition04(0.0)
	, mRepetition05(0)
	, mStep(0)
{

}

bool Game::Initialize()
{
	// �E�B���h�E�̍쐬
	sf::RenderWindow* window = new sf::RenderWindow(sf::VideoMode(static_cast<unsigned int>(mWindowSize.x), static_cast<unsigned int>(mWindowSize.y)), "SFML Window");
	mWindow = window;
	
	// �t�@�C���̓ǂݍ���
	LoadData();

	mClock.restart();

	mTicksCount = mClock.getElapsedTime();

	return true;
}

void Game::RunLoop()
{
	while (mIsRunning)
	{
		ProcessInput();
		UpdateGame();
		GenerateOutput();
	}
}

void Game::LoadData()
{
	// �t�H���g�����[�h���A������
	if (!mFont.loadFromFile("Assets/fonts/arial.ttf"))
	{
		std::cout << "error : failed loading font." << std::endl;
	}
	mInfoTxt.setFont(mFont);
	mInfoTxt.setCharacterSize(mWindow->getSize().y);
	mInfoTxt.setScale(20.0f / static_cast<float>(mWindow->getSize().y), 20.0f / static_cast<float>(mWindow->getSize().y));

	// �Ֆʃf�[�^��ǂݎ��
	/*
	std::string filename = "Assets/board.txt";
	mFilenames.push_back(filename);
	std::ifstream file(filename);
	if (!file.is_open())
	{
		std::cerr << "Failed to open file" << std::endl;
		return;
	}

	std::vector<std::string> lines;
	std::string line;
	while (std::getline(file, line))
	{
		lines.push_back(line);
	}

	mBoardData.emplace(filename, lines);
	file.close();
	//*/

	// ���������̏ꍇ
	// MySolution
	//*
	InputBoardData();
	MySolution* gen = new MySolution(mBoardSize, mBaggageNum, mRepetition01, mRepetition02, mRepetition03, mRepetition04, mRepetition05);
	std::vector<std::string> lines = gen->GetBoard();
	delete(gen);
	mCurrentKey = GetDateTime();
	mFilenames.push_back(mCurrentKey);
	mBoardData.emplace(mCurrentKey, lines);
	mInitBoardData.emplace(mCurrentKey, lines);
	//*/

	// �Ֆʂ̏�����Ԃ��Z�b�g
	std::vector<sf::Vector2i> mBoxesPos;
	{
		int i = 0, j = 0;
		std::string tmpstr = "";
		for (const auto& line : lines)
		{
			
			for (const auto& item : line)
			{
				switch (item)
				{
				case ' ':
					tmpstr += ' ';
					break;
				case '#':
					tmpstr += '#';
					break;
				case '$':
					tmpstr += ' ';
					mBoxesPos.push_back(sf::Vector2i(j, i));
					break;
				case '.':
					tmpstr += '.';
					mGoalPos.push_back(sf::Vector2i(j, i));
					break;
				case '*':
					tmpstr += '.';
					mBoxesPos.push_back(sf::Vector2i(j, i));
					mGoalPos.push_back(sf::Vector2i(j, i));
					break;
				case '@':
					tmpstr += ' ';
					mInitialPlayerPos = sf::Vector2i(j, i);
					break;
				case '+':
					tmpstr += '.';
					mInitialPlayerPos = sf::Vector2i(j, i);
					mGoalPos.push_back(sf::Vector2i(j, i));
					break;
				default:
					break;
				}
				j++;
			}
			mBoardState.push_back(tmpstr);
			tmpstr = "";
			i++;
			j = 0;
		}
	}

	// �w�i�̍쐬
	// �w�i�͉�ʂ̒����ɔz�u
	mBackGround = new BackGround(this);
	mBackGround->SetPosition(sf::Vector2f(mWindow->getSize().x / 2.0f, mWindow->getSize().y / 2.0f));

	// �{�[�h���쐬
	new GameBoard(this);

	// �v���C���[�̍쐬
	new Player(this);

	// �ו����쐬
	// �ו��̏������W�𓾂�
	for (const auto& item : mBoxesPos)
	{
		new Baggage(this, item);
		mInitialBaggagePos.emplace(mBaggages.back(), item);
	}

}

void Game::UnloadData()
{
	// �A�N�^�[���폜
	delete mBackGround;
	delete mGameBoard;
	delete mPlayer;
	while (!mBaggages.empty())
	{
		delete mBaggages.back();
	}

	// �e�N�X�`�����폜
	for (auto& item : mTextures)
	{
		delete item.second;
	}
	mTextures.clear();
}

void Game::ProcessInput()
{
	sf::Event event;
	while (mWindow->pollEvent(event))
	{
		switch (event.type)
		{
		case sf::Event::Closed:
			mIsRunning = false;
			break;
		}
	}

	// �Q�[���S�̂Ɋւ���e����͏���
	// Esc�L�[�ŃQ�[���I��
	if (sf::Keyboard::isKeyPressed(sf::Keyboard::Escape))
	{
		mIsRunning = false;
	}

	if (mInputCooldown <= 0.0f) {
		// Ctrl + z ��undo����
		if (sf::Keyboard::isKeyPressed(sf::Keyboard::LControl) && sf::Keyboard::isKeyPressed(sf::Keyboard::Z))
		{
			mInputCooldown = 0.13f;
			CallUndo();
		}

		// Ctrl + y ��redo����
		if (sf::Keyboard::isKeyPressed(sf::Keyboard::LControl) && sf::Keyboard::isKeyPressed(sf::Keyboard::Y))
		{
			mInputCooldown = 0.13f;
			CallRedo();
		}

		// PGUP�ōŐV�̏�Ԃɂ���
		if (sf::Keyboard::isKeyPressed(sf::Keyboard::PageUp))
		{
			while (mLogs.size() > mStep)
			{
				CallRedo();
			}
		}

		// PGDN�ŏ�����Ԃɂ���
		if (sf::Keyboard::isKeyPressed(sf::Keyboard::PageDown))
		{
			while (mStep > 0)
			{
				CallUndo();
			}
		}

		// Ctrl + s �Ō��݂̔Ֆʂ��Z�[�u
		if (sf::Keyboard::isKeyPressed(sf::Keyboard::LControl) && sf::Keyboard::isKeyPressed(sf::Keyboard::S))
		{
			mInputCooldown = 0.13f;
			CallSave();
		}

		// H �Ńw���v��ʂ̕\��
		if (sf::Keyboard::isKeyPressed(sf::Keyboard::H))
		{
			mInputCooldown = 0.13f;
			DisplayHelpWindow();
		}

		// F5 �ŔՖʂ̃����[�h�i���������̔ՖʂȂ�V���ȔՖʂ̐����j
		if (sf::Keyboard::isKeyPressed(sf::Keyboard::F5))
		{
			mInputCooldown = 0.13f;
			CallReload();
		}
	}

	// �S�Ă�Actor�̓��͏������s��
	mUpdatingActors = true;
	
	mGameBoard->ProcessInput(&event.key);
	mBackGround->ProcessInput(&event.key);
	mPlayer->ProcessInput(&event.key);
	for (auto& item : mBaggages)
	{
		item->ProcessInput(&event.key);
	}
	
	mUpdatingActors = false;
}

void Game::UpdateGame()
{
	// �f���^�^�C�����v�Z����
	// �Ŋ��̃t���[������16ms�����҂�
	while (mClock.getElapsedTime().asMilliseconds() - mTicksCount.asMilliseconds() < 16);

	float deltaTime = static_cast<float>(mClock.getElapsedTime().asMilliseconds() - mTicksCount.asMilliseconds()) / 1000.0f;
	if (deltaTime > 0.05f)
	{
		deltaTime = 0.05f;
	}
	mTicksCount = mClock.getElapsedTime();
	mInputCooldown -= deltaTime;

	// �S�ẴA�N�^�[���X�V
	mUpdatingActors = true;

	mGameBoard->Update(deltaTime);
	mBackGround->Update(deltaTime);
	mPlayer->Update(deltaTime);
	for (auto& item : mBaggages)
	{
		item->Update(deltaTime);
	}

	mUpdatingActors = false;

	// �ҋ@���̃A�N�^�[�����s�\��Ԃɂ���
	if (mPendingGameBoard != nullptr)
	{
		mGameBoard = mPendingGameBoard;
		mPendingGameBoard = nullptr;
	}
	if (mPendingBackGround != nullptr)
	{
		mBackGround = mPendingBackGround;
		mPendingBackGround = nullptr;
	}
	if (mPendingPlayer != nullptr)
	{
		mPlayer = mPendingPlayer;
		mPendingPlayer = nullptr;
	}
	for (auto item : mPendingBaggages)
	{
		mBaggages.push_back(item);
	}
	mPendingBaggages.clear();

	// ���񂾃A�N�^�[���폜����
	if (mGameBoard->GetState().GetEState() == State::EDead)
	{
		delete mGameBoard;
	}
	if (mBackGround->GetState().GetEState() == State::EDead)
	{
		delete mBackGround;
	}
	if (mPlayer->GetState().GetEState() == State::EDead)
	{
		delete mPlayer;
	}
	for (auto& item : mBaggages)
	{
		if (item->GetState().GetEState() == State::EDead)
		{
			delete item;
		}
	}
}

void Game::GenerateOutput()
{
	// �S�ẴX�v���C�g�����R���|�[�l���g��`��
	mWindow->clear();

	// �w�ʂ���`��
	mBackGroundComponent->Draw(mWindow);
	mGameBoardComponent->Draw(mWindow);
	mPlayerComponent->Draw(mWindow);
	for (const auto& item : mBaggageComponents)
	{
		item->Draw(mWindow);
	}

	// �Q�[���S�̂Ɋւ�����̕`��
	mGameInfo = "Steps : " + std::to_string(mStep);
	mInfoTxt.setString(mGameInfo);
	mWindow->draw(mInfoTxt);
	
	mWindow->display();

	// �Q�[���̏I��������Ăяo��
	HasComplete();

	if (mIsComplete)
	{
		// �ʒm�p�̃E�B���h�E���쐬
		sf::RenderWindow notificationWindow(sf::VideoMode(400, 200), "Notice");

		std::string message = "Congratulations!\nScore : " + std::to_string(mStep) + " steps";
		sf::Text text(message, mFont, 50);
		sf::FloatRect textRect = text.getLocalBounds();
		text.setOrigin(textRect.left + textRect.width / 2.0f, textRect.top + textRect.height / 2.0f);
		text.setPosition(sf::Vector2f(notificationWindow.getSize().x / 2.0f, notificationWindow.getSize().y / 2.0f));

		// �Q�[���E�B���h�E����̈ʒu��ݒ�
		notificationWindow.setPosition(sf::Vector2i(200, 200));

		// �Q�[�����[�v���~
		mWindow->setActive(false);

		// �ʒm�p�̃E�B���h�E��\��
		while (notificationWindow.isOpen())
		{
			sf::Event notificationEvent;
			while (notificationWindow.pollEvent(notificationEvent))
			{
				if (
					notificationEvent.type == sf::Event::Closed ||
					sf::Keyboard::isKeyPressed(sf::Keyboard::Escape) ||
					sf::Keyboard::isKeyPressed(sf::Keyboard::Enter) ||
					sf::Keyboard::isKeyPressed(sf::Keyboard::Space)
					)
				{
					// ���O���Z�[�u
					OutputLogs();
					notificationWindow.close();
					mIsRunning = false;
				}

				if (sf::Keyboard::isKeyPressed(sf::Keyboard::F5))
				{
					notificationWindow.close();
					CallReload();
				}
			}
			notificationWindow.clear();
			notificationWindow.draw(text);
			notificationWindow.display();
		}

		// �Q�[�����[�v���ĊJ
		mWindow->setActive(true);
	}
}

sf::Texture* Game::LoadTexture(const std::string& fileName)
{
	sf::Texture* tex = new sf::Texture();
	// �������O�̃e�N�X�`�������ɍ쐬����Ă��邩���ׂ�
	auto iter = mTextures.find(fileName);
	if (iter != mTextures.end())
	{
		tex = iter->second;
	}
	else
	{
		// �t�@�C�������[�h
		sf::Image* img = new sf::Image();

		if (!img->loadFromFile(fileName.c_str()))
		{
			std::cout << "Failed to load texture file " << fileName.c_str() << std::endl;
			return nullptr;
		}

		// �摜�f�[�^����e�N�X�`�����쐬
		if (!tex->loadFromImage(*img))
		{
			std::cout << "Failed to convert surface to texture for " << fileName.c_str() << std::endl;
			delete img;
			return nullptr;
		}
		delete img;

		mTextures.emplace(fileName.c_str(), tex);
	}
	return tex;
}

void Game::Shutdown()
{
	mWindow->close();
	UnloadData();
	delete mWindow;
}

// �Q�[���{�[�h�̒ǉ��ƍ폜
void Game::AddActor(GameBoard* gameboard)
{
	// �X�V���̃A�N�^�[������΁A�ҋ@���ɒǉ�����
	if (mUpdatingActors)
	{
		mPendingGameBoard = gameboard;
	}
	else
	{
		mGameBoard = gameboard;
	}
}

void Game::RemoveActor(GameBoard* gameboard)
{
	// �ҋ@���̃A�N�^�[�ɂ��邩�H
	if (mPendingGameBoard == gameboard)
	{
		mPendingGameBoard = nullptr;
	}
	// �A�N�^�[�ɂ��邩�H
	if (mGameBoard == gameboard)
	{
		mGameBoard = nullptr;
	}
}

// �w�i�̒ǉ��ƍ폜
void Game::AddActor(BackGround* background)
{
	// �X�V���̃A�N�^�[������΁A�ҋ@���ɒǉ�����
	if (mUpdatingActors)
	{
		mPendingBackGround = background;
	}
	else
	{
		mBackGround = background;
	}
}

void Game::RemoveActor(BackGround* background)
{
	// �ҋ@���̃A�N�^�[�ɂ��邩�H
	if (mPendingBackGround == background)
	{
		mPendingBackGround = nullptr;
	}
	// �A�N�^�[�ɂ��邩�H
	if (mBackGround == background)
	{
		mBackGround = nullptr;
	}
}

// �v���C���[�̒ǉ��ƍ폜
void Game::AddActor(Player* player)
{
	// �X�V���̃A�N�^�[������΁A�ҋ@���ɒǉ�����
	if (mUpdatingActors)
	{
		mPendingPlayer = player;
	}
	else
	{
		mPlayer = player;
	}
}

void Game::RemoveActor(Player* player)
{
	// �ҋ@���̃A�N�^�[�ɂ��邩�H
	if (mPendingPlayer == player)
	{
		mPendingPlayer = nullptr;
	}
	// �A�N�^�[�ɂ��邩�H
	if (mPlayer == player)
	{
		mPlayer = nullptr;
	}
}

// �ו��̒ǉ��ƍ폜
void Game::AddActor(Baggage* baggage)
{
	// �X�V���̃A�N�^�[������΁A�ҋ@���ɒǉ�����
	if (mUpdatingActors)
	{
		mPendingBaggages.push_back(baggage);
	}
	else
	{
		mBaggages.push_back(baggage);
	}
}

void Game::RemoveActor(Baggage* baggage)
{
	// �ҋ@�����H
	auto iter = std::find(mPendingBaggages.begin(), mPendingBaggages.end(), baggage);
	if (iter != mPendingBaggages.end())
	{
		std::iter_swap(iter, mPendingBaggages.end() - 1);
		mPendingBaggages.pop_back();
	}

	// �A�N�^�[�ɂ��邩�H
	iter = std::find(mBaggages.begin(), mBaggages.end(), baggage);
	if (iter != mBaggages.end())
	{
		std::iter_swap(iter, mBaggages.end() - 1);
		mBaggages.pop_back();
	}
}

// �X�v���C�g�̒ǉ��ƍ폜
void Game::AddSprite(class GameBoardComponent* GBComp)
{
	mGameBoardComponent = GBComp;
}

void Game::RemoveSprite(class GameBoardComponent* GBComp)
{
	mGameBoardComponent = nullptr;
}

void Game::AddSprite(class BackGroundComponent* BGComp)
{
	mBackGroundComponent = BGComp;
}

void Game::RemoveSprite(class BackGroundComponent* BGComp)
{
	mBackGroundComponent = nullptr;
}

void Game::AddSprite(class PlayerComponent* PComp)
{
	mPlayerComponent = PComp;
}

void Game::RemoveSprite(class PlayerComponent* PComp)
{
	mPlayerComponent = nullptr;
}

void Game::AddSprite(class BaggageComponent* BComp)
{
	int myDrawOrder = BComp->GetDrawOrder();
	auto iter = mBaggageComponents.begin();
	for (;
		iter != mBaggageComponents.end();
		++iter)
	{
		if (myDrawOrder < (*iter)->GetDrawOrder())
		{
			break;
		}
	}

	mBaggageComponents.insert(iter, BComp);
}

void Game::RemoveSprite(class BaggageComponent* BComp)
{
	auto iter = std::find(mBaggageComponents.begin(), mBaggageComponents.end(), BComp);
	mBaggageComponents.erase(iter);
}

void Game::CallUndo()
{
	// �ړ��̃��O������A�X�e�b�v����1�ȏォ
	if (mLogs.size() > 0 && mStep > 0)
	{
		// �ו����ړ����Ă�����
		if (!mLogs[mStep - 1].isBMoved)
		{
			// �v���C���[�̈ʒu��߂�
			if (mStep == 1)
			{
				mPlayer->SetBoardCoordinate(mInitialPlayerPos);
				mPlayer->SetDirection(Player::ESouth);
			}
			else if (mStep > 1)
			{
				mPlayer->SetBoardCoordinate(mLogs[mStep - 1].pCoordinate.first);
				mPlayer->SetDirection(mLogs[mStep - 1].direction2);
			}
		}
		else if (mLogs[mStep - 1].isBMoved)
		{
			// �Ō�̃X�e�b�v�ɊY������ו���T��
			Baggage* ptrBaggage = nullptr;
			for (const auto& item : mBaggages)
			{
				if (item->GetBoardCoordinate() == mLogs[mStep - 1].bCoordinate.second)
				{
					ptrBaggage = item;
					break;
				}
			}

			// �v���C���[�Ɖו��̈ʒu��߂�
			if (mStep == 1)
			{
				mPlayer->SetBoardCoordinate(mInitialPlayerPos);
				mPlayer->SetDirection(Player::ESouth);
				ptrBaggage->SetBoardCoordinate(mInitialBaggagePos[ptrBaggage]);
			}
			else if (mStep > 1)
			{
				mPlayer->SetBoardCoordinate(mLogs[mStep - 1].pCoordinate.first);
				mPlayer->SetDirection(mLogs[mStep - 1].direction2);
				ptrBaggage->SetBoardCoordinate(mLogs[mStep - 1].bCoordinate.first);
			}
		}
		mStep--;
		
	}
}

void Game::CallRedo()
{
	// ���O�̃T�C�Y�͌��݂̃X�e�b�v�����傫����
	if (mLogs.size() > mStep)
	{
		// �ו����ړ����Ă�����
		if (!mLogs[mStep].isBMoved)
		{
			// �v���C���[�̈ʒu��i�߂�
			mPlayer->SetBoardCoordinate(mLogs[mStep].pCoordinate.second);
			mPlayer->SetDirection(mLogs[mStep].direction2);
		}
		else if (mLogs[mStep].isBMoved)
		{
			// ���X�e�b�v�ɊY������ו���T��
			Baggage* ptrBaggage = nullptr;
			for (const auto& item : mBaggages)
			{
				if (item->GetBoardCoordinate() == mLogs[mStep].bCoordinate.first)
				{
					ptrBaggage = item;
					break;
				}
			}

			// �v���C���[�Ɖו��̈ʒu��i�߂�
			mPlayer->SetBoardCoordinate(mLogs[mStep].pCoordinate.second);
			mPlayer->SetDirection(mLogs[mStep].direction2);
			ptrBaggage->SetBoardCoordinate(mLogs[mStep].bCoordinate.second);
		}
		mStep++;
	}
}

void Game::CallSave()
{
	auto now = std::chrono::system_clock::now();
	std::time_t currentTime = std::chrono::system_clock::to_time_t(now);

	// ������ɕϊ�
	std::tm localTime;
	localtime_s(&localTime, &currentTime);
	char buffer[80];
	std::strftime(buffer, sizeof(buffer), "%Y%m%d %H%M%S", &localTime);
	std::string timeStr = buffer;

	std::string fileName = "Save/" + timeStr + ".txt";
	std::ofstream outputFile(fileName);

	if (outputFile.is_open())
	{
		for (const auto& str : mInitBoardData[mFilenames.front()])
		{
			outputFile << str << std::endl;
		}

		outputFile.close();
	}
	else
	{
		std::cerr << "Error: Unable to open file '" << fileName << "' for writing." << std::endl;
	}
}

void Game::CallReload()
{
	// ���O���t�@�C���o�͂��Ă���S�ď���
	OutputLogs();
	mLogs.clear();

	// �v���C���[�Ɖו��ƔՖʂ��X�V
	InputBoardData();
	MySolution* gen = new MySolution(mBoardSize, mBaggageNum, mRepetition01, mRepetition02, mRepetition03, mRepetition04, mRepetition05);
	std::vector<std::string> lines = gen->GetBoard();
	delete(gen);

	mFilenames.pop_back();
	mInitialPlayerPos = sf::Vector2i{ -1, -1 };
	mInitialBaggagePos.clear();
	mGoalPos.clear();
	mInitBoardData.erase(mCurrentKey);
	mBoardData.erase(mCurrentKey);
	mBoardState.clear();
	mCurrentKey.clear();
	mStep = 0;
	
	mCurrentKey = GetDateTime();
	mFilenames.emplace_back(mCurrentKey);
	mBoardData.emplace(mCurrentKey, lines);
	mInitBoardData.emplace(mCurrentKey, lines);

	// �Ֆʂ̏�����Ԃ��Z�b�g
	std::vector<sf::Vector2i> mBoxesPos;
	{
		int i = 0, j = 0;
		std::string tmpstr = "";
		for (const auto& line : lines)
		{

			for (const auto& item : line)
			{
				switch (item)
				{
				case ' ':
					tmpstr += ' ';
					break;
				case '#':
					tmpstr += '#';
					break;
				case '$':
					tmpstr += ' ';
					mBoxesPos.push_back(sf::Vector2i(j, i));
					break;
				case '.':
					tmpstr += '.';
					mGoalPos.push_back(sf::Vector2i(j, i));
					break;
				case '*':
					tmpstr += '.';
					mBoxesPos.push_back(sf::Vector2i(j, i));
					mGoalPos.push_back(sf::Vector2i(j, i));
					break;
				case '@':
					tmpstr += ' ';
					mInitialPlayerPos = sf::Vector2i(j, i);
					break;
				case '+':
					tmpstr += '.';
					mInitialPlayerPos = sf::Vector2i(j, i);
					mGoalPos.push_back(sf::Vector2i(j, i));
					break;
				default:
					break;
				}
				j++;
			}
			mBoardState.push_back(tmpstr);
			tmpstr = "";
			i++;
			j = 0;
		}
	}

	// �{�[�h���č\�z
	mGameBoard->Reload();

	// �v���C���[���č\�z
	mPlayer->Reload();

	// �ו����č\�z
	while (!mBaggages.empty())
	{
		delete mBaggages.back();
	}
	mBaggages.clear();

	for (const auto& item : mBoxesPos)
	{
		new Baggage(this, item);
		mInitialBaggagePos.emplace(mBaggages.back(), item);
	}
}

void Game::RemoveRedo()
{
	// ���݂̃X�e�b�v�����傫���X�e�b�v�̃��O�����ׂč폜
	std::vector<Log> subLogs(mLogs.begin() + mStep, mLogs.end());
	auto it = mLogs.begin();
	while (it != mLogs.end()) {
		if (it - mLogs.begin() >= mStep)
		{
			it = mLogs.erase(it);
		}
		else 
		{
			++it;
		}
	}

	// �����蒼���ꍇ�A����܂ł̌o�H�𐳉��̌o�H����h�������؍\���ɂ���
	if (!subLogs.empty())
	{
		if (mLogs.empty())
		{
			mLogs = subLogs;
		}
		else
		{
			mLogs.back().thread.push_back(subLogs);
		}
	}
}

void Game::AddLog(const sf::Vector2i& playerPos1, const sf::Vector2i& playerPos2, const Player::Direction& direction1, const Player::Direction& direction2)
{
	// ���݂̓��t�Ǝ������擾
	auto now = std::chrono::system_clock::now();
	auto now_ms = std::chrono::time_point_cast<std::chrono::milliseconds>(now);
	auto ms = now_ms.time_since_epoch().count() % 1000;  // �~���b�������擾

	// �N�����Ǝ����𕶎���Ƀt�H�[�}�b�g
	std::time_t now_time = std::chrono::system_clock::to_time_t(now);
	std::tm timeinfo;
	localtime_s(&timeinfo, &now_time);
	char buffer[24];
	std::strftime(buffer, sizeof(buffer), "%Y-%m-%d-%H-%M-%S", &timeinfo);

	// �~���b��3���\�����邽�߂Ƀp�f�B���O��ǉ�
	std::string ms_str = std::to_string(ms);
	ms_str = std::string(3 - ms_str.length(), '0') + ms_str;

	// �~���b��ǉ����ĕ������\��
	std::string datetime_str(buffer);
	datetime_str += "." + ms_str;

	Log tmpLog = { std::make_pair(playerPos1, playerPos2), std::make_pair(sf::Vector2i{ 0, 0 }, sf::Vector2i{ 0, 0 }), false, direction1, direction2, datetime_str };
	mLogs.push_back(tmpLog);
}

void Game::AddLog(const sf::Vector2i& playerPos1, const sf::Vector2i& playerPos2, const sf::Vector2i& baggagePos1, const sf::Vector2i& baggagePos2, const Player::Direction& direction1, const Player::Direction& direction2)
{
	// ���݂̓��t�Ǝ������擾
	auto now = std::chrono::system_clock::now();
	auto now_ms = std::chrono::time_point_cast<std::chrono::milliseconds>(now);
	auto ms = now_ms.time_since_epoch().count() % 1000;  // �~���b�������擾

	// �N�����Ǝ����𕶎���Ƀt�H�[�}�b�g
	std::time_t now_time = std::chrono::system_clock::to_time_t(now);
	std::tm timeinfo;
	localtime_s(&timeinfo, &now_time);
	char buffer[24];
	std::strftime(buffer, sizeof(buffer), "%Y-%m-%d-%H-%M-%S", &timeinfo);

	// �~���b��3���\�����邽�߂Ƀp�f�B���O��ǉ�
	std::string ms_str = std::to_string(ms);
	ms_str = std::string(3 - ms_str.length(), '0') + ms_str;

	// �~���b��ǉ����ĕ������\��
	std::string datetime_str(buffer);
	datetime_str += "." + ms_str;

	Log tmpLog = { std::make_pair(playerPos1, playerPos2), std::make_pair(baggagePos1, baggagePos2), true, direction1, direction2, datetime_str };
	mLogs.push_back(tmpLog);
}

void Game::OutputLogs()
{
	std::string datetime_str = GetDateTime(), filename = "Assets/Logs/" + datetime_str + ".txt";

	// ���O���o��
	std::ofstream outFile(filename);

	if (outFile.is_open())
	{
		outFile << ConvertLogToStr(mLogs, 0);
		outFile.close();
	}
}

std::string Game::GetDateTime()
{
	// ���t�Ǝ������擾
	// ���݂̓��t�Ǝ������擾
	auto now = std::chrono::system_clock::now();
	std::time_t now_time = std::chrono::system_clock::to_time_t(now);

	// �N�����Ǝ����𕶎���Ƀt�H�[�}�b�g
	std::tm timeinfo;
	localtime_s(&timeinfo, &now_time);
	char buffer[20];
	std::strftime(buffer, sizeof(buffer), "%Y-%m-%d-%H-%M-%S", &timeinfo);
	std::string datetime_str(buffer);

	return datetime_str;
}

std::string Game::ConvertLogToStr(const std::vector<Log>& logs, const unsigned long long& current)
{
	std::string result = "";

	result += "( ";
	for (unsigned long long i = 0; i < logs.size(); i++)
	{
		result += std::to_string(current + i) + "{ " + logs[i].time + " }" + " : [" + std::to_string(logs[i].pCoordinate.first.x) + ", " + std::to_string(logs[i].pCoordinate.first.y) + "] ";
		result += "[" + std::to_string(logs[i].pCoordinate.second.x) + ", " + std::to_string(logs[i].pCoordinate.second.y) + "] ";
		if (logs[i].isBMoved)
		{
			result += "[" + std::to_string(logs[i].bCoordinate.first.x) + ", " + std::to_string(logs[i].bCoordinate.first.y) + "] ";
			result += "[" + std::to_string(logs[i].bCoordinate.second.x) + ", " + std::to_string(logs[i].bCoordinate.second.y) + "] ";
		}

		if (!logs[i].thread.empty())
		{
			for (const auto& log : logs[i].thread)
			{
				result += " " + ConvertLogToStr(log, current);
			}
		}
	}
	result += " ) ";

	return result;
}

void Game::HasComplete()
{
	// �S�Ẳו����S�[����ɂ��邩���ׂ�
	// 1�ł��S�[����ɂȂ����̂�����΁A�Q�[�����p������
	mIsComplete = true;
	for (const auto& item : mBaggages)
	{
		auto it = std::find(mGoalPos.begin(), mGoalPos.end(), item->GetBoardCoordinate());

		if (it == mGoalPos.end())
		{
			mIsComplete = false;
			break;
		}
	}
}

void Game::InputBoardData()
{
	// ���͈ʒu�ƁA���͍���
	std::vector<std::string> values(8, "");
	values[0] = std::to_string(mBoardSize.x);
	values[1] = std::to_string(mBoardSize.y);
	values[2] = std::to_string(mBaggageNum);
	values[3] = std::to_string(mRepetition01);
	values[4] = std::to_string(mRepetition02);
	values[5] = std::to_string(mRepetition03);
	values[6] = std::to_string(mRepetition04);
	values[7] = std::to_string(mRepetition05);
	int currentPos = values[0].length() - 1, currentElem = 0;

	// ���͗p�̃E�B���h�E���쐬
	sf::RenderWindow inputPromptWindow(sf::VideoMode(640, 360), "Input Prompt");

	// �e���̓t�H�[��
	sf::Text inputWidth(" > Input width : " + std::to_string(mBoardSize.x), mFont, 36),
		inputHeight("   Input height : " + std::to_string(mBoardSize.y), mFont, 36),
		inputBaggageNum("   Input the number of baggage : " + std::to_string(mBaggageNum), mFont, 36),
		inputRepet1("   Input repetition rate1 : " + std::to_string(mRepetition01), mFont, 36),
		inputRepet2("   Input repetition rate2 : " + std::to_string(mRepetition02), mFont, 36),
		inputRepet3("   Input repetition rate3 : " + std::to_string(mRepetition03), mFont, 36),
		inputRepet4("   Input repetition rate4 : " + std::to_string(mRepetition04), mFont, 36),
		inputRepet5("   Input repetition rate5 : " + std::to_string(mRepetition05), mFont, 36);
	sf::FloatRect inputWidthRect = inputWidth.getLocalBounds(),
		inputHeightRect = inputHeight.getLocalBounds(),
		inputBaggageNumRect = inputBaggageNum.getLocalBounds(),
		inputRepet1Rect = inputRepet1.getLocalBounds(),
		inputRepet2Rect = inputRepet2.getLocalBounds(),
		inputRepet3Rect = inputRepet3.getLocalBounds(),
		inputRepet4Rect = inputRepet4.getLocalBounds(),
		inputRepet5Rect = inputRepet5.getLocalBounds();
	inputWidth.setOrigin(inputWidthRect.left + inputWidthRect.width / 2.0f, inputWidthRect.top + inputWidthRect.height / 2.0f);
	inputWidth.setPosition(sf::Vector2f{inputWidthRect.width / 2.0f, inputWidthRect.height / 2.0f + 10.0f });
	inputHeight.setOrigin(inputHeightRect.left + inputHeightRect.width / 2.0f, inputHeightRect.top + inputHeightRect.height / 2.0f);
	inputHeight.setPosition(sf::Vector2f{inputHeightRect.width / 2.0f, inputWidthRect.height + inputWidthRect.height / 2.0f + 20.0f });
	inputBaggageNum.setOrigin(inputBaggageNumRect.left + inputBaggageNumRect.width / 2.0f, inputBaggageNumRect.top + inputBaggageNumRect.height / 2.0f);
	inputBaggageNum.setPosition(sf::Vector2f{ inputBaggageNumRect.width / 2.0f, inputWidthRect.height + inputHeightRect.height + inputWidthRect.height / 2.0f + 30.0f });
	inputRepet1.setOrigin(inputRepet1Rect.left + inputRepet1Rect.width / 2.0f, inputRepet1Rect.top + inputRepet1Rect.height / 2.0f);
	inputRepet1.setPosition(sf::Vector2f{ inputRepet1Rect.width / 2.0f, inputWidthRect.height + inputHeightRect.height + inputBaggageNumRect.height + inputWidthRect.height / 2.0f + 40.0f });
	inputRepet2.setOrigin(inputRepet2Rect.left + inputRepet2Rect.width / 2.0f, inputRepet2Rect.top + inputRepet2Rect.height / 2.0f);
	inputRepet2.setPosition(sf::Vector2f{ inputRepet2Rect.width / 2.0f, inputWidthRect.height + inputHeightRect.height + inputBaggageNumRect.height + inputRepet1Rect.height + inputWidthRect.height / 2.0f + 50.0f });
	inputRepet3.setOrigin(inputRepet3Rect.left + inputRepet3Rect.width / 2.0f, inputRepet3Rect.top + inputRepet3Rect.height / 2.0f);
	inputRepet3.setPosition(sf::Vector2f{ inputRepet3Rect.width / 2.0f, inputWidthRect.height + inputHeightRect.height + inputBaggageNumRect.height + inputRepet1Rect.height + inputRepet2Rect.height + inputWidthRect.height / 2.0f + 60.0f });
	inputRepet4.setOrigin(inputRepet4Rect.left + inputRepet4Rect.width / 2.0f, inputRepet4Rect.top + inputRepet4Rect.height / 2.0f);
	inputRepet4.setPosition(sf::Vector2f{ inputRepet4Rect.width / 2.0f, inputWidthRect.height + inputHeightRect.height + inputBaggageNumRect.height + inputRepet1Rect.height + inputRepet2Rect.height + inputRepet3Rect.height + inputWidthRect.height / 2.0f + 70.0f });
	inputRepet5.setOrigin(inputRepet5Rect.left + inputRepet5Rect.width / 2.0f, inputRepet5Rect.top + inputRepet5Rect.height / 2.0f);
	inputRepet5.setPosition(sf::Vector2f{ inputRepet5Rect.width / 2.0f, inputWidthRect.height + inputHeightRect.height + inputBaggageNumRect.height + inputRepet1Rect.height + inputRepet2Rect.height + inputRepet3Rect.height + inputRepet4Rect.height + inputWidthRect.height / 2.0f + 80.0f });

	// �Q�[���E�B���h�E����̈ʒu��ݒ�
	inputPromptWindow.setPosition(sf::Vector2i(200, 200));

	// �Q�[�����[�v���~
	mWindow->setActive(false);

	// �f���^�^�C���֘A
	sf::Time ticksCount = mClock.getElapsedTime();
	
	// �L�[�{�[�h���͂̃N�[���_�E��
	std::unordered_map<sf::Uint32, float> cooldown;

	// �E�B���h�E������܂Ń��[�v
	while (inputPromptWindow.isOpen())
	{
		// 60FPS�ɍ��킹�Ēx����������
		while (mClock.getElapsedTime().asMilliseconds() - ticksCount.asMilliseconds() < 16);

		float deltaTime = static_cast<float>(mClock.getElapsedTime().asMilliseconds() - ticksCount.asMilliseconds()) / 1000.0f;
		if (deltaTime > 0.05f)
		{
			deltaTime = 0.05f;
		}
		ticksCount = mClock.getElapsedTime();
		// ���͂̃N�[���_�E��������
		for (auto& inputCooldown : cooldown)
		{
			inputCooldown.second -= deltaTime;
		}

		// �C�x���g�L���[�����݂���ꍇ�A����ɉ�����������S�čs��
		sf::Event event;
		while (inputPromptWindow.pollEvent(event))
		{
			// ���͓r���ŃE�B���h�E�����ꍇ
			if (event.type == sf::Event::Closed || sf::Keyboard::isKeyPressed(sf::Keyboard::Escape) || sf::Keyboard::isKeyPressed(sf::Keyboard::Enter))
			{
				// ���l�����͂���Ă��Ȃ��ꍇ
				if (values[0].length() == 0 || values[1].length() == 0 || values[2].length() == 0)
				{
					values[0] = "9";
					values[1] = "9";
					values[2] = "3";
				}
				else
				{
					// ���l���������Ȃ��Ƃ�
					if (std::stoi(values[0]) <= 5)
					{
						values[0] = "5";
					}

					if (std::stoi(values[1]) <= 5)
					{
						values[1] = "5";
					}

					if (std::stoi(values[2]) > ((std::stoi(values[0]) - 4) * (std::stoi(values[1]) - 4) * 3 + 1) / 4)
					{
						values[2] = std::to_string(((std::stoi(values[0]) - 4) * (std::stoi(values[1]) - 4) * 3 + 1) / 4);
					}
				}

				inputPromptWindow.close();
			}
			// �o�b�N�X�y�[�X�L�[
			else if (sf::Keyboard::isKeyPressed(sf::Keyboard::BackSpace) && currentPos >= 0)
			{
				if (cooldown[59] <= 0.0f)
				{
					values[currentElem].pop_back();
					currentPos--;
					cooldown[59] = 0.13f;
				}
			}
			// �\���L�[
			else if (sf::Keyboard::isKeyPressed(sf::Keyboard::Down) && currentElem < values.size() - 1)
			{
				if (cooldown[event.text.unicode] <= 0.0f)
				{
					currentElem++;
					currentPos = values[currentElem].length() - 1;
					cooldown[event.text.unicode] = 0.13f;
				}
			}
			else if (sf::Keyboard::isKeyPressed(sf::Keyboard::Up) && currentElem > 0)
			{
				if (cooldown[event.text.unicode] <= 0.0f)
				{
					currentElem--;
					currentPos = values[currentElem].length() - 1;
					cooldown[event.text.unicode] = 0.13f;
				}
			}
			/*else if (sf::Keyboard::isKeyPressed(sf::Keyboard::Right) && currentPos < values[currentElem].length() - 1)
			{
				if (cooldown[event.text.unicode] <= 0.0f)
				{
					currentPos++;
				}
				cooldown[event.text.unicode] = 0.13f;
			}
			else if (sf::Keyboard::isKeyPressed(sf::Keyboard::Left) && currentPos > 0)
			{
				if (cooldown[event.text.unicode] <= 0.0f)
				{
					currentPos--;
				}
				cooldown[event.text.unicode] = 0.13f;
			}*/
			// �����L�[
			else if (event.type == sf::Event::TextEntered && event.text.unicode >= 48 && event.text.unicode <= 57)
			{
				if (cooldown[event.text.unicode] <= 0.0f)
				{
					values[currentElem] += std::to_string(event.text.unicode - 48);
					currentPos++;
				}
				cooldown[event.text.unicode] = 0.13f;
			}
			else if (event.type == sf::Event::TextEntered && event.text.unicode == 46 && currentElem == 5 && values[5].find(".") == std::string::npos)
			{
				if (cooldown[event.text.unicode] <= 0.0f)
				{
					values[currentElem] += ".";
					currentPos++;
				}
				cooldown[event.text.unicode] = 0.13f;
			}
		}

		// �e�L�X�g���X�V
		if (currentElem == 0)
		{
			inputWidth.setString(" > Input width : " + values[0]);
			inputHeight.setString("   Input height : " + values[1]);
			inputBaggageNum.setString("   Input the number of baggage : " + values[2]);
			inputRepet1.setString("   Input repetition rate1 : " + values[3]);
			inputRepet2.setString("   Input repetition rate2 : " + values[4]);
			inputRepet3.setString("   Input repetition rate3 : " + values[5]);
			inputRepet4.setString("   Input repetition rate4 : " + values[6]);
			inputRepet5.setString("   Input repetition rate5 : " + values[7]);
		}
		else if (currentElem == 1)
		{
			inputWidth.setString("   Input width : " + values[0]);
			inputHeight.setString(" > Input height : " + values[1]);
			inputBaggageNum.setString("   Input the number of baggage : " + values[2]);
			inputRepet1.setString("   Input repetition rate1 : " + values[3]);
			inputRepet2.setString("   Input repetition rate2 : " + values[4]);
			inputRepet3.setString("   Input repetition rate3 : " + values[5]);
			inputRepet4.setString("   Input repetition rate4 : " + values[6]);
			inputRepet5.setString("   Input repetition rate5 : " + values[7]);
		}
		else if(currentElem == 2)
		{
			inputWidth.setString("   Input width : " + values[0]);
			inputHeight.setString("   Input height : " + values[1]);
			inputBaggageNum.setString(" > Input the number of baggage : " + values[2]);
			inputRepet1.setString("   Input repetition rate1 : " + values[3]);
			inputRepet2.setString("   Input repetition rate2 : " + values[4]);
			inputRepet3.setString("   Input repetition rate3 : " + values[5]);
			inputRepet4.setString("   Input repetition rate4 : " + values[6]);
			inputRepet5.setString("   Input repetition rate5 : " + values[7]);
		}
		else if (currentElem == 3)
		{
			inputWidth.setString("   Input width : " + values[0]);
			inputHeight.setString("   Input height : " + values[1]);
			inputBaggageNum.setString("   Input the number of baggage : " + values[2]);
			inputRepet1.setString(" > Input repetition rate1 : " + values[3]);
			inputRepet2.setString("   Input repetition rate2 : " + values[4]);
			inputRepet3.setString("   Input repetition rate3 : " + values[5]);
			inputRepet4.setString("   Input repetition rate4 : " + values[6]);
			inputRepet5.setString("   Input repetition rate5 : " + values[7]);
		}
		else if (currentElem == 4)
		{
			inputWidth.setString("   Input width : " + values[0]);
			inputHeight.setString("   Input height : " + values[1]);
			inputBaggageNum.setString("   Input the number of baggage : " + values[2]);
			inputRepet1.setString("   Input repetition rate1 : " + values[3]);
			inputRepet2.setString(" > Input repetition rate2 : " + values[4]);
			inputRepet3.setString("   Input repetition rate3 : " + values[5]);
			inputRepet4.setString("   Input repetition rate4 : " + values[6]);
			inputRepet5.setString("   Input repetition rate5 : " + values[7]);
		}
		else if (currentElem == 5)
		{
			inputWidth.setString("   Input width : " + values[0]);
			inputHeight.setString("   Input height : " + values[1]);
			inputBaggageNum.setString("   Input the number of baggage : " + values[2]);
			inputRepet1.setString("   Input repetition rate1 : " + values[3]);
			inputRepet2.setString("   Input repetition rate2 : " + values[4]);
			inputRepet3.setString(" > Input repetition rate3 : " + values[5]);
			inputRepet4.setString("   Input repetition rate4 : " + values[6]);
			inputRepet5.setString("   Input repetition rate5 : " + values[7]);
		}
		else if (currentElem == 6)
		{
			inputWidth.setString("   Input width : " + values[0]);
			inputHeight.setString("   Input height : " + values[1]);
			inputBaggageNum.setString("   Input the number of baggage : " + values[2]);
			inputRepet1.setString("   Input repetition rate1 : " + values[3]);
			inputRepet2.setString("   Input repetition rate2 : " + values[4]);
			inputRepet3.setString("   Input repetition rate3 : " + values[5]);
			inputRepet4.setString(" > Input repetition rate4 : " + values[6]);
			inputRepet5.setString("   Input repetition rate5 : " + values[7]);
		}
		else if (currentElem == 7)
		{
			inputWidth.setString("   Input width : " + values[0]);
			inputHeight.setString("   Input height : " + values[1]);
			inputBaggageNum.setString("   Input the number of baggage : " + values[2]);
			inputRepet1.setString("   Input repetition rate1 : " + values[3]);
			inputRepet2.setString("   Input repetition rate2 : " + values[4]);
			inputRepet3.setString("   Input repetition rate3 : " + values[5]);
			inputRepet4.setString("   Input repetition rate4 : " + values[6]);
			inputRepet5.setString(" > Input repetition rate5 : " + values[7]);
		}

		// �f�B�X�v���C�\��
		inputPromptWindow.clear();
		inputPromptWindow.draw(inputWidth);
		inputPromptWindow.draw(inputHeight);
		inputPromptWindow.draw(inputBaggageNum);
		inputPromptWindow.draw(inputRepet1);
		inputPromptWindow.draw(inputRepet2);
		inputPromptWindow.draw(inputRepet3);
		inputPromptWindow.draw(inputRepet4);
		inputPromptWindow.draw(inputRepet5);
		inputPromptWindow.display();
	}

	// ���l���X�V
	mBoardSize = sf::Vector2i{ std::stoi(values[0]), std::stoi(values[1]) };
	mBaggageNum = std::stoi(values[2]);
	mRepetition01 = std::stoi(values[3]);
	mRepetition02 = std::stoi(values[4]);
	mRepetition03 = std::stod(values[5]);
	mRepetition04 = std::stod(values[6]);
	mRepetition05 = std::stoi(values[7]);

	// �Q�[�����[�v���ĊJ
	mWindow->setActive(true);
}

void Game::DisplayHelpWindow()
{
	// �e�L�X�g�f�[�^�̓ǂݍ���
	std::string filename = "Assets/help.txt";
	mFilenames.push_back(filename);
	std::ifstream file(filename);
	if (!file.is_open())
	{
		std::cerr << "Failed to open file" << std::endl;
		return;
	}

	std::string helpText = "", line;
	while (std::getline(file, line))
	{
		helpText += line + "\n";
	}
	helpText.pop_back();

	file.close();

	// �E�B���h�E���쐬
	sf::RenderWindow inputPromptWindow(sf::VideoMode(640, 360), "Help Window");

	// �e���̓t�H�[��
	sf::Text text(helpText, mFont, 24);
	sf::FloatRect textRect = text.getLocalBounds();
	text.setOrigin(textRect.left + textRect.width / 2.0f, textRect.top + textRect.height / 2.0f);
	text.setPosition(sf::Vector2f{ textRect.width / 2.0f, textRect.height / 2.0f + 10.0f });
	
	// �Q�[���E�B���h�E����̈ʒu��ݒ�
	inputPromptWindow.setPosition(sf::Vector2i(200, 200));

	// �Q�[�����[�v���~
	mWindow->setActive(false);

	// �E�B���h�E������܂Ń��[�v
	while (inputPromptWindow.isOpen())
	{
		// �C�x���g�L���[�����݂���ꍇ�A����ɉ�����������S�čs��
		sf::Event event;
		while (inputPromptWindow.pollEvent(event))
		{
			// �E�B���h�E�����
			if (event.type == sf::Event::Closed || sf::Keyboard::isKeyPressed(sf::Keyboard::Escape) || sf::Keyboard::isKeyPressed(sf::Keyboard::Enter))
			{
				inputPromptWindow.close();
			}
		}

		// �f�B�X�v���C�\��
		inputPromptWindow.clear();
		inputPromptWindow.draw(text);
		inputPromptWindow.display();
	}

	// �Q�[�����[�v���ĊJ
	mWindow->setActive(true);
}