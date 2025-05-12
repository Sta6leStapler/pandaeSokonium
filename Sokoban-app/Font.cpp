#include "Font.h"
#include <iostream>

Font::Font()
	: mFont(std::make_unique<sf::Font>())
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

	return true;
}

void Font::Unload()
{
	mFont.reset();
}

sf::Texture* Font::RenderText(const std::string& text, const sf::Color& color /*= sf::Color::White*/, int pointSize /*= 30*/)
{
	sf::Texture* texture = nullptr;

	// �w��T�C�Y�̃t�H���g�f�[�^��T��
	if (mFontSize.find(pointSize) != mFontSize.end())
	{
		// sf::Text �I�u�W�F�N�g��ݒ�
		sf::Text sfText;
		sfText.setFont(*mFont.get());			// �t�H���g��ݒ�
		sfText.setString(text);					// �\�����镶�����ݒ�
		sfText.setCharacterSize(pointSize);		// �����T�C�Y��ݒ�
		sfText.setFillColor(color);	// �e�L�X�g�̐F�i�K�v�ɉ����ĕύX�j

		// �e�L�X�g�̃T�C�Y�Ɋ�Â��`��p�� RenderTexture ���쐬
		sf::FloatRect textBounds = sfText.getLocalBounds();
		sf::RenderTexture renderTexture;
		if (!renderTexture.create(static_cast<unsigned int>(textBounds.width), static_cast<unsigned int>(textBounds.height)))
		{
			std::cerr << "Failed creating RenderTexture." << std::endl;
		}

		// RenderTexture �Ƀe�L�X�g��`��
		renderTexture.clear(sf::Color::Transparent);			// �w�i�𓧖��ɃN���A
		sfText.setPosition(-textBounds.left, -textBounds.top);	// �e�L�X�g�̈ʒu��␳
		renderTexture.draw(sfText);								// �e�L�X�g��`��
		renderTexture.display();								// �`�挋�ʂ��m��

		// RenderTexture����sf::Texture�𐶐�
		texture = new sf::Texture(renderTexture.getTexture());
	}
	else
	{
		std::cout << "error : This font size is not supported." << std::endl;
	}

	return texture;
}

sf::Texture* Font::RenderTextOnCenter(const std::string& text, const sf::Vector2f& boundingBox, const sf::Color& color /*= sf::Color::White*/, int pointSize /*= 30*/)
{
	sf::Texture* texture = nullptr;

	// �w��T�C�Y�̃t�H���g�f�[�^��T��
	if (mFontSize.find(pointSize) != mFontSize.end())
	{
		// sf::Text �I�u�W�F�N�g��ݒ�
		sf::Text sfText;
		sfText.setFont(*mFont.get());			// �t�H���g��ݒ�
		sfText.setString(text);					// �\�����镶�����ݒ�
		sfText.setCharacterSize(pointSize);		// �����T�C�Y��ݒ�
		sfText.setFillColor(color);	// �e�L�X�g�̐F�i�K�v�ɉ����ĕύX�j

		// �e�L�X�g�̃��[�J���o�E���f�B���O�{�b�N�X���擾
		sf::FloatRect textBounds = sfText.getLocalBounds();

		// �e�L�X�g�̒��������̈ʒu���v�Z
		sf::Vector2f pos{ (boundingBox.x - textBounds.width) / 2.0f - textBounds.left, (boundingBox.y - textBounds.height) / 2.0f - textBounds.top };

		sf::RenderTexture renderTexture;
		if (!renderTexture.create(boundingBox.x, boundingBox.y)) {
			std::cerr << "RenderTexture�̍쐬�Ɏ��s���܂����B" << std::endl;
		}

		// RenderTexture �Ƀe�L�X�g��`��
		renderTexture.clear(sf::Color::Transparent);			// �w�i�𓧖��ɃN���A
		sfText.setPosition(pos.x, pos.y);	// �e�L�X�g�̈ʒu��␳
		renderTexture.draw(sfText);								// �e�L�X�g��`��
		renderTexture.display();								// �`�挋�ʂ��m��

		// RenderTexture����sf::Texture�𐶐�
		texture = new sf::Texture(renderTexture.getTexture());
	}
	else
	{
		std::cout << "error : This font size is not supported." << std::endl;
	}

	return texture;
}