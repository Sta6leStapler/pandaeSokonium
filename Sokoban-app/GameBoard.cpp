#include "GameBoard.h"

#include "Game.h"
#include "SpriteComponent.h"

#include <iostream>
#include <fstream>

GameBoard::GameBoard(Game* game)
	: mState(IActor::ActorState::EActive)
	, mPosition(0.0, 0.0)
	, mScale(sf::Vector2f(1.0f, 1.0f))
	, mRotation(0.0f)
	, mComponents(std::vector<IComponent*>{})
	, mSpriteComponent(nullptr)
	, mTextures(std::unordered_map<std::string, sf::Texture*>{})
	, mGame(game)
	, mBoardName(game->GetCurrentKey())
{
	mGame->AddActor(this);

	// SpriteComponent���쐬����
	// ���ƕǂ̃^�C����p��
	// �ǂݍ��񂾔Ֆʃf�[�^�ɉ������X�e�[�W�̃e�N�X�`�����쐬����
	mSpriteComponent = new SpriteComponent(this, 100, 50);
	std::string filename = "Assets/Floor.png";
	mTextures.emplace(filename, game->LoadTexture(filename));
	filename = "Assets/Wall.png";
	mTextures.emplace(filename, game->LoadTexture(filename));
	filename = "Assets/Goal.png";
	mTextures.emplace(filename, game->LoadTexture(filename));
	
	std::vector<std::string> lines = game->GetBoardData()[mBoardName];

	// �Ֆʂ̉����𑵂���
	size_t maxX = 0;
	for (const auto& item : lines)
	{
		if (item.length() > maxX)
		{
			maxX = item.length();
		}
	}

	// �Ֆʂ̃e�N�X�`�����쐬
	sf::RenderTexture* boardTexture = new sf::RenderTexture();
	// 2024_09_05 �E�B���h�E�T�C�Y����`��͈͂ɕύX
	BoundingBox viewArea = mGame->GetBoardViewArea();

	// ���ƕǂ̃X�v���C�g���쐬
	sf::Sprite tmpFloor, tmpWall, tmpGoal;
	tmpFloor.setTexture(*mTextures["Assets/Floor.png"]);
	tmpWall.setTexture(*mTextures["Assets/Wall.png"]);
	tmpGoal.setTexture(*mTextures["Assets/Goal.png"]);
	
	// �X�v���C�g���e�N�X�`���ɓ\��t���Ă���
	boardTexture->create(static_cast<int>(tmpFloor.getGlobalBounds().width * maxX), static_cast<int>(tmpFloor.getGlobalBounds().height * lines.size()));
	for (int i = 0; i < lines.size(); i++)
	{
		for (int j = 0; j < lines[i].length(); j++)
		{
			switch (lines[i][j])
			{
			case ' ':
				tmpFloor.setPosition(tmpFloor.getGlobalBounds().width * static_cast<float>(j), tmpFloor.getGlobalBounds().height * static_cast<float>(i));
				boardTexture->draw(tmpFloor);
				break;
			case '#':
				tmpWall.setPosition(tmpWall.getGlobalBounds().width * static_cast<float>(j), tmpWall.getGlobalBounds().height * static_cast<float>(i));
				boardTexture->draw(tmpWall);
				break;
			default:
				break;
			case '$':
				tmpFloor.setPosition(tmpFloor.getGlobalBounds().width * static_cast<float>(j), tmpFloor.getGlobalBounds().height * static_cast<float>(i));
				boardTexture->draw(tmpFloor);
				break;
			case '.':
				tmpGoal.setPosition(tmpGoal.getGlobalBounds().width * static_cast<float>(j), tmpGoal.getGlobalBounds().height * static_cast<float>(i));
				boardTexture->draw(tmpGoal);
				break;
			case '*':
				tmpGoal.setPosition(tmpGoal.getGlobalBounds().width * static_cast<float>(j), tmpGoal.getGlobalBounds().height * static_cast<float>(i));
				boardTexture->draw(tmpGoal);
				break;
			case '@':
				tmpFloor.setPosition(tmpFloor.getGlobalBounds().width * static_cast<float>(j), tmpFloor.getGlobalBounds().height * static_cast<float>(i));
				boardTexture->draw(tmpFloor);
				break;
			case '+':
				tmpGoal.setPosition(tmpGoal.getGlobalBounds().width * static_cast<float>(j), tmpGoal.getGlobalBounds().height * static_cast<float>(i));
				boardTexture->draw(tmpGoal);
			}
		}
	}

	sf::Texture* tmpTexture = new sf::Texture(boardTexture->getTexture());
	delete boardTexture;

	mSpriteComponent->SetTexture(tmpTexture);

	// �X�P�[�����O�ƈʒu�̏��������s��
	// �\���G���A�̃T�C�Y�@/ �Ֆʂ̃T�C�Y �����߂�
	// �\���G���A�̕�����������΃^�C���͏k�����ׂ��ŁA�t�Ȃ�g�傷��ׂ�
	float minScale = std::min((viewArea.second.x - viewArea.first.x) / static_cast<float>(tmpTexture->getSize().x),
		(viewArea.second.y - viewArea.first.y) / static_cast<float>(tmpTexture->getSize().y));

	mScale = sf::Vector2f(minScale, -minScale);

	// �]���̕�������������
	// *���� UI���ł����ꍇ�̓I�t�Z�b�g�������Ă���
	mPosition = sf::Vector2f
	{
		viewArea.first.x + (viewArea.second.x - viewArea.first.x - static_cast<float>(tmpTexture->getSize().x) * mScale.x) / 2.0f,
		viewArea.first.y + (viewArea.second.y - viewArea.first.y - static_cast<float>(tmpTexture->getSize().y) * mScale.y) / 2.0f
	};
}

