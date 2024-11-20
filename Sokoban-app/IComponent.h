#pragma once

#include "SFML/Window/Event.hpp"

class IComponent
{
	// �f�X�g���N�^
	virtual ~IComponent() = default;

	// �f���^�^�C���ł��̃R���|�[�l���g���X�V����
	virtual void Update(float deltaTime) = 0;

	// �X�V�������擾
	virtual int GetUpdateOrder() = 0;
};

