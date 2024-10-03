#pragma once

#include <string>
#include <functional>
#include "SFML/Graphics.hpp"
#include "Font.h"

class Button
{
public:
	// �R���X�g���N�^�́A�{�^�����̂ƁA�t�H���g�ƁA�R�[���o�b�N�֐��ƁA
	// �{�^���̈ʒu����ѐ��@���󂯎��
	Button(const std::string& name, class Font* font, std::function<void()> onClick, const sf::Vector2i& pos, const sf::Vector2i& dims);
	~Button();

	// �\���e�L�X�g��ݒ肵�A�e�N�X�`���𐶐�����
	void SetName(const std::string& name);

	// ���W���{�^���͈͓̔��Ȃ�true��Ԃ�
	bool ContainsPoint(const sf::Vector2i& pt) const;

	// �{�^���������ꂽ�Ƃ��ɌĂяo�����
	void OnClick();

	// �Q�b�^�[�Z�b�^�[


private:
	std::function<void()> mOnClick;
	std::string mName;
	class sf::Texture* mNameTex;
	class Font* mFont;
	sf::Vector2i mPosition;
	sf::Vector2i mDimensions;
	bool mHighlighted;
};