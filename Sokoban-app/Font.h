#pragma once

#include <string>
#include <unordered_set>
#include "SFML/Graphics.hpp"

class Font
{
public:
	Font();
	~Font();

	// �t�@�C���̃��[�h/�A�����[�h
	bool Load(const std::string& fileName);
	void Unload();

	// ��������e�N�X�`���ɕ`��
	class sf::Texture* RenderText(const std::string& text, const sf::Color& color = sf::Color::White, int pointSize = 30);

private:
	// �t�H���g�f�[�^
	std::unique_ptr<sf::Font> mFont;
	// �|�C���g�T�C�Y
	std::unordered_set<int> mFontSize;
};

