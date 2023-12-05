#pragma once

#include "SFML/Graphics.hpp"
#include "State.h"
#include <cmath>

class BackGround
{
public:
	BackGround(class Game* game);
	virtual ~BackGround();

	// Game�N���X����X�V�������Ăяo��
	void Update(float deltaTime);

	// �A�N�^�[�����S�ẴR���|�[�l���g���X�V
	void UpdateComponents(float deltaTime);

	// Game�N���X������͏������Ăяo��
	void ProcessInput(const sf::Event::KeyEvent* keyState);

	// �R���|�[�l���g�̒ǉ��ƍ폜
	void AddComponent(class BackGroundComponent* component);
	void RemoveComponent(class BackGroundComponent* component);

	// �Q�b�^�[�ƃZ�b�^�[
	State GetState() const { return mState; }
	void SetState(const State state) { mState = state; }

	const sf::Vector2f& GetPosition() const { return mPosition; }
	void SetPosition(const sf::Vector2f& pos) { mPosition = pos; }
	sf::Vector2f GetScale() const { return mScale; }
	void SetScale(sf::Vector2f scale) { mScale = scale; }
	float GetRotation() const { return mRotation; }
	void SetRotation(float rotation) { mRotation = rotation; }

	sf::Vector2f GetForward() const { return sf::Vector2f(std::cos(mRotation), -std::sin(mRotation)); }

	class Game* GetGame() { return mGame; }

private:
	// �A�N�^�[�̏��
	State mState;

	// �ϊ�
	sf::Vector2f mPosition;
	sf::Vector2f mScale;
	float mRotation;

	std::vector<class BackGroundComponent*> mComponents;

	class Game* mGame;

};