GameBoard::~GameBoard()
{
	mGame->RemoveActor(this);
	// �R���|�[�l���g���폜����
	while (!mComponents.empty())
	{
		delete mComponents.back();
	}
}

void GameBoard::Update(float deltaTime)
{
	if (mState == IActor::ActorState::EActive)
	{
		UpdateComponents(deltaTime);

		// ���̃A�N�^�[���L�̍X�V����������Ώ���
	}
}

void GameBoard::UpdateComponents(float deltaTime)
{
	for (auto item : mComponents)
	{
		item->Update(deltaTime);
	}
}

void GameBoard::ProcessInput(const sf::Event* event)
{
	if (mState == IActor::ActorState::EActive)
	{
		// �A�N�^�[�����S�Ă�Component�̓��͏������s��
		// �ǂ�Component�����ɓƎ��̏������������Ă��Ȃ���Ή������Ȃ�
		ProcessInputComponents(event);

		// ���̃A�N�^�[���L�̐U�镑��������Ώ���
		
	}

}

void GameBoard::ProcessInputComponents(const sf::Event* event)
{
	for (auto& component : mComponents)
	{
		component->ProcessInput(event);
	}
}

void GameBoard::AddComponent(IComponent* component)
{
	int myOrder = component->GetUpdateOrder();
	auto iter = mComponents.begin();
	for (;
		iter != mComponents.end();
		++iter)
	{
		if (myOrder < (*iter)->GetUpdateOrder())
		{
			break;
		}
	}

	mComponents.insert(iter, component);
}

void GameBoard::RemoveComponent(IComponent* component)
{
	auto iter = std::find(mComponents.begin(), mComponents.end(), component);
	if (iter != mComponents.end())
	{
		mComponents.erase(iter);
	}
}

