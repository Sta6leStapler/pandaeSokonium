#include "Player.h"
#include "Game.h"
#include <fstream>

#include <iostream>

Player::Player(Game* game)
	: mState(State::EActive)
	, mPosition(0.0f, 0.0f)
	, mScale(sf::Vector2f(1.0f, 1.0f))
	, mRotation(0.0f)
	, mGame(game)
	, mBoardName(game->GetFilename(0))
	, mDirection(ESouth)
	, prevDirection(ESouth)
	, mMoveCooldown(0.0f)
	, mDetection(false)
{
	mGame->AddActor(this);

	// �Ֆʃf�[�^��Game�N���X����擾����
	std::vector<std::string> lines = game->GetBoardData()[mBoardName];
	
	// �Ֆʂ̉����𓾂�
	int maxX = 0;
	for (const auto& item : lines)
	{
		if (item.length() > maxX)
		{
			maxX = (int)item.length();
		}
	}

	// �v���C���[�̏������W�𓾂�
	{
		int i = 0, j = 0;
		for (const auto& line : lines)
		{
			for (const auto& item : line)
			{
				if (item == '@')
				{
					mBoardCoordinate = sf::Vector2i(j, i);
					break;
				}
				else if (item == '+')
				{
					mBoardCoordinate = sf::Vector2i(j, i);
					break;
				}
				j++;
			}
			i++;
			j = 0;
		}
	}

	// �E�B���h�E�T�C�Y
	// 2024_09_05 �`��͈͂ɏC��
	BoundingBox viewArea = mGame->GetBoardViewArea();

	// �v���C���[�̃^�C����p��
	// �R���|�[�l���g���쐬
	PlayerComponent* pc = new PlayerComponent(this, 100, 150);
	
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
	pc->SetTexture(mTextures[Direction::ESouth]);
	mComponent = pc;

	// �X�P�[�����O�ƈʒu�̏��������s��
	// 2024_09_05 �ՖʂƓ��l�ɏC��
	float minScale = std::min((viewArea.second.x - viewArea.first.x) / static_cast<float>(mTextures[Direction::EEast]->getSize().x * maxX), (viewArea.second.y - viewArea.first.y) / static_cast<float>(mTextures[Direction::EEast]->getSize().y * lines.size()));
	mScale = sf::Vector2f(minScale, minScale);

	// �Ֆʂ̗]���̕���������
	// 2024_09_05 �ՖʂƓ��l�ɏC��
	mPosition = sf::Vector2f(
		((viewArea.second.x - viewArea.first.x) - (static_cast<float>(mTextures[Direction::EEast]->getSize().x * maxX) * minScale)) / 2.0f + static_cast<float>(mTextures[Direction::EEast]->getSize().x * mBoardCoordinate.x) * mScale.x,
		((viewArea.second.y - viewArea.first.y) - (static_cast<float>(mTextures[Direction::EEast]->getSize().y * lines.size()) * minScale)) / 2.0f + static_cast<float>(mTextures[Direction::EEast]->getSize().y * mBoardCoordinate.y) * mScale.y
	);
}

Player::~Player()
{
	mGame->RemoveActor(this);
	// �R���|�[�l���g���폜����
	delete mComponent;
}

