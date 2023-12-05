#include "BackGround.h"
#include "Game.h"

BackGround::BackGround(Game* game)
	: mState(State::EActive)
	, mPosition(0.0, 0.0)
	, mScale(sf::Vector2f(1.0f, 1.0f))
	, mRotation(0.0f)
	, mGame(game)
{
	mGame->AddActor(this);

	// SpriteComponent���쐬����
	// ���ƕǂ̃^�C����p��
	// TODO �ǂݍ��񂾔Ֆʃf�[�^�ɉ������X�e�[�W�̃e�N�X�`�����쐬����
	BackGroundComponent* gbc = new BackGroundComponent(this, 100, 150);
	gbc->SetTexture(game->LoadTexture("Assets/BackGround.png"));
}

BackGround::~BackGround()
{
	mGame->RemoveActor(this);
	// �R���|�[�l���g���폜����
	while (!mComponents.empty())
	{
		delete mComponents.back();
	}
}

void BackGround::Update(float deltaTime)
{
	if (mState.GetEState() == State::EActive)
	{
		UpdateComponents(deltaTime);

		// ���̃A�N�^�[���L�̍X�V����������Ώ���
	}
}

void BackGround::UpdateComponents(float deltaTime)
{
	for (auto item : mComponents)
	{
		item->Update(deltaTime);
	}
}

void BackGround::ProcessInput(const sf::Event::KeyEvent* keyState)
{
	if (mState.GetEState() == State::EActive)
	{
		// �A�N�^�[�����S�Ă�Component�̓��͏������s��
		// �ǂ�Component�����ɓƎ��̏������������Ă��Ȃ���Ή������Ȃ�

		for (auto item : mComponents)
		{
			item->ProcessInput(keyState);
		}

		// ���̃A�N�^�[���L�̐U�镑��������Ώ���

	}

}

void BackGround::AddComponent(BackGroundComponent* component)
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

void BackGround::RemoveComponent(BackGroundComponent* component)
{
	auto iter = std::find(mComponents.begin(), mComponents.end(), component);
	if (iter != mComponents.end())
	{
		mComponents.erase(iter);
	}
}