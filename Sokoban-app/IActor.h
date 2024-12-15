#pragma once

#include "SFML/Graphics.hpp"

class IActor
{
public:
	virtual ~IActor() = default;

	// Game�N���X����X�V�������Ăяo��
	virtual void Update(float deltaTime) = 0;

	// �A�N�^�[�����S�ẴR���|�[�l���g���X�V
	virtual void UpdateComponents(float deltaTime) = 0;

	// Game�N���X������͏������Ăяo��
	virtual void ProcessInput(const sf::Event* event) = 0;

	// �A�N�^�[�����S�ẴR���|�[�l���g�̓��͏������s��
	virtual void ProcessInputComponents(const sf::Event* event) = 0;

	// �R���|�[�l���g�̒ǉ��ƍ폜
	virtual void AddComponent(class IComponent* component) = 0;
	virtual void RemoveComponent(class IComponent* component) = 0;

	// ��Ԃ��A�N�e�B�u���N���[�W���O���ҋ@�����Ǘ�
	enum class ActorState { EActive, EClosing, EPending };

	// �Q�b�^�[�ƃZ�b�^�[
	virtual class Game* GetGame() = 0;
	virtual ActorState GetState() = 0;
	virtual void SetState(const ActorState state) = 0;
	virtual sf::Vector2f GetPosition() = 0;
	virtual sf::Vector2f GetScale() = 0;
	virtual float GetRotation() = 0;
};