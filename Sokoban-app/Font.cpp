#include "Font.h"
#include <iostream>

Font::Font()
{

}

Font::~Font()
{

}

bool Font::Load(const std::string& fileName)
{
	// �T�|�[�g����t�H���g�T�C�Y
	std::vector<int> fontSizes = {
		8, 9, 10, 11, 12, 14, 16, 18, 20, 22, 24, 26, 28, 30, 32, 34, 36, 38, 40, 42, 44, 46, 48, 52, 56, 60, 64, 68, 72
	};

	// �t�H���g�̃��[�h
	mFont = std::make_unique<sf::Font>();
	if (!mFont->loadFromFile(fileName))
	{
		std::cout << "error : failed loading font." << std::endl;
		return false;
	}
	

	// �T�C�Y�̃��X�g�ɒǉ�
	for (const auto& size : fontSizes)
	{
		mFontSize.emplace(size);
	}
}

void Font::Unload()
{
	mFont.reset();
}

sf::Texture* Font::RenderText(const std::string& text, const sf::Color& color = sf::Color::White, int pointSize = 30)
{
	sf::Texture* texture = nullptr;

	// �w��T�C�Y�̃t�H���g�f�[�^��T��
	if (mFontSize.find(pointSize) != mFontSize.end())
	{
		texture = new sf::Texture(mFont->getTexture(pointSize));
	}
	else
	{
		std::cout << "error : This font size is not supported." << std::endl;
	}

	return texture;
}