void Player::Update(float deltaTime)
{
	// TODO �X�V������肪�X�V�O�̔ՖʂɂȂ��Ă���

	mMoveCooldown -= deltaTime;

	if (mState.GetEState() == State::EActive)
	{
		UpdateComponents(deltaTime);

		// ���̃A�N�^�[���L�̍X�V����������Ώ���
		// �����̍X�V����������R���|�[�l���g�̃X�v���C�g��؂�ւ���
		mComponent->SetTexture(mTextures[mDirection]);

		// �v���C���[��ו��𔲂����Ֆʂ̏��
		std::vector<std::string> boardState = mGame->GetBoardState();

		if (mDetection)
		{
			// �ړ���̍��W
			sf::Vector2i destination = sf::Vector2i(mBoardCoordinate.x + static_cast<int>(std::cos(mRotation)), mBoardCoordinate.y + static_cast<int>(std::sin(mRotation)));

			mDetection = false;

			// �܂��ړ��悪�ǂłȂ����m�F����
			if (boardState[destination.y][destination.x] != '#')
			{
				// �ړ���ɉו������邩�`�F�b�N
				const auto boxes = mGame->GetBaggages();
				std::vector<sf::Vector2i> boxesPos;
				for (const auto& item : boxes)
				{
					boxesPos.push_back(item->GetBoardCoordinate());
				}
				auto iter = boxesPos.end();
				for (auto it = boxesPos.begin(); it != boxesPos.end(); ++it)
				{
					if (*it == destination)
					{
						iter = it;
						break;
					}
				}

				if (iter == boxesPos.end())
				{
					// �ו����Ȃ������ꍇ
					// �ړ��O�̍��W
					sf::Vector2i lastPos = mBoardCoordinate;

					// �ړ�����
					mBoardCoordinate = destination;
					mPosition = sf::Vector2f(
						mPosition.x + static_cast<float>(mTextures[mDirection]->getSize().x) * mScale.x * std::cos(mRotation),
						mPosition.y + static_cast<float>(mTextures[mDirection]->getSize().y) * mScale.y * std::sin(mRotation)
					);

					// �Q�[���N���X���烍�O���������ފ֐����Ăяo��
					mGame->RemoveRedo();
					mGame->AddStep();
					mGame->AddLog(lastPos, destination, prevDirection, mDirection);
					prevDirection = mDirection;
				}
				else
				{
					// �ו����������ꍇ
					// �X�ɂ�����}�X��ɉו������邩�`�F�b�N
					iter = boxesPos.end();
					for (auto it = boxesPos.begin(); it != boxesPos.end(); ++it)
					{
						if (*it == sf::Vector2i(destination.x + static_cast<int>(std::cos(mRotation)), destination.y + static_cast<int>(std::sin(mRotation))))
						{
							iter = it;
							break;
						}
					}
					// �ו���2����ł��炸�A�ǂɓ˂�������Ȃ���Ήו����ƈړ�
					if (iter == boxesPos.end() && boardState[destination.y + static_cast<int>(std::sin(mRotation))][destination.x + static_cast<int>(std::cos(mRotation))] != '#')
					{
						// �ړ��O�̍��W
						sf::Vector2i lastPos = mBoardCoordinate;

						// �ړ�����
						mBoardCoordinate = destination;
						mPosition = sf::Vector2f(
							mPosition.x + static_cast<float>(mTextures[mDirection]->getSize().x) * mScale.x * std::cos(mRotation),
							mPosition.y + static_cast<float>(mTextures[mDirection]->getSize().y) * mScale.y * std::sin(mRotation)
						);

						sf::Vector2i baggageLastPos, baggageDestination;

						for (auto& item : mGame->GetBaggages())
						{
							if (item->GetBoardCoordinate() == destination)
							{
								baggageLastPos = sf::Vector2i(item->GetBoardCoordinate().x, item->GetBoardCoordinate().y);
								baggageDestination = sf::Vector2i(item->GetBoardCoordinate().x + static_cast<int>(std::cos(mRotation)), item->GetBoardCoordinate().y + static_cast<int>(std::sin(mRotation)));
								item->SetBoardCoordinate(baggageDestination);
								break;
							}
						}

						// �Q�[���N���X���烍�O���������ފ֐����Ăяo��
						mGame->RemoveRedo();
						mGame->AddStep();
						mGame->AddLog(lastPos, destination, baggageLastPos, baggageDestination, prevDirection, mDirection);
						prevDirection = mDirection;
					}
				}
			}
		}
	}
}

void Player::UpdateComponents(float deltaTime)
{
	mComponent->Update(deltaTime);
}

