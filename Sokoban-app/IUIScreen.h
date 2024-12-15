#pragma once

#include "SFML/Window/Event.hpp"

// UI��ʋ��ʂ̃C���^�[�t�F�[�X�N���X
class IUIScreen {
public:
    virtual ~IUIScreen() = default;

    // UI��ʂ̍X�V�����i���Ԍo�߂ɉ���������j
    virtual void Update(float deltaTime) = 0;

    // UI��ʂ̕`�揈��
    virtual void Draw(sf::RenderWindow* mWindow) = 0;

    // ���[�U�[���͂̏���
    virtual void ProcessInput(const sf::Event* event, const sf::Vector2i& mousePos) = 0;

    // ��Ԃ��A�N�e�B�u���N���[�W���O���ҋ@�����Ǘ�
    enum UIState { EActive, EClosing, EPending };

    // ��Ԃ��N���[�W���O�ɂ���
    virtual void Close() = 0;

    // ��Ԃ��擾
    virtual UIState GetState() = 0;
};