#pragma once
#include "SFML/Graphics.hpp"
#include "State.h"
#include <unordered_map>
#include <cmath>

class Baggage
{
public:
	enum BState
	{
		OnGoal,
		OnFloor,
		Deadlock
	};

	Baggage(class Game* game, sf::Vector2i bCoordinate);
	virtual ~Baggage();

	// Game�N���X����X�V�������Ăяo��
	void Update(float deltaTime);

	// �A�N�^�[�����S�ẴR���|�[�l���g���X�V
	void UpdateComponents(float deltaTime);

	// Game�N���X������͏������Ăяo��
	void ProcessInput(const sf::Event::KeyEvent* keyState);

	// �R���|�[�l���g�̒ǉ��ƍ폜
	void AddComponent(class BaggageComponent* component);
	void RemoveComponent(class BaggageComponent* component);

	// �Q�b�^�[�ƃZ�b�^�[
	State GetState() const { return mState; }
	void SetState(const State state) { mState = state; }

	const sf::Vector2f& GetPosition() const { return mPosition; }
	void SetPosition(const sf::Vector2f& pos) { mPosition = pos; }
	sf::Vector2f GetScale() const { return mScale; }
	void SetScale(const sf::Vector2f scale) { mScale = scale; }
	float GetRotation() const { return mRotation; }
	void SetRotation(const float rotation) { mRotation = rotation; }

	sf::Vector2f GetForward() const { return sf::Vector2f(std::cos(mRotation), -std::sin(mRotation)); }

	class Game* GetGame() { return mGame; }

	sf::Vector2i GetBoardCoordinate() const { return mBoardCoordinate; }
	void SetBoardCoordinate(const sf::Vector2i boardCoordinate);

private:
	// �A�N�^�[�̏��
	class State mState;

	// �ϊ�
	sf::Vector2f mPosition;
	sf::Vector2f mScale;
	float mRotation;

	class BaggageComponent* mComponent;

	std::string mBoardName;

	// �R���|�[�l���g�Ɏg���e�N�X�`��
	std::unordered_map<BState, sf::Texture*> mTextures;

	class Game* mGame;

	// �ȉ��̓A�N�^�[���L�̃����o�ϐ�
	// �ו��̔Ֆʏ�̍��W
	sf::Vector2i mBoardCoordinate;

	// �ו����S�[����ɂ��邩�ǂ���
	BState bState;
};