void Player::ProcessInput(const sf::Event::KeyEvent* keyState)
{
	if (mState.GetEState() == State::EActive)
	{
		// �A�N�^�[�����S�Ă�Component�̓��͏������s��
		// �ǂ�Component�����ɓƎ��̏������������Ă��Ȃ���Ή������Ȃ�
		mComponent->ProcessInput(keyState);

		// ���̃A�N�^�[���L�̐U�镑��������Ώ���
		// �ړ����͂̃N�[���_�E����0�ȉ��Ȃ���͂��󂯕t����
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
				break;
			case sf::Keyboard::Up:
				if (prevKeys.code != sf::Keyboard::Up)
				{
					mDirection = Direction::ENorth;
					mRotation = static_cast<float>(3.0 * std::acos(-1) / 2.0);
				}
				break;
			case sf::Keyboard::Left:
				if (prevKeys.code != sf::Keyboard::Left)
				{
					mDirection = Direction::EWest;
					mRotation = static_cast<float>(std::acos(-1));
				}
				break;
			case sf::Keyboard::Down:
				if (prevKeys.code != sf::Keyboard::Down)
				{
					mDirection = Direction::ESouth;
					mRotation = static_cast<float>(std::acos(-1) / 2.0);
				}
				break;
			default:
				mMoveCooldown = 0.0f;
				mDetection = false;
				break;
			}
		}
	}

	prevKeys = *keyState;
}

void Player::AddComponent(PlayerComponent* component)
{
	mComponent = component;
}

void Player::RemoveComponent(PlayerComponent* component)
{
	mComponent = nullptr;
}

void Player::SetBoardCoordinate(const sf::Vector2i boardCoordinate)
{
	mPosition = sf::Vector2f(
		mPosition.x + static_cast<float>(mTextures[mDirection]->getSize().x) * mScale.x * static_cast<float>(boardCoordinate.x - mBoardCoordinate.x),
		mPosition.y + static_cast<float>(mTextures[mDirection]->getSize().y) * mScale.y * static_cast<float>(boardCoordinate.y - mBoardCoordinate.y)
	);
	mBoardCoordinate = boardCoordinate;
}

void Player::Reload()
{
	// �Ֆʃf�[�^��Game�N���X����擾����
	mBoardName = mGame->GetFilename(0);
	std::vector<std::string> lines = mGame->GetBoardData()[mBoardName];

	// �Ֆʂ̉����𓾂�
	int maxX = 0;
	for (const auto& item : lines)
	{
		if (item.length() > maxX)
		{
			maxX = (int)item.length();
		}
	}

	// �v���C���[�̏������W�𓾂�
	{
		int i = 0, j = 0;
		for (const auto& line : lines)
		{
			for (const auto& item : line)
			{
				if (item == '@')
				{
					mBoardCoordinate = sf::Vector2i(j, i);
					break;
				}
				else if (item == '+')
				{
					mBoardCoordinate = sf::Vector2i(j, i);
					break;
				}
				j++;
			}
			i++;
			j = 0;
		}
	}

	// �E�B���h�E�T�C�Y
	sf::Vector2f wSize = mGame->GetWindowSize();

	// �v���C���[�̃^�C����p��
	// �R���|�[�l���g���쐬
	PlayerComponent* pc = new PlayerComponent(this, 100, 150);

	// �R���|�[�l���g�Ƀe�N�X�`�����Z�b�g
	pc->SetTexture(mTextures[Direction::ESouth]);
	mComponent = pc;

	// 2024_09_05 �`��͈͂ɏC��
	BoundingBox viewArea = mGame->GetBoardViewArea();

	// �X�P�[�����O�ƈʒu�̏��������s��
	// 2024_09_05 �ՖʂƓ��l�ɏC��
	float minScale = std::min((viewArea.second.x - viewArea.first.x) / static_cast<float>(mTextures[Direction::EEast]->getSize().x * maxX), (viewArea.second.y - viewArea.first.y) / static_cast<float>(mTextures[Direction::EEast]->getSize().y * lines.size()));
	mScale = sf::Vector2f(minScale, minScale);

	// �Ֆʂ̗]���̕���������
	// 2024_09_05 �ՖʂƓ��l�ɏC��
	mPosition = sf::Vector2f(
		((viewArea.second.x - viewArea.first.x) - (static_cast<float>(mTextures[Direction::EEast]->getSize().x * maxX) * minScale)) / 2.0f + static_cast<float>(mTextures[Direction::EEast]->getSize().x * mBoardCoordinate.x) * mScale.x,
		((viewArea.second.y - viewArea.first.y) - (static_cast<float>(mTextures[Direction::EEast]->getSize().y * lines.size()) * minScale)) / 2.0f + static_cast<float>(mTextures[Direction::EEast]->getSize().y * mBoardCoordinate.y) * mScale.y
	);
}