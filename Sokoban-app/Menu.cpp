#include "Menu.h"
#include "Game.h"
#include <fstream>

#include <iostream>

Menu::Menu(Game* game)
	: mState(State::EActive)
	, mPosition(0.0f, 0.0f)
	, mScale(sf::Vector2f(1.0f, 1.0f))
	, mRotation(0.0f)
	, mGame(game)
	, mBoardName(game->GetFilename(0))
	, mMoveCooldown(0.0f)
	, mDetection(false)
{
	mGame->AddActor(this);

	// �`��͈�
	BoundingBox viewArea = mGame->GetBoardViewArea();

	// �v���C���[�̃^�C����p��
	// �R���|�[�l���g���쐬
	MenuComponent* mc = new MenuComponent(this, 100, 150);

	// �t�@�C����ǂݍ���
	std::string filename = "Assets/playerE.png";
	sf::Image image;
	image.loadFromFile(filename);
	mTextures.emplace(Direction::EEast, game->LoadTexture(filename));
	filename = "Assets/playerN.png";
	image.loadFromFile(filename);
	mTextures.emplace(Direction::ENorth, game->LoadTexture(filename));
	filename = "Assets/playerW.png";
	image.loadFromFile(filename);
	mTextures.emplace(Direction::EWest, game->LoadTexture(filename));
	filename = "Assets/playerS.png";
	image.loadFromFile(filename);
	mTextures.emplace(Direction::ESouth, game->LoadTexture(filename));


	// �R���|�[�l���g�Ƀe�N�X�`�����Z�b�g
	mc->SetTexture(mTextures[Direction::ESouth]);
	mComponent = mc;

	// �X�P�[�����O�ƈʒu�̏��������s��
	float minScale = std::min((viewArea.second.x - viewArea.first.x) / static_cast<float>(mTextures[Direction::EEast]->getSize().x * maxX), (viewArea.second.y - viewArea.first.y) / static_cast<float>(mTextures[Direction::EEast]->getSize().y * lines.size()));
	mScale = sf::Vector2f(minScale, minScale);

	// �]���̕���������
	mPosition = sf::Vector2f(
		((viewArea.second.x - viewArea.first.x) - (static_cast<float>(mTextures[Direction::EEast]->getSize().x * maxX) * minScale)) / 2.0f + static_cast<float>(mTextures[Direction::EEast]->getSize().x * mBoardCoordinate.x) * mScale.x,
		((viewArea.second.y - viewArea.first.y) - (static_cast<float>(mTextures[Direction::EEast]->getSize().y * lines.size()) * minScale)) / 2.0f + static_cast<float>(mTextures[Direction::EEast]->getSize().y * mBoardCoordinate.y) * mScale.y
	);
}

Menu::~Menu()
{
	mGame->RemoveActor(this);
	// �R���|�[�l���g���폜����
	delete mComponent;
}

void Menu::Update(float deltaTime)
{
	mMoveCooldown -= deltaTime;

	if (mState.GetEState() == State::EActive)
	{
		UpdateComponents(deltaTime);

		// ���̃A�N�^�[���L�̍X�V����������Ώ���

	}
}

void Menu::UpdateComponents(float deltaTime)
{
	mComponent->Update(deltaTime);
}

void Menu::ProcessInput(const sf::Event::KeyEvent* keyState)
{
	if (mState.GetEState() == State::EActive)
	{
		// �A�N�^�[�����S�Ă�Component�̓��͏������s��
		// �ǂ�Component�����ɓƎ��̏������������Ă��Ȃ���Ή������Ȃ�
		mComponent->ProcessInput(keyState);

		// ���̃A�N�^�[���L�̐U�镑��������Ώ���
		// ���͂̃N�[���_�E����0�ȉ��Ȃ���͂��󂯕t����
		if (mMoveCooldown <= 0.0f)
		{
			mMoveCooldown = 0.13f;
			mDetection = true;
			switch (keyState->code)
			{
			case sf::Keyboard::Right:
				if (prevKeys.code != sf::Keyboard::Right)
				{
					mDirection = Direction::EEast;
					mRotation = 0.0f;
				}
			default:
				mMoveCooldown = 0.0f;
				mDetection = false;
				break;
			}
		}
	}

	prevKeys = *keyState;
}

void Menu::AddComponent(MenuComponent* component)
{
	mComponent = component;
}

void Menu::RemoveComponent(MenuComponent* component)
{
	mComponent = nullptr;
}

void Menu::Reload()
{
	// TODO �e��K�v�ȃf�[�^��Game�N���X����擾����
	

	// �E�B���h�E�T�C�Y
	sf::Vector2f wSize = mGame->GetWindowSize();

	// �v���C���[�̃^�C����p��
	// �R���|�[�l���g���쐬
	MenuComponent* mc = new MenuComponent(this, 100, 150);

	// �R���|�[�l���g�Ƀe�N�X�`�����Z�b�g
	mc->SetTexture(mTextures[Direction::ESouth]);
	mComponent = mc;

	// �X�P�[�����O�ƈʒu�̏��������s��
	float minScale = std::min(wSize.x / static_cast<float>(mTextures[Direction::EEast]->getSize().x * maxX), wSize.y / static_cast<float>(mTextures[Direction::EEast]->getSize().y * lines.size()));
	mScale = sf::Vector2f(minScale, minScale);

	// �Ֆʂ̗]���̕���������
	mPosition = sf::Vector2f(
		(wSize.x - (static_cast<float>(mTextures[Direction::EEast]->getSize().x * maxX) * minScale)) / 2.0f + static_cast<float>(mTextures[Direction::EEast]->getSize().x * mBoardCoordinate.x) * mScale.x,
		(wSize.y - (static_cast<float>(mTextures[Direction::EEast]->getSize().y * lines.size()) * minScale)) / 2.0f + static_cast<float>(mTextures[Direction::EEast]->getSize().y * mBoardCoordinate.y) * mScale.y
	);
}