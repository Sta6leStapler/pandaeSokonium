#include "Baggage.h"
#include "Game.h"
#include <fstream>

Baggage::Baggage(Game* game, sf::Vector2i bCoordinate)
	: mState(State::EActive)
	, mPosition(0.0, 0.0)
	, mScale(sf::Vector2f(1.0f, 1.0f))
	, mRotation(0.0f)
	, mGame(game)
	, mBoardName(game->GetFilename(0))
	, mBoardCoordinate(bCoordinate)
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

	// �E�B���h�E�T�C�Y
	// 2024_09_05 �`��͈͂ɏC��
	BoundingBox viewArea = mGame->GetBoardViewArea();

	// �v���C���[�̃^�C����p��
	// �R���|�[�l���g���쐬
	BaggageComponent* pc = new BaggageComponent(this, 100, 150);
	// �����ʒu���S�[����ɂ��邩�ǂ���
	bState = BState::OnFloor;
	if (mGame->GetBoardState()[mBoardCoordinate.y][mBoardCoordinate.x] == '.')
	{
		bState = BState::OnGoal;
	}

	// �t�@�C����ǂݍ���
	std::string filename = "Assets/Box.png";
	sf::Image image;
	image.loadFromFile(filename);
	mTextures.emplace(BState::OnFloor, game->LoadTexture(filename));
	filename = "Assets/ShinyBox.png";
	image.loadFromFile(filename);
	mTextures.emplace(BState::OnGoal, game->LoadTexture(filename));
	filename = "Assets/DeadlockedBox.png";
	image.loadFromFile(filename);
	mTextures.emplace(BState::Deadlock, game->LoadTexture(filename));

	// �R���|�[�l���g�Ƀe�N�X�`�����Z�b�g
	pc->SetTexture(mTextures[bState]);
	mComponent = pc;

	// �X�P�[�����O�ƈʒu�̏��������s��
	// 2024_09_05 �ՖʂƓ��l�ɏC��
	float minScale = std::min((viewArea.second.x - viewArea.first.x) / static_cast<float>(mTextures[bState]->getSize().x * maxX), (viewArea.second.y - viewArea.first.y) / static_cast<float>(mTextures[bState]->getSize().y * lines.size()));
	mScale = sf::Vector2f(minScale, minScale);

	// �Ֆʂ̗]���̕���������
	// 2024_09_05 �ՖʂƓ��l�ɏC��
	mPosition = sf::Vector2f(
		((viewArea.second.x - viewArea.first.x) - (static_cast<float>(mTextures[bState]->getSize().x * maxX) * minScale)) / 2.0f + static_cast<float>(mTextures[bState]->getSize().x * mBoardCoordinate.x) * mScale.x,
		((viewArea.second.y - viewArea.first.y) - (static_cast<float>(mTextures[bState]->getSize().y * lines.size()) * minScale)) / 2.0f + static_cast<float>(mTextures[bState]->getSize().y * mBoardCoordinate.y) * mScale.y
	);
}

Baggage::~Baggage()
{
	mGame->RemoveActor(this);
	// �R���|�[�l���g���폜����
	delete mComponent;
}

void Baggage::Update(float deltaTime)
{
	if (mState.GetEState() == State::EActive)
	{
		UpdateComponents(deltaTime);

		// ���̃A�N�^�[���L�̍X�V����������Ώ���
		// ���̃A�N�^�[�̈ʒu�ɉ����ăe�N�X�`����ς���
		if (mGame->GetBoardState()[mBoardCoordinate.y][mBoardCoordinate.x] == '.')
		{
			mComponent->SetTexture(mTextures[BState::OnGoal]);
		}
		else if (mGame->GetHUDHelper()->isDeadlockedBaggage(mBoardCoordinate))
		{
			mComponent->SetTexture(mTextures[BState::Deadlock]);
		}
		else
		{
			mComponent->SetTexture(mTextures[BState::OnFloor]);
		}
	}
}

void Baggage::UpdateComponents(float deltaTime)
{
	mComponent->Update(deltaTime);
}

void Baggage::ProcessInput(const sf::Event::KeyEvent* keyState)
{
	if (mState.GetEState() == State::EActive)
	{
		// �A�N�^�[�����S�Ă�Component�̓��͏������s��
		// �ǂ�Component�����ɓƎ��̏������������Ă��Ȃ���Ή������Ȃ�
		mComponent->ProcessInput(keyState);

		// ���̃A�N�^�[���L�̐U�镑��������Ώ���
		// �ו��̓L�[�{�[�h���͂ɂ���Ď����I�ɂ͓����Ȃ�

	}

}

void Baggage::AddComponent(BaggageComponent* component)
{
	mComponent = component;
}

void Baggage::RemoveComponent(BaggageComponent* component)
{
	mComponent = nullptr;
}

void Baggage::SetBoardCoordinate(const sf::Vector2i boardCoordinate)
{
	mPosition = sf::Vector2f(
		mPosition.x + static_cast<float>(mTextures[bState]->getSize().x) * mScale.x * static_cast<float>(boardCoordinate.x - mBoardCoordinate.x),
		mPosition.y + static_cast<float>(mTextures[bState]->getSize().y) * mScale.y * static_cast<float>(boardCoordinate.y - mBoardCoordinate.y)
	);
	mBoardCoordinate = boardCoordinate;
}