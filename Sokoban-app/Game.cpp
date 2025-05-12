#include "Game.h"

#include "IActor.h"
#include "IComponent.h"
#include "SpriteComponent.h"

#include "GameBoard.h"
#include "BackGround.h"
#include "Baggage.h"
#include "IUIScreen.h"
#include "PauseMenu.h"
#include "MySolution.h"
#include "THUD.h"
#include "HUDHelper.h"

#include <iostream>
#include <filesystem>
#include <fstream>
#include <chrono>
#include <ctime>

Game::Game()
	: mWindow(nullptr)
	, mGameState(GameState::EGamePlay)
	, mPlayer(nullptr)
	, mGameBoard(nullptr)
	, mBaggages(std::vector<Baggage*>{})
	, mHUDHelper(nullptr)
	, mUpdatingActors(false)
	, mIsComplete(false)
	, mWindowSize(1600.0, 900.0)
	, mBoardViewArea(BoundingBox{ sf::Vector2f{ mWindowSize.x - mWindowSize.y, 0.0 }, sf::Vector2f{ mWindowSize.x, mWindowSize.y } })
	, mInputCooldown(0.0f)
	, mBoardSize(sf::Vector2i{ 0, 0 })
	, mBaggageNum(5)
	, mBaggageLimit(1)
	, mRepetition01(1)
	, mRepetition02(10)
	, mRepetition03(0.0)
	, mRepetition04(0.0)
	, mRepetition05(0)
	, mStep(0)
{
	// �Ֆʂ����[�h����x�ɍX�V����͈̂ȉ��̃����o�ϐ�
	// mBoardSize
	// ���݂̔ՖʃT�C�Y
	// mBaggageNum
	// ���݂̔Ֆʂ̉ו��̐�
	// mRepetition01
	// mRepetition02
	// mRepetition03
	// mRepetition04
	// mRepetition05
	// �Ֆʂ̎����������s���ۂ̃p�����[�^
	// mCurrentKey
	// ���݂̔Ֆʂ̃L�[ (�Q�[���N�������烍�[�h���ꂽ�Ֆʂ�ǂݍ��ނ̂Ɏg��)
	// mFilenames
	// �Q�[���N�������烍�[�h���ꂽ�Ֆʂ̃L�[�̃��X�g
	// mBoardData
	// �Q�[���N�������烍�[�h���ꂽ�Ֆʂ̃��X�g (�i�s��Ԃ𔽉f)
	// mInitBoardData
	// �Q�[���N�������烍�[�h���ꂽ�Ֆʂ̏�����ԃ��X�g
	// mBoardState
	// ���݂̔Ֆʂ̃v���C���[�Ɖו����������n�`���
	// mGoalPos
	// ���݂̔Ֆʂ̃S�[���ʒu�̃��X�g
	// mBaggageLimit
	// ���݂̔Ֆʂ̍L���Ə����ǃ}�X�̐�����������v�Z�����A�ݒu�ł���ו��̐�
	// mInitialPlayerPos
	// ���݂̔Ֆʂ̃v���C���[�̏����ʒu
	// mInitialBaggagePos
	// ���݂̔Ֆʂ̉ו��̏����ʒu
	// mStep
	// ���݂̔Ֆʂ̎萔
	// mLogs
	// ���݂̔Ֆʂ̓��͂̃��O
}

bool Game::Initialize()
{
	// �E�B���h�E�̍쐬
	// �Ō�2�̈����͉�ʃT�C�Y�Œ�̂��߂̂��܂��Ȃ�
	sf::RenderWindow* window = new sf::RenderWindow(sf::VideoMode(static_cast<unsigned int>(mWindowSize.x), static_cast<unsigned int>(mWindowSize.y)), "SFML Window", sf::Style::Titlebar | sf::Style::Close);
	mWindow = window;
	
	// �t�@�C���̓ǂݍ���
	LoadData();

	mClock.restart();

	mTicksCount = mClock.getElapsedTime();

	mStart = std::chrono::system_clock::now();

	return true;
}

void Game::RunLoop()
{
	while (mGameState != GameState::EQuit)
	{
		ProcessInput();
		UpdateGame();
		GenerateOutput();
	}
}

