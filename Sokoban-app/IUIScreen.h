#pragma once

#include "SFML/Window/Event.hpp"

// UI��ʋ��ʂ̃C���^�[�t�F�[�X�N���X
class IUIScreen {
public:
    virtual ~IUIScreen() = default;

    // UI��ʂ̍X�V�����i���Ԍo�߂ɉ���������j
    virtual void Update(float deltaTime) = 0;

    // UI��ʂ̕`�揈��
    virtual void Draw() = 0;

    // ���[�U�[���͂̏���
    virtual void ProcessInput(const sf::Event::KeyEvent* keyState) = 0;
};