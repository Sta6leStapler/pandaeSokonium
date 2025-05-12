#pragma once

#include <string>
#include <functional>
#include "SFML/Graphics.hpp"

class Button
{
public:
	// �R���X�g���N�^�́A�{�^�����̂ƁA�t�H���g�ƁA�R�[���o�b�N�֐��ƁA
	// �{�^���̈ʒu����ѐ��@���󂯎��
	Button(const std::string& name, class Font* font, std::function<void()> onClick, const sf::Vector2f& pos, const sf::Vector2f& dims);
	~Button();

	// �\���e�L�X�g��ݒ肵�A�e�N�X�`���𐶐�����
	void SetName(const std::string& name, const sf::Color& color = sf::Color::White, int pointSize = 30);
	void SetNameCenter(const std::string& name, const sf::Vector2f& boundingBox, const sf::Color& color = sf::Color::White, int pointSize = 30);

	// ���W���{�^���͈͓̔��Ȃ�true��Ԃ�
	bool ContainsPoint(const sf::Vector2f& pt) const;

	// �{�^���������ꂽ�Ƃ��ɌĂяo�����
	void OnClick();

	// �Q�b�^�[�Z�b�^�[
	sf::Texture* GetNameTex() const { return mNameTex; }
	sf::Vector2f GetPosition() const { return mPosition; }
	bool GetHighlighted() const { return mHighlighted; }
	void SetHighlighted(bool sel) { mHighlighted = sel; }

private:
	std::function<void()> mOnClick;
	std::string mName;
	class sf::Texture* mNameTex;
	class Font* mFont;
	sf::Vector2f mPosition;
	sf::Vector2f mDimensions;	// �{�^���̃T�C�Y
	bool mHighlighted;
};