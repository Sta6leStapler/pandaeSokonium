#pragma once

#include "SFML/Graphics.hpp"

class IComponent
{
public:
	// �f�X�g���N�^
	virtual ~IComponent() = default;

	// �f���^�^�C���ł��̃R���|�[�l���g���X�V����
	virtual void Update(float deltaTime) = 0;

	// �R���|�[�l���g���L�̓��͏���
	virtual void ProcessInput(const sf::Event* event) = 0;

	// �Q�b�^�[�ƃZ�b�^�[
	virtual int GetUpdateOrder() = 0;
};