void Game::LoadData()
{
	// gui�̃e�[�}�����[�h
	mGui = std::make_unique<tgui::Gui>(*mWindow);
	mTheme = std::make_unique<tgui::Theme>("Assets/themes/Black.txt");

	// �t�H���g�����[�h���A������
	if (!mFont.loadFromFile("Assets/fonts/arial.ttf"))
	{
		std::cout << "error : failed loading font." << std::endl;
	}
	mInfoTxt.setFont(mFont);
	mInfoTxt.setCharacterSize(mWindow->getSize().y);
	mInfoTxt.setScale(20.0f / static_cast<float>(mWindow->getSize().y), 20.0f / static_cast<float>(mWindow->getSize().y));

	// �Ֆʃf�[�^��ǂݎ��
	//*
	std::string filename = "Assets/Boards/default.txt", name = "default";
	mCurrentKey = name;
	mFilenames.push_back(name);
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

	mBoardData.emplace(name, lines);
	mInitBoardData.emplace(name, lines);
	file.close();
	//*/

	// ���������̏ꍇ
	// MySolution
	/*
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
	new BackGround(this);

	// �{�[�h���쐬
	mGameBoard = new GameBoard(this);
	mBoardSize = sf::Vector2i{ static_cast<int>(lines.front().size()), static_cast<int>(lines.size()) };

	// �v���C���[�̍쐬
	mPlayer = new Player(this);

	// �ו����쐬
	// �ו��̏������W�𓾂�
	for (const auto& item : mBoxesPos)
	{
		mBaggages.emplace_back(new Baggage(this, item));
		mInitialBaggagePos.emplace(mBaggages.back(), item);
	}

	// HUD�⏕�̃N���X��錾
	mHUDHelper = new HUDHelper(this);

	// TODO UI�֘A�̃N���X�̃C���X�^���X�쐬�̏���������
	// HUD�̃C���X�^���X�쐬
	// �Ֆʂ̏��𓾂Ă���쐬����
	// tgui��p����ꍇ�͈ȉ��ɏ���
	// �Q�[�����[�v�����̏����͖�����
	new THUD(this, mWindow);

	// �����HUD�N���X��p����ꍇ�͈ȉ��̃R�����g�A�E�g��߂�
	// new HUD(this);
}

void Game::UnloadData()
{
	// �A�N�^�[���폜
	while (!mActiveActors.empty())
	{
		delete mActiveActors.back();
	}

	// HUD�w���p�[���폜
	delete mHUDHelper;

	// UI�X�^�b�N�����ׂč폜
	while (!mUIStack.empty())
	{
		delete mUIStack.back();
		mUIStack.pop_back();
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
			mGameState = GameState::EQuit;
			break;
		}

		// �Q�[���S�̂Ɋւ���e����͏���
		if (mInputCooldown <= 0.0f)
		{
			// Esc�L�[�Ń|�[�Y���j���[���J��
			if (sf::Keyboard::isKeyPressed(sf::Keyboard::Escape) && mGameState == GameState::EGamePlay)
			{
				mInputCooldown = 0.13f;
				new PauseMenu(this);
				mGameState = GameState::EPaused;
			}

			// z ��undo����
			if (sf::Keyboard::isKeyPressed(sf::Keyboard::Z))
			{
				mInputCooldown = 0.13f;
				CallUndo();
			}

			// y ��redo����
			if (sf::Keyboard::isKeyPressed(sf::Keyboard::Y))
			{
				mInputCooldown = 0.13f;
				CallRedo();
			}

			// PGUP�ōŐV�̏�Ԃɂ���
			if (sf::Keyboard::isKeyPressed(sf::Keyboard::PageUp))
			{
				CallRedoAll();
			}

			// PGDN�ŏ�����Ԃɂ���
			if (sf::Keyboard::isKeyPressed(sf::Keyboard::PageDown))
			{
				CallReset();
			}

			// Ctrl + r�őS�ă��Z�b�g
			if (sf::Keyboard::isKeyPressed(sf::Keyboard::LControl) && sf::Keyboard::isKeyPressed(sf::Keyboard::R))
			{
				mInputCooldown = 0.13f;
				CallRestart();
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

		// �Q�[���v���C��ԂȂ�A�N�^�[�̓��͏������s��
		if (mGameState == GameState::EGamePlay)
		{
			// �S�Ă�Actor�̓��͏������s��
			mUpdatingActors = true;

			for (auto& actor : mActiveActors)
			{
				actor->ProcessInput(&event);
			}

			mUpdatingActors = false;
			// �A�N�^�[�̓��͏����͂����܂�
		}

		// ���̑��̏�Ԃł�UI�̓��͏������s��
		// �Q�[���v���C���Ɏg����UI�͖�����
		if (!mUIStack.empty())
		{
			// ��Ԏ�O�̃��C����UI�̓��͏����̂ݍs��
			mUIStack.back()->ProcessInput(&event, sf::Mouse::getPosition(*mWindow));
		}
	}
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

	for (auto& actor : mActiveActors)
	{
		actor->Update(deltaTime);
	}
	//mPlayer->Update(deltaTime);
	for (auto& item : mBaggages)
	{
		item->Update(deltaTime);
	}

	// �Ֆʂ̍X�V���s�����̂ŁAHUDHelper�̓��e���X�V
	mHUDHelper->Update();

	mUpdatingActors = false;
	// �A�N�^�[�̍X�V�͂����܂�

	// �X�^�b�N��UI��ʂ��X�V����
	for (auto& ui : mUIStack)
	{
		if (ui->GetState() == IUIScreen::EActive)
		{
			ui->Update(deltaTime);
		}
	}

	// �N���[�W���O��Ԃ�UI��ʂ����ׂĔj������
	// �X�^�b�N�Ɠ����̏��������邽�߁A���X�g�̖�������폜�����Ă���
	for (int i = static_cast<int>(mUIStack.size()) - 1; i >= 0; --i)
	{
		if (mUIStack[i]->GetState() == IUIScreen::EClosing)
		{
			delete mUIStack[i];
			mUIStack.erase(mUIStack.begin() + i);
		}
	}

	// �ҋ@���̃A�N�^�[�����s�\��Ԃɂ���
	for (auto& pendingActor : mPendingActors)
	{
		mActiveActors.emplace_back(pendingActor);
	}

	// ���񂾃A�N�^�[���폜����
	std::vector<IActor*> deadActors;
	for (auto actor : mActiveActors)
	{
		if (actor->GetState() == IActor::ActorState::EClosing)
		{
			deadActors.emplace_back(actor);
		}
	}

	for (auto actor : deadActors)
	{
		delete actor;
	}

	// �Q�[���̃N���A������Ăяo��
	HasComplete();
}

void Game::GenerateOutput()
{
	// �S�ẴX�v���C�g�����R���|�[�l���g��`��
	mWindow->clear();

	// �w�ʂ���`��
	DrawSprites();

	// UI�̓Q�[���I�u�W�F�N�g�̏�ɕ`�悷��̂ł����ɏ���������
	DrawUI();
	
	mWindow->display();

	// �Q�[���N���A��ԂȂ烊�U���g��ʂ��o��
	if (mIsComplete)
	{
		DisplayResult();
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

// �A�N�^�[�̒ǉ��ƍ폜
void Game::AddActor(IActor* actor)
{
	// �X�V���̃A�N�^�[������΁A�ҋ@���ɒǉ�����
	if (mUpdatingActors)
	{
		mPendingActors.emplace_back(actor);
	}
	else
	{
		mActiveActors.emplace_back(actor);
	}
}

void Game::RemoveActor(IActor* actor)
{
	// �����̃A�N�^�[�����݂��Ȃ���Ή������Ȃ�
	// �ҋ@�����H
	auto iter = std::find(mPendingActors.begin(), mPendingActors.end(), actor);
	if (iter != mPendingActors.end())
	{
		std::iter_swap(iter, mPendingActors.end() - 1);
		mPendingActors.pop_back();
	}

	// �A�N�^�[�ɂ��邩�H
	iter = std::find(mActiveActors.begin(), mActiveActors.end(), actor);
	if (iter != mActiveActors.end())
	{
		std::iter_swap(iter, mActiveActors.end() - 1);
		mActiveActors.pop_back();
	}
}

// �X�v���C�g�̒ǉ��ƍ폜
void Game::AddSprite(SpriteComponent* sprite)
{
	int myDrawOrder = sprite->GetDrawOrder();
	auto iter = mSprites.begin();
	for (;
		iter != mSprites.end();
		++iter)
	{
		if (myDrawOrder < (*iter)->GetDrawOrder())
		{
			break;
		}
	}

	mSprites.insert(iter, sprite);
}

void Game::RemoveSprite(SpriteComponent* sprite)
{
	auto iter = std::find(mSprites.begin(), mSprites.end(), sprite);
	if (iter != mSprites.end())
	{
		mSprites.erase(iter);
	}
}

// �X�v���C�g�̕`��
void Game::DrawSprites()
{
	for (auto& sprite : mSprites)
	{
		sprite->Draw(mWindow);
	}
}

// UI�̕`��
void Game::DrawUI()
{
	for (const auto& ui : mUIStack)
	{
		ui->Draw(mWindow);
	}
}

// �Ֆʃ��X�g�ɐV�����Ֆʂ�ǉ�
void Game::AddBoard(const std::string& key, const std::vector<std::string>& lines)
{
	if (!mInitBoardData.count(key))
	{
		mInitBoardData.emplace(key, lines);
		mBoardData.emplace(key, lines);
		mFilenames.emplace_back(key);
	}
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

void Game::CallReset()
{
	while (mStep > 0)
	{
		CallUndo();
	}
}

void Game::CallRedoAll()
{
	while (mLogs.size() > mStep)
	{
		CallRedo();
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
		for (const auto& str : mInitBoardData[mCurrentKey])
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
	// �v���C���[�Ɖו��ƔՖʂ��X�V
	// �p�����[�^�ݒ�ۂɁAclose��������Ȃ���΁A�����̍X�V�������s��
	if (InputBoardData())
	{
		// ���O���t�@�C���o�͂��Ă���S�ď���
		OutputLogs();
		ResetParameters();

		// �X�V���ꂽGame�N���X�̃����o�ϐ����Q�Ƃ��ĔՖʂ𐶐�
		MySolution* gen = new MySolution(mBoardSize, mBaggageNum, mRepetition01, mRepetition02, mRepetition03, mRepetition04, mRepetition05);
		std::vector<std::string> lines = gen->GetBoard();
		delete(gen);

		// �������ꂽ�Ֆʂ̃L�[�͎����𕶎���ɕϊ��������̂ɂ���
		mCurrentKey = GetDateTime();
		mFilenames.emplace_back(mCurrentKey);
		mBoardData.emplace(mCurrentKey, lines);
		mInitBoardData.emplace(mCurrentKey, lines);
		mBaggageLimit = GetBaggageNumLimit(mBoardSize, mRepetition03);

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

		// HUDHelper���č\�z
		mHUDHelper = new HUDHelper(this);

		mStart = std::chrono::system_clock::now();
	}
}

void Game::CallRestart()
{
	// ���O���t�@�C���o�͂��Ă���S�ď���
	OutputLogs();
	ResetParameters();

	mBoardData[mCurrentKey] = mInitBoardData[mCurrentKey];

	// �{�[�h���č\�z
	mGameBoard->Reload();

	// �v���C���[���č\�z
	mPlayer->Reload();

	// �ו����č\�z
	for (const auto& baggage : mInitialBaggagePos)
	{
		baggage.first->SetBoardCoordinate(baggage.second);
	}

	mStart = std::chrono::system_clock::now();
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
		for (const auto& row : mInitBoardData[mGameBoard->GetBoardName()])
		{
			outFile << row << std::endl;
		}
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
	// current�͌��݂̃X�e�b�v��
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

		// ���򂵂������́A����n�_�ŃJ�b�g���A�S�Ă̕�����ۊ��ʂň͂��ăJ���}��؂�ŕ��ׂ�
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

void Game::DisplayResult()
{
	bool isChildWindowOpened = true;
	bool isPlayLog = false;
	// �\���p�̃E�B���h�E���쐬
	auto child = tgui::ChildWindow::create();
	child->setRenderer(mTheme->getRenderer("ChildWindow"));
	child->setClientSize({ 960, 540 });
	child->setPosition(420, 80);
	child->setTitle("Notice");
	child->onClose([&isChildWindowOpened]() {
		isChildWindowOpened = false;
		});
	mGui->add(child);

	// ListBox�Ńe�L�X�g��\��
	auto listBox = tgui::ListBox::create();
	listBox->setRenderer(mTheme->getRenderer("ListBox"));
	listBox->setSize(960, 500);
	listBox->setItemHeight(32);
	listBox->setPosition(0, 0);
	listBox->setTextSize(20);
	listBox->addItem("Result");
	listBox->addItem("Steps : " + std::to_string(mStep));
	long time = static_cast<long>(GetSecTime());
	std::stringstream m{}, s{};
	m << std::setw(2) << std::setfill('0') << time / 60;
	s << std::setw(2) << std::setfill('0') << time % 60;
	listBox->addItem("Time : " + m.str() + ":" + s.str());
	listBox->addItem("");
	listBox->addItem("");
	child->add(listBox);

	// ���͏I���p�{�^��
	auto exitButton = tgui::Button::create("Close");
	exitButton->setRenderer(mTheme->getRenderer("Button"));
	exitButton->setSize(120, 30);
	exitButton->setPosition(static_cast<int>(child->getSize().x - exitButton->getSize().x) - 10, static_cast<int>(listBox->getSize().y) + 5);
	exitButton->onPress([&]() {
		std::cout << "Exit child window action triggered!" << std::endl;
		isChildWindowOpened = false;
		});
	child->add(exitButton);

	// �v���C�������Đ����郂�[�h�Ɉڂ�{�^��
	auto playLogButton = tgui::Button::create("Play Log");
	playLogButton->setRenderer(mTheme->getRenderer("Button"));
	playLogButton->setSize(120, 30);
	playLogButton->setPosition(static_cast<int>(child->getSize().x - exitButton->getSize().x * 2.0f) - 30, static_cast<int>(listBox->getSize().y) + 5);
	playLogButton->onPress([&]() {
		std::cout << "Play Log action triggered!" << std::endl;
		isChildWindowOpened = false;
		isPlayLog = true;
		});
	child->add(playLogButton);

	// �Q�[�����[�v���~
	mWindow->setActive(false);

	// �f���^�^�C���֘A
	sf::Time ticksCount = mClock.getElapsedTime();

	// ���͂ƍX�V�ƃE�B���h�E�̏I������
	// �E�B���h�E������܂Ń��[�v
	while (isChildWindowOpened)
	{
		// 60FPS�ɍ��킹�Ēx����������
		while (mClock.getElapsedTime().asMilliseconds() - ticksCount.asMilliseconds() < 16);

		float deltaTime = static_cast<float>(mClock.getElapsedTime().asMilliseconds() - ticksCount.asMilliseconds()) / 1000.0f;
		if (deltaTime > 0.05f)
		{
			deltaTime = 0.05f;
		}
		ticksCount = mClock.getElapsedTime();

		// �C�x���g�L���[�����݂���ꍇ�A����ɉ�����������S�čs��
		sf::Event event;
		while (mWindow->pollEvent(event))
		{
			if (event.type == sf::Event::Closed ||
				event.key.code == sf::Keyboard::Escape ||
				event.key.code == sf::Keyboard::Enter)
			{
				isChildWindowOpened = false;
			}

			mGui->handleEvent(event);
		}

		mWindow->clear();

		// �w�ʂ���`��
		for (auto& sprite : mSprites)
		{
			sprite->Draw(mWindow);
		}

		// UI�̓Q�[���I�u�W�F�N�g�̏�ɕ`�悷��̂ł����ɏ���������
		for (const auto& ui : mUIStack)
		{
			ui->Draw(mWindow);
		}

		mGui->draw(); // TGUI�̕`��
		mWindow->display();
	}

	child->close();

	if (isPlayLog)
	{
		DisplayPlayLogs(mCurrentKey);
	}

	// 1��߂�
	CallUndo();

	// �Q�[�����[�v���ĊJ
	mWindow->setActive(true);
}

bool Game::InputBoardData()
{
	bool result = true;
	bool isChildWindowOpened = true;
	// ���͗p�̃E�B���h�E���쐬
	auto child = tgui::ChildWindow::create();
	child->setRenderer(mTheme->getRenderer("ChildWindow"));	
	child->setClientSize({ 960, 540 });
	child->setPosition(420, 80);
	child->setTitle("Input Prompt");
	child->onClose([&isChildWindowOpened]() {
		isChildWindowOpened = false;
		});
	mGui->add(child);

	// �e�L�X�g
	struct BoardInfo
	{
		std::string mName;
		double mMin;
		double mMax;
		double mInitialValue;
		sf::Vector2i mPos;
		bool mIsInteger;
	};
	std::vector<BoardInfo> textInfoes(8, BoardInfo{ "", 0.0, 0.0, 0.0, sf::Vector2i{ 0, 0 } });
	textInfoes[0] = BoardInfo{ "width : ", static_cast<double>(mSizeMin.x), static_cast<double>(mSizeMax.x), static_cast<double>(mBoardSize.x), sf::Vector2i{ 500, 12 }, true };
	textInfoes[1] = BoardInfo{ "height : ", static_cast<double>(mSizeMin.y), static_cast<double>(mSizeMax.y), static_cast<double>(mBoardSize.y), sf::Vector2i{ 500, 48 }, true };
	textInfoes[2] = BoardInfo{ "baggage : ", 1.0, static_cast<double>(GetBaggageNumLimit(mBoardSize)), static_cast<double>(mBaggageNum), sf::Vector2i{500, 84}, true};
	textInfoes[3] = BoardInfo{ "number of times reset : ", 0.0, 32.0, static_cast<double>(mRepetition01), sf::Vector2i{ 500, 120 }, true };
	textInfoes[4] = BoardInfo{ "number of times transportation : ", 1.0, 256.0, static_cast<double>(mRepetition02), sf::Vector2i{ 500, 156 }, true };
	textInfoes[5] = BoardInfo{ "wall tile rate : ", 0.0, mWallRateLimit, mRepetition03, sf::Vector2i{ 500, 192 }, false };
	textInfoes[6] = BoardInfo{ "visited tile rate : ", 0.0, mVisitedRateLimit, mRepetition04, sf::Vector2i{ 500, 228 }, false };
	textInfoes[7] = BoardInfo{ "evaluation function : ", static_cast<double>(mEvaluateFancIndexRange.first), static_cast<double>(mEvaluateFancIndexRange.second), static_cast<double>(mRepetition05), sf::Vector2i{ 500, 264 }, true };
	auto listBox = tgui::ListBox::create();
	listBox->setRenderer(mTheme->getRenderer("ListBox"));
	listBox->setSize(960, 500);
	listBox->setItemHeight(36);
	listBox->setPosition(0, 0);
	for (int i = 0, size = static_cast<int>(textInfoes.size()); i < size; ++i)
	{
		listBox->addItem(textInfoes[i].mName, std::to_string(i));
	}
	child->add(listBox);

	// ���͗p�̃X���C�_�[�ƃe�L�X�g�{�b�N�X�̒ǉ�
	std::map<int, std::pair<tgui::Slider::Ptr, tgui::EditBox::Ptr>> inputers{};
	int index = 0;
	for (const auto& textInfo : textInfoes)
	{
		// �X���C�_�[�̒ǉ�
		auto slider = tgui::Slider::create();
		slider->setRenderer(mTheme->getRenderer("Slider"));
		slider->setPosition(textInfo.mPos.x, textInfo.mPos.y);
		slider->setSize(400, 18);
		slider->setValue(static_cast<float>(textInfo.mInitialValue));
		slider->setMinimum(static_cast<float>(textInfo.mMin));
		slider->setMaximum(static_cast<float>(textInfo.mMax));
		// ���͂𐮐��Ǝ����ŕ�����
		if (textInfo.mIsInteger)
		{
			slider->setStep(1.0f);
		}
		else
		{
			slider->setStep(0.001f);
		}
		child->add(slider);

		// �e�L�X�g�{�b�N�X�̒ǉ�
		auto editBox = tgui::EditBox::create();
		editBox->setRenderer(mTheme->getRenderer("EditBox"));
		editBox->setSize(80, 25);
		editBox->setTextSize(18);
		editBox->setPosition(textInfo.mPos.x - 100, textInfo.mPos.y);
		child->add(editBox);
		if (textInfo.mIsInteger)
		{
			editBox->setText(std::to_string(static_cast<int>(textInfo.mInitialValue)));

		}
		else
		{
			editBox->setText(std::to_string(std::floor(textInfo.mInitialValue * 1000.0) / 1000.0));
		}

		// �X���C�_�[�ƃe�L�X�g�{�b�N�X�𓯊�
		inputers.emplace(index, std::pair<tgui::Slider::Ptr, tgui::EditBox::Ptr>{ slider, editBox });
		SyncSliderWithEditBox(inputers[index].first, inputers[index].second, textInfo.mIsInteger);

		++index;
	}

	// ���͏I���p�{�^��
	auto exitButton = tgui::Button::create("Enter");
	exitButton->setRenderer(mTheme->getRenderer("Button"));
	exitButton->setSize(120, 30);
	exitButton->setPosition(static_cast<int>(child->getSize().x - exitButton->getSize().x) - 10, static_cast<int>(listBox->getSize().y) + 5);
	exitButton->onPress([&]() {
		std::cout << "Exit child window action triggered!" << std::endl;
		isChildWindowOpened = false;
		});
	child->add(exitButton);

	// ���̓L�����Z���p�{�^��
	auto canselButton = tgui::Button::create("Cansel");
	canselButton->setRenderer(mTheme->getRenderer("Button"));
	canselButton->setSize(120, 30);
	canselButton->setPosition(static_cast<int>(child->getSize().x - exitButton->getSize().x * 2.0) - 15, static_cast<int>(listBox->getSize().y) + 5);
	canselButton->onPress([&]() {
		std::cout << "Cansel input action triggered!" << std::endl;
		isChildWindowOpened = false;
		result = false;
		});
	child->add(canselButton);

	// �f���^�^�C���֘A
	sf::Time ticksCount = mClock.getElapsedTime();

	// ���͂ƍX�V�ƃE�B���h�E�̏I������
	// �E�B���h�E������܂Ń��[�v
	while (isChildWindowOpened)
	{
		// 60FPS�ɍ��킹�Ēx����������
		while (mClock.getElapsedTime().asMilliseconds() - ticksCount.asMilliseconds() < 16);

		float deltaTime = static_cast<float>(mClock.getElapsedTime().asMilliseconds() - ticksCount.asMilliseconds()) / 1000.0f;
		if (deltaTime > 0.05f)
		{
			deltaTime = 0.05f;
		}
		ticksCount = mClock.getElapsedTime();

		// �ו��̐��͔ՖʃT�C�Y��wall rate�ɂ���ď�����ς��̂ŁA�X�V����
		int tmpBaggageLimit = GetBaggageNumLimit(sf::Vector2i{ static_cast<int>(inputers[0].first->getValue()), static_cast<int>(inputers[1].first->getValue()) }, inputers[5].first->getValue());
		if (static_cast<int>(inputers[2].first->getValue()) > tmpBaggageLimit)
		{
			inputers[2].first->setValue(static_cast<float>(tmpBaggageLimit));
		}
		textInfoes[2].mMax = static_cast<double>(tmpBaggageLimit);
		inputers[2].first->setMaximum(static_cast<float>(tmpBaggageLimit));

		// ListBox�̓��e���X�V
		textInfoes[0].mName = "width : " + std::to_string(static_cast<int>(inputers[0].first->getValue()));
		textInfoes[1].mName = "height : " + std::to_string(static_cast<int>(inputers[1].first->getValue()));
		textInfoes[2].mName = "baggage : " + std::to_string(static_cast<int>(inputers[2].first->getValue()));
		textInfoes[3].mName = "number of times reset : " + std::to_string(static_cast<int>(inputers[3].first->getValue()));
		textInfoes[4].mName = "number of times transportation : " + std::to_string(static_cast<int>(inputers[4].first->getValue()));
		textInfoes[5].mName = "wall tile rate : " + std::to_string(inputers[5].first->getValue());
		textInfoes[6].mName = "visited tile rate : " + std::to_string(inputers[6].first->getValue());
		textInfoes[7].mName = "evaluation function : " + std::to_string(static_cast<int>(inputers[7].first->getValue()));

		// �C�x���g�L���[�����݂���ꍇ�A����ɉ�����������S�čs��
		sf::Event event;
		while (mWindow->pollEvent(event))
		{
			if (event.type == sf::Event::Closed)
			{
				isChildWindowOpened = false;
			}

			mGui->handleEvent(event);
		}

		mWindow->clear();

		// �w�ʂ���`��
		for (auto& sprite : mSprites)
		{
			sprite->Draw(mWindow);
		}

		// UI�̓Q�[���I�u�W�F�N�g�̏�ɕ`�悷��̂ł����ɏ���������
		for (const auto& ui : mUIStack)
		{
			ui->Draw(mWindow);
		}

		mGui->draw(); // TGUI�̕`��
		mWindow->display();
	}

	// ���͂��L�����Z�����Ȃ��̂ł���΁A���l���X�V
	if (result)
	{
		mBoardSize = sf::Vector2i{ static_cast<int>(inputers[0].first->getValue()), static_cast<int>(inputers[1].first->getValue()) };
		mBaggageNum = static_cast<int>(inputers[2].first->getValue());
		mRepetition01 = static_cast<int>(inputers[3].first->getValue());
		mRepetition02 = static_cast<int>(inputers[4].first->getValue());
		mRepetition03 = inputers[5].first->getValue();
		mRepetition04 = inputers[6].first->getValue();
		mRepetition05 = static_cast<int>(inputers[7].first->getValue());
	}

	child->close();

	// �Q�[�����[�v���ĊJ
	mWindow->setActive(true);

	return result;
}

void Game::SyncSliderWithEditBox(tgui::Slider::Ptr slider, tgui::EditBox::Ptr editBox, const bool& isInteger)
{
	if (isInteger)
	{
		slider->onValueChange([editBox](float newValue)
			{
				editBox->setText(std::to_string(static_cast<int>(newValue)));
			});

		editBox->onReturnKeyPress([slider, editBox](const tgui::String& text)
			{
				try
				{
					int value = std::stoi(editBox->getText().toStdString()); // �e�L�X�g�𐮐��ɕϊ�
					if (value >= static_cast<int>(slider->getMinimum()) && value <= static_cast<int>(slider->getMaximum()))
					{
						slider->setValue(static_cast<float>(value)); // �l��Slider�ɔ��f
					}
					else
					{
						// �͈͊O�̏ꍇ�A����Slider�̒l��\��
						editBox->setText(std::to_string(static_cast<int>(slider->getValue())));
					}
				}
				catch (const std::exception&)
				{
					// ���l�ȊO�̓��͂��������ꍇ�A����Slider�̒l��\��
					editBox->setText(std::to_string(static_cast<int>(slider->getValue())));
				}
			});
	}
	else
	{
		slider->onValueChange([editBox](float newValue)
			{
				editBox->setText(std::to_string(newValue));
			});

		editBox->onReturnKeyPress([slider, editBox](const tgui::String& text)
			{
				try
				{
					float value = std::stof(editBox->getText().toStdString()); // �e�L�X�g�𐮐��ɕϊ�
					if (value >= slider->getMinimum() && value <= slider->getMaximum())
					{
						slider->setValue(value); // �l��Slider�ɔ��f
					}
					else 
					{
						// �͈͊O�̏ꍇ�A����Slider�̒l��\��
						editBox->setText(std::to_string(slider->getValue()));
					}
				}
				catch (const std::exception&)
				{
					// ���l�ȊO�̓��͂��������ꍇ�A����Slider�̒l��\��
					editBox->setText(std::to_string(slider->getValue()));
				}
			});
	}
}

void Game::DisplayHelpWindow()
{
	// �e�L�X�g�f�[�^�̓ǂݍ���
	std::string filename = "Assets/help.txt";
	std::ifstream file(filename);
	if (!file.is_open())
	{
		std::cerr << "Failed to open file" << std::endl;
		return;
	}
	std::vector<std::string> helpTexts;
	std::string line = "";
	while (std::getline(file, line))
	{
		helpTexts.emplace_back(line);
	}
	
	file.close();

	bool isChildWindowOpened = true;
	// �\���p�̃E�B���h�E���쐬
	auto child = tgui::ChildWindow::create();
	child->setRenderer(mTheme->getRenderer("ChildWindow"));
	child->setClientSize({ 960, 540 });
	child->setPosition(420, 80);
	child->setTitle("How to play");
	child->onClose([&isChildWindowOpened]() {
		isChildWindowOpened = false;
		});
	mGui->add(child);

	// ListBox�Ńe�L�X�g��\��
	auto listBox = tgui::ListBox::create();
	listBox->setRenderer(mTheme->getRenderer("ListBox"));
	listBox->setSize(960, 500);
	listBox->setItemHeight(32);
	listBox->setPosition(0, 0);
	listBox->setTextSize(20);
	for (const auto& row : helpTexts)
	{
		listBox->addItem(row);
	}
	child->add(listBox);

	// ���͏I���p�{�^��
	auto exitButton = tgui::Button::create("Close");
	exitButton->setRenderer(mTheme->getRenderer("Button"));
	exitButton->setSize(120, 30);
	exitButton->setPosition(static_cast<int>(child->getSize().x - exitButton->getSize().x) - 10, static_cast<int>(listBox->getSize().y) + 5);
	exitButton->onPress([&]() {
		std::cout << "Exit child window action triggered!" << std::endl;
		isChildWindowOpened = false;
		});
	child->add(exitButton);

	// �Q�[�����[�v���~
	mWindow->setActive(false);

	// �f���^�^�C���֘A
	sf::Time ticksCount = mClock.getElapsedTime();

	// ���͂ƍX�V�ƃE�B���h�E�̏I������
	// �E�B���h�E������܂Ń��[�v
	while (isChildWindowOpened)
	{
		// 60FPS�ɍ��킹�Ēx����������
		while (mClock.getElapsedTime().asMilliseconds() - ticksCount.asMilliseconds() < 16);

		float deltaTime = static_cast<float>(mClock.getElapsedTime().asMilliseconds() - ticksCount.asMilliseconds()) / 1000.0f;
		if (deltaTime > 0.05f)
		{
			deltaTime = 0.05f;
		}
		ticksCount = mClock.getElapsedTime();

		// �C�x���g�L���[�����݂���ꍇ�A����ɉ�����������S�čs��
		sf::Event event;
		while (mWindow->pollEvent(event))
		{
			if (event.type == sf::Event::Closed ||
				event.key.code == sf::Keyboard::Escape ||
				event.key.code == sf::Keyboard::Enter)
			{
				isChildWindowOpened = false;
			}

			mGui->handleEvent(event);
		}

		mWindow->clear();

		// �w�ʂ���`��
		for (auto& sprite : mSprites)
		{
			sprite->Draw(mWindow);
		}

		// UI�̓Q�[���I�u�W�F�N�g�̏�ɕ`�悷��̂ł����ɏ���������
		for (const auto& ui : mUIStack)
		{
			ui->Draw(mWindow);
		}

		mGui->draw(); // TGUI�̕`��
		mWindow->display();
	}

	child->close();

	// �Q�[�����[�v���ĊJ
	mWindow->setActive(true);
}

void Game::SelectBoards()
{
	// �Ֆʃ��X�g�̓ǂݍ��݂��s��
	struct Contents
	{
		std::vector<std::string> mBoard;
		sf::Texture mTexture;
	};
	std::map<std::string, Contents> fileContents{};

	// �Q�Ƃ���Ֆʃ��X�g�̃f�B���N�g�����w��
	std::filesystem::directory_iterator iter("Assets/Boards"), end;
	sf::Vector2i tileSize{ mTextures["Assets/Floor.png"]->getSize() };
	std::error_code err;

	try
	{
		// �f�B���N�g�����̂��ׂẴG���g�����`�F�b�N
		for (; iter != end && !err; iter.increment(err))
		{
			// �G���g�����t�@�C���ł���A�g���q��.txt�̏ꍇ
			const std::filesystem::directory_entry entry = *iter;
			if (entry.is_regular_file() && entry.path().extension() == ".txt")
			{
				std::ifstream file(entry.path());
				if (file)
				{
					std::vector<std::string> lines;
					std::string line;
					sf::Vector2u totalSize{ 0, 0 };

					// �t�@�C����1�s���ǂݍ���
					while (std::getline(file, line))
					{
						lines.push_back(line);
						if (totalSize.x < static_cast<unsigned int>(line.length()))
						{
							totalSize.x = static_cast<unsigned int>(line.length());
						}
					}
					totalSize.y = static_cast<unsigned int>(lines.size());

					// �ǂ݂��񂾓��e����A�Ֆʂ̃e�N�X�`�����쐬
					sf::Texture tmpBoardTex{};

					// RenderTexture���쐬
					sf::RenderTexture renderTexture{};
					if (!renderTexture.create(totalSize.x * tileSize.x, totalSize.y * tileSize.y))
					{
						std::cerr << "Failed to create render texture: " << entry.path() << "\n";
					}
					renderTexture.clear(sf::Color::Transparent); // �w�i�𓧖���

					// �X�v���C�g��render texture�ɓ\��t���Ă����A�^�C���摜��z�u
					for (int y = 0; y < static_cast<int>(totalSize.y); ++y)
					{
						for (int x = 0; x < line.length(); ++x)
						{
							std::vector<sf::Texture*> currentTex{};
							switch (lines[y][x])
							{
							case ' ':
								currentTex.emplace_back(mTextures["Assets/Floor.png"]);
								break;
							case '#':
								currentTex.emplace_back(mTextures["Assets/Wall.png"]);
								break;
							case '.':
								currentTex.emplace_back(mTextures["Assets/Goal.png"]);
								break;
							case '$':
								currentTex.emplace_back(mTextures["Assets/Floor.png"]);
								currentTex.emplace_back(mTextures["Assets/Box.png"]);
								break;
							case '*':
								currentTex.emplace_back(mTextures["Assets/Goal.png"]);
								currentTex.emplace_back(mTextures["Assets/ShinyBox.png"]);
								break;
							case '@':
								currentTex.emplace_back(mTextures["Assets/Floor.png"]);
								currentTex.emplace_back(mTextures["Assets/playerN.png"]);
								break;
							case '+':
								currentTex.emplace_back(mTextures["Assets/Goal.png"]);
								currentTex.emplace_back(mTextures["Assets/playerN.png"]);
								break;
							default:
								break;
							}

							for (auto& tmpTex : currentTex)
							{
								sf::Sprite tmpSprite(*tmpTex);
								tmpSprite.setPosition(static_cast<float>(x * tileSize.x), static_cast<float>(y * tileSize.y));
								renderTexture.draw(tmpSprite);
							}
						}
					}
					// �\��t�����X�v���C�g��render texture�ɓK�p
					renderTexture.display();

					// ���������e�N�X�`�����擾
					const sf::Texture& combinedTexture = renderTexture.getTexture();

					// �t�@�C�����i�g���q�Ȃ��j���L�[�Ƃ��Ċi�[
					fileContents[entry.path().stem().string()] = Contents{ lines, combinedTexture };

					// �����Q�[���N���X�ɑ��݂��Ȃ��Ֆʂ̏ꍇ�A�ǉ��������s��
					if (!mInitBoardData.count(entry.path().stem().string()))
					{
						mInitBoardData.emplace(entry.path().stem().string(), lines);
						mBoardData.emplace(entry.path().stem().string(), lines);
						mFilenames.emplace_back(entry.path().stem().string());
					}
				}
				else 
				{
					std::cerr << "Failed to open file: " << entry.path() << "\n";
				}
			}
		}
	}
	catch (const std::exception& e) 
	{
		std::cerr << "Error while accessing directory: " << e.what() << "\n";
	}

	std::string selectedBoardName{};
	bool isChildWindowOpened = true;
	// �\���p�̃E�B���h�E���쐬
	auto child = tgui::ChildWindow::create();
	child->setRenderer(mTheme->getRenderer("ChildWindow"));
	child->setClientSize({ 960, 540 });
	child->setPosition(420, 80);
	child->setTitle("Board manager");
	child->onClose([&isChildWindowOpened]() {
		isChildWindowOpened = false;
		});
	mGui->add(child);

	// ListBox�Ńe�L�X�g��\��
	auto listBox = tgui::ListBox::create();
	listBox->setRenderer(mTheme->getRenderer("ListBox"));
	listBox->setSize(240, 500);
	listBox->setItemHeight(24);
	listBox->setPosition(0, 0);
	listBox->setTextSize(20);
	listBox->setAutoScroll(false);
	for (const auto& item : fileContents)
	{
		listBox->addItem(item.first, item.first);
	}
	child->add(listBox);

	// Picture�E�B�W�F�b�g���쐬���ĉ摜�\���p�ɒǉ�
	BoundingBox childPictureBB{ sf::Vector2i{ static_cast<int>(listBox->getPosition().x + listBox->getSize().x), static_cast<int>(listBox->getPosition().y) },
		sf::Vector2i{ static_cast<int>(child->getSize().x), static_cast<int>(listBox->getPosition().y + listBox->getSize().y)}};
	auto picture = tgui::Picture::create();
	picture->setSize(childPictureBB.second.x - childPictureBB.first.x, childPictureBB.second.y - childPictureBB.first.y);
	picture->setPosition(childPictureBB.first.x, childPictureBB.first.y);
	child->add(picture);

	// ListBox�̗v�f���I�����ꂽ��A�Ή�����Ֆʂ��E�B���h�E�E���ɕ\��
	listBox->onItemSelect([&](const tgui::String& selectedItem) {
		std::cout << "Select board on listbox action triggered!" << std::endl;
		selectedBoardName = "";
		if (fileContents.count(selectedItem.toStdString()))
		{
			selectedBoardName = selectedItem.toStdString();
			// picture�̃T�C�Y�ƈʒu�����Z�b�g
			picture->setSize(childPictureBB.second.x - childPictureBB.first.x, childPictureBB.second.y - childPictureBB.first.y);
			picture->setPosition(childPictureBB.first.x, childPictureBB.first.y);
			// sf::Texture����tgui::texture�ւ̕ϊ�
			sf::Image tmpImage = fileContents[selectedItem.toStdString()].mTexture.copyToImage();
			tgui::Texture tmpTGuiTex{};
			tmpTGuiTex.loadFromPixelData(tmpImage.getSize(), tmpImage.getPixelsPtr());
			picture->getRenderer()->setTexture(tmpTGuiTex);
			// �摜�̃X�P�[�����O�ƃI�t�Z�b�g�̐ݒ�
			float scaleFactor = std::min(picture->getSize().x / static_cast<float>(tmpImage.getSize().x), picture->getSize().y / static_cast<float>(tmpImage.getSize().y));
			picture->setSize(tgui::Layout2d{ static_cast<float>(tmpImage.getSize().x) * scaleFactor, static_cast<float>(tmpImage.getSize().y) * scaleFactor });
			picture->setPosition({ childPictureBB.first.x + (child->getSize().x - childPictureBB.first.x - static_cast<float>(tmpImage.getSize().x) * scaleFactor) / 2.0f, childPictureBB.first.y + (childPictureBB.second.y - childPictureBB.first.y - static_cast<float>(tmpImage.getSize().y) * scaleFactor) / 2.0f });
		}
		});

	// ���͏I���p�{�^��
	auto applyButton = tgui::Button::create("Apply");
	applyButton->setRenderer(mTheme->getRenderer("Button"));
	applyButton->setSize(120, 30);
	applyButton->setPosition(static_cast<int>(child->getSize().x - applyButton->getSize().x * 2.0f) - 30, static_cast<int>(listBox->getSize().y) + 5);
	applyButton->onPress([&]() {
		std::cout << "Apply board action triggered!" << std::endl;
		if (std::find(mFilenames.begin(), mFilenames.end(), selectedBoardName) != mFilenames.end())
		{
			mCurrentKey = selectedBoardName;
			ChangeBoard();
			isChildWindowOpened = false;
		}
		});
	child->add(applyButton);

	auto exitButton = tgui::Button::create("Cancel");
	exitButton->setRenderer(mTheme->getRenderer("Button"));
	exitButton->setSize(120, 30);
	exitButton->setPosition(static_cast<int>(child->getSize().x - exitButton->getSize().x) - 10, static_cast<int>(listBox->getSize().y) + 5);
	exitButton->onPress([&]() {
		std::cout << "Cansel selection action triggered!" << std::endl;
		isChildWindowOpened = false;
		});
	child->add(exitButton);

	// �Q�[�����[�v���~
	mWindow->setActive(false);

	// �f���^�^�C���֘A
	sf::Time ticksCount = mClock.getElapsedTime();

	// ���͂ƍX�V�ƃE�B���h�E�̏I������
	// �E�B���h�E������܂Ń��[�v
	while (isChildWindowOpened)
	{
		// 60FPS�ɍ��킹�Ēx����������
		while (mClock.getElapsedTime().asMilliseconds() - ticksCount.asMilliseconds() < 16);

		float deltaTime = static_cast<float>(mClock.getElapsedTime().asMilliseconds() - ticksCount.asMilliseconds()) / 1000.0f;
		if (deltaTime > 0.05f)
		{
			deltaTime = 0.05f;
		}
		ticksCount = mClock.getElapsedTime();

		// �C�x���g�L���[�����݂���ꍇ�A����ɉ�����������S�čs��
		sf::Event event;
		while (mWindow->pollEvent(event))
		{
			if (event.type == sf::Event::Closed ||
				event.key.code == sf::Keyboard::Escape ||
				event.key.code == sf::Keyboard::Enter)
			{
				isChildWindowOpened = false;
			}

			mGui->handleEvent(event);
		}

		mWindow->clear();

		// �w�ʂ���`��
		for (auto& sprite : mSprites)
		{
			sprite->Draw(mWindow);
		}

		// UI�̓Q�[���I�u�W�F�N�g�̏�ɕ`�悷��̂ł����ɏ���������
		for (const auto& ui : mUIStack)
		{
			ui->Draw(mWindow);
		}

		mGui->draw(); // TGUI�̕`��
		mWindow->display();
	}

	// �E�B���h�E�̃��������
	child->close();

	// �Q�[�����[�v���ĊJ
	mWindow->setActive(true);
}

void Game::ChangeBoard()
{
	// ���O���t�@�C���o�͂��Ă���S�ď���
	// TODO �Ֆʂ��ς������̂��̂ɂȂ��Ă��܂��Ă���
	OutputLogs();
	mLogs.clear();

	// �X�V���ꂽGame�N���X�̃����o�ϐ����Q�Ƃ��ĔՖʂ𐶐�
	std::vector<std::string> lines{ mInitBoardData[mCurrentKey] };

	mBoardSize = sf::Vector2i{ 0, 0 };
	mBaggageNum = 0;
	mInitialPlayerPos = sf::Vector2i{ -1, -1 };
	mInitialBaggagePos.clear();
	mGoalPos.clear();
	mBoardState.clear();
	mStep = 0;
	delete  mHUDHelper;

	// �������ꂽ�Ֆʂ̃L�[�͎����𕶎���ɕϊ��������̂ɂ���
	mBoardSize = sf::Vector2i{ static_cast<int>(lines.front().length()), static_cast<int>(lines.size()) };
	mBaggageLimit = GetBaggageNumLimit(mBoardSize);

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
					++mBaggageNum;
					mBoxesPos.push_back(sf::Vector2i(j, i));
					break;
				case '.':
					tmpstr += '.';
					mGoalPos.push_back(sf::Vector2i(j, i));
					break;
				case '*':
					tmpstr += '.';
					++mBaggageNum;
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
		mBaggages.pop_back();
	}
	mBaggages.clear();

	for (const auto& item : mBoxesPos)
	{
		mBaggages.emplace_back(new Baggage(this, item));
		mInitialBaggagePos.emplace(mBaggages.back(), item);
	}

	// HUDHelper���č\�z
	mHUDHelper = new HUDHelper(this);

	mStart = std::chrono::system_clock::now();
}

void Game::DisplayPlayLogs(const std::string& boardKey)
{
	// �����f�[�^��ǂ݂���
	// ��łȂ���΃��O�f�[�^�����̂܂܎����Ă���
	std::map<std::string, std::pair<Board, std::vector<Log>>> loadedPlayLogs{};
	if (!boardKey.empty())
	{
		loadedPlayLogs.emplace(mCurrentKey, std::make_pair(mInitBoardData[mCurrentKey], mLogs));
	}
	else
	{
		// TODO �t�H���_����ǂ݂���
	}

	if (!loadedPlayLogs.empty())
	{
		// �\���p�̃E�B���h�E���쐬
		bool isChildWindowOpened = true;
		auto child = tgui::ChildWindow::create();
		child->setRenderer(mTheme->getRenderer("ChildWindow"));
		child->setClientSize({ 960, 540 });
		child->setPosition(420, 80);
		child->setTitle("Play Log Viewer");
		child->onClose([&isChildWindowOpened]() {
			isChildWindowOpened = false;
			});
		mGui->add(child);

		// ListBox�Ń��O�̃��X�g��\��
		auto logListBox = tgui::ListBox::create();
		logListBox->setRenderer(mTheme->getRenderer("ListBox"));
		logListBox->setSize(240, 260);
		logListBox->setItemHeight(20);
		logListBox->setPosition(0, 0);
		logListBox->setTextSize(16);
		logListBox->setAutoScroll(false);
		for (const auto& item : loadedPlayLogs)
		{
			logListBox->addItem(item.first, item.first);
		}
		child->add(logListBox);

		// ListBox�ōĐ����̃��O�̏���\��
		auto infoListBox = tgui::ListBox::create();
		infoListBox->setRenderer(mTheme->getRenderer("ListBox"));
		infoListBox->setSize(240, 240);
		infoListBox->setItemHeight(20);
		infoListBox->setPosition(0, logListBox->getSize().y);
		infoListBox->setTextSize(16);
		infoListBox->setAutoScroll(false);
		child->add(infoListBox);

		// �v���C���[�I���p�{�^��
		auto exitButton = tgui::Button::create("exit");
		exitButton->setRenderer(mTheme->getRenderer("Button"));
		exitButton->setSize(120, 30);
		exitButton->setPosition(static_cast<int>(child->getSize().x - exitButton->getSize().x) - 10, static_cast<int>(logListBox->getSize().y + infoListBox->getSize().y) + 5);
		exitButton->onPress([&]() {
			std::cout << "Exit selection action triggered!" << std::endl;
			isChildWindowOpened = false;
			});
		child->add(exitButton);

		// TODO ���̑���

		// �Q�[�����[�v���~
		mWindow->setActive(false);

		// �f���^�^�C���֘A
		sf::Time ticksCount = mClock.getElapsedTime();

		// ���͂ƍX�V�ƃE�B���h�E�̏I������
		// �E�B���h�E������܂Ń��[�v
		while (isChildWindowOpened)
		{
			// 60FPS�ɍ��킹�Ēx����������
			while (mClock.getElapsedTime().asMilliseconds() - ticksCount.asMilliseconds() < 16);

			float deltaTime = static_cast<float>(mClock.getElapsedTime().asMilliseconds() - ticksCount.asMilliseconds()) / 1000.0f;
			if (deltaTime > 0.05f)
			{
				deltaTime = 0.05f;
			}
			ticksCount = mClock.getElapsedTime();

			// �C�x���g�L���[�����݂���ꍇ�A����ɉ�����������S�čs��
			sf::Event event;
			while (mWindow->pollEvent(event))
			{
				if (event.type == sf::Event::Closed ||
					event.key.code == sf::Keyboard::Escape ||
					event.key.code == sf::Keyboard::Enter)
				{
					isChildWindowOpened = false;
				}

				mGui->handleEvent(event);
			}

			mWindow->clear();

			// �w�ʂ���`��
			for (auto& sprite : mSprites)
			{
				sprite->Draw(mWindow);
			}

			// UI�̓Q�[���I�u�W�F�N�g�̏�ɕ`�悷��̂ł����ɏ���������
			for (const auto& ui : mUIStack)
			{
				ui->Draw(mWindow);
			}

			mGui->draw(); // TGUI�̕`��
			mWindow->display();
		}

		// �E�B���h�E�̃��������
		child->close();

		// �Q�[�����[�v���ĊJ
		mWindow->setActive(true);
	}
}

std::vector<sf::Vector2i> Game::GetBaggagesPos() const
{
	std::vector<sf::Vector2i> result{};

	for (const auto& baggage : mBaggages)
	{
		result.emplace_back(baggage->GetBoardCoordinate());
	}

	return result;
}

int Game::GetBaggageNumLimit(const sf::Vector2i& size, const double& wallRate) const
{
	int result = 1;

	result = static_cast<int>(std::floor(static_cast<double>((size.x - 1) * (size.y - 1)) * (1.0 - wallRate)));

	return result;
}

void Game::ResetParameters()
{
	// ���O���N���A
	mLogs.clear();

	// ����������
	mInitialPlayerPos = sf::Vector2i{ -1, -1 };
	mInitialBaggagePos.clear();
	mGoalPos.clear();
	mBoardState.clear();
	mCurrentKey.clear();
	mStep = 0;

	// HUDHelper���폜
	delete mHUDHelper;
	mHUDHelper = nullptr;
}
