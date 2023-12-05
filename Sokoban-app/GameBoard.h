#pragma once

#include "SFML/Graphics.hpp"
#include "State.h"
#include <unordered_map>
#include <cmath>

class GameBoard
{
public:
	GameBoard(class Game* game);
	virtual ~GameBoard();

	// Game�N���X����X�V�������Ăяo��
	void Update(float deltaTime);

	// �A�N�^�[�����S�ẴR���|�[�l���g���X�V
	void UpdateComponents(float deltaTime);

	// Game�N���X������͏������Ăяo��
	void ProcessInput(const sf::Event::KeyEvent* keyState);

	// �R���|�[�l���g�̒ǉ��ƍ폜
	void AddComponent(class GameBoardComponent* component);
	void RemoveComponent(class GameBoardComponent* component);

	// �Ֆʂ��X�V����
	void Reload();

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

	class GameBoardComponent* mGameBoardComponent;

	std::string mBoardName;

	// �R���|�[�l���g�Ɏg���e�N�X�`��
	std::unordered_map<std::string, sf::Texture*> mTextures;

	class Game* mGame;

};