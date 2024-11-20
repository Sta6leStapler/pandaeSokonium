#pragma once

#include "SFML/Window/Event.hpp"
#include "IComponent.h"

class IActor
{
	// �f�X�g���N�^
	virtual ~IActor() = default;

	// �Q�[���N���X�̋@�\���g�����ɌĂяo���֐�
	virtual class Game* GetGame() = 0;

	// Game�N���X����X�V�������Ăяo��
	virtual void Update(float deltaTime) = 0;

	// �A�N�^�[�����S�ẴR���|�[�l���g���X�V
	virtual void UpdateComponents(float deltaTime) = 0;

	// Game�N���X������͏������Ăяo��
	virtual void ProcessInput(const sf::Event::KeyEvent* keyState) = 0;

	// �R���|�[�l���g�̒ǉ��ƍ폜
	virtual void AddComponent(class IComponent* component) = 0;
	virtual void RemoveComponent(class IComponent* component) = 0;

};