void GameBoard::Reload()
{
	mBoardName = mGame->GetCurrentKey();
	std::vector<std::string> lines = mGame->GetBoardData()[mBoardName];

	// �Ֆʂ̉����𑵂���
	size_t maxX = 0;
	for (const auto& item : lines)
	{
		if (item.length() > maxX)
		{
			maxX = item.length();
		}
	}

	// �Ֆʂ̃e�N�X�`�����쐬
	sf::RenderTexture* boardTexture = new sf::RenderTexture();
	sf::Vector2f wSize = mGame->GetWindowSize();

	// ���ƕǂ̃X�v���C�g���쐬
	sf::Sprite tmpFloor, tmpWall, tmpGoal;
	tmpFloor.setTexture(*mTextures["Assets/Floor.png"]);
	tmpWall.setTexture(*mTextures["Assets/Wall.png"]);
	tmpGoal.setTexture(*mTextures["Assets/Goal.png"]);

	// �X�v���C�g���e�N�X�`���ɓ\��t���Ă���
	boardTexture->create(static_cast<int>(tmpFloor.getGlobalBounds().width * maxX), static_cast<int>(tmpFloor.getGlobalBounds().height * lines.size()));
	for (int i = 0; i < lines.size(); i++)
	{
		for (int j = 0; j < lines[i].length(); j++)
		{
			switch (lines[i][j])
			{
			case ' ':
				tmpFloor.setPosition(tmpFloor.getGlobalBounds().width * static_cast<float>(j), tmpFloor.getGlobalBounds().height * static_cast<float>(i));
				boardTexture->draw(tmpFloor);
				break;
			case '#':
				tmpWall.setPosition(tmpWall.getGlobalBounds().width * static_cast<float>(j), tmpWall.getGlobalBounds().height * static_cast<float>(i));
				boardTexture->draw(tmpWall);
				break;
			default:
				break;
			case '$':
				tmpFloor.setPosition(tmpFloor.getGlobalBounds().width * static_cast<float>(j), tmpFloor.getGlobalBounds().height * static_cast<float>(i));
				boardTexture->draw(tmpFloor);
				break;
			case '.':
				tmpGoal.setPosition(tmpGoal.getGlobalBounds().width * static_cast<float>(j), tmpGoal.getGlobalBounds().height * static_cast<float>(i));
				boardTexture->draw(tmpGoal);
				break;
			case '*':
				tmpGoal.setPosition(tmpGoal.getGlobalBounds().width * static_cast<float>(j), tmpGoal.getGlobalBounds().height * static_cast<float>(i));
				boardTexture->draw(tmpGoal);
				break;
			case '@':
				tmpFloor.setPosition(tmpFloor.getGlobalBounds().width * static_cast<float>(j), tmpFloor.getGlobalBounds().height * static_cast<float>(i));
				boardTexture->draw(tmpFloor);
				break;
			case '+':
				tmpGoal.setPosition(tmpGoal.getGlobalBounds().width * static_cast<float>(j), tmpGoal.getGlobalBounds().height * static_cast<float>(i));
				boardTexture->draw(tmpGoal);
			}
		}
	}

	sf::Texture* tmpTexture = new sf::Texture(boardTexture->getTexture());
	delete boardTexture;

	mSpriteComponent->SetTexture(tmpTexture);

	// 2024_09_05 �E�B���h�E�T�C�Y����`��͈͂ɕύX
	BoundingBox viewArea = mGame->GetBoardViewArea();

	// �X�P�[�����O�ƈʒu�̏��������s��
	// �\���G���A�̃T�C�Y�@/ �Ֆʂ̃T�C�Y �����߂�
	// �\���G���A�̕�����������΃^�C���͏k�����ׂ��ŁA�t�Ȃ�g�傷��ׂ�
	float minScale = std::min((viewArea.second.x - viewArea.first.x) / static_cast<float>(tmpTexture->getSize().x),
		(viewArea.second.y - viewArea.first.y) / static_cast<float>(tmpTexture->getSize().y));

	mScale = sf::Vector2f(minScale, -minScale);

	// �]���̕�������������
	// *���� UI���ł����ꍇ�̓I�t�Z�b�g�������Ă���
	mPosition = sf::Vector2f
	{
		viewArea.first.x + (viewArea.second.x - viewArea.first.x - static_cast<float>(tmpTexture->getSize().x) * mScale.x) / 2.0f,
		viewArea.first.y + (viewArea.second.y - viewArea.first.y - static_cast<float>(tmpTexture->getSize().y) * mScale.y) / 2.0f
	};

}