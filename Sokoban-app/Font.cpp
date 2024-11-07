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
	// サポートするフォントサイズ
	std::vector<int> fontSizes = {
		8, 9, 10, 11, 12, 14, 16, 18, 20, 22, 24, 26, 28, 30, 32, 34, 36, 38, 40, 42, 44, 46, 48, 52, 56, 60, 64, 68, 72
	};

	// フォントのロード
	if (!mFont->loadFromFile(fileName))
	{
		std::cout << "error : failed loading font." << std::endl;
		return false;
	}
	

	// サイズのリストに追加
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

	// 指定サイズのフォントデータを探す
	if (mFontSize.find(pointSize) != mFontSize.end())
	{
		// sf::Text オブジェクトを設定
		sf::Text sfText;
		sfText.setFont(*mFont.get());			// フォントを設定
		sfText.setString(text);					// 表示する文字列を設定
		sfText.setCharacterSize(pointSize);		// 文字サイズを設定
		sfText.setFillColor(color);	// テキストの色（必要に応じて変更可）

		// テキストのサイズに基づき描画用の RenderTexture を作成
		sf::FloatRect textBounds = sfText.getLocalBounds();
		sf::RenderTexture renderTexture;
		if (!renderTexture.create(static_cast<unsigned int>(textBounds.width), static_cast<unsigned int>(textBounds.height)))
		{
			std::cerr << "Failed creating RenderTexture." << std::endl;
		}

		// RenderTexture にテキストを描画
		renderTexture.clear(sf::Color::Transparent);			// 背景を透明にクリア
		sfText.setPosition(-textBounds.left, -textBounds.top);	// テキストの位置を補正
		renderTexture.draw(sfText);								// テキストを描画
		renderTexture.display();								// 描画結果を確定

		// RenderTextureからsf::Textureを生成
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

	// 指定サイズのフォントデータを探す
	if (mFontSize.find(pointSize) != mFontSize.end())
	{
		// sf::Text オブジェクトを設定
		sf::Text sfText;
		sfText.setFont(*mFont.get());			// フォントを設定
		sfText.setString(text);					// 表示する文字列を設定
		sfText.setCharacterSize(pointSize);		// 文字サイズを設定
		sfText.setFillColor(color);	// テキストの色（必要に応じて変更可）

		// テキストのローカルバウンディングボックスを取得
		sf::FloatRect textBounds = sfText.getLocalBounds();

		// テキストの中央揃えの位置を計算
		sf::Vector2f pos{ (boundingBox.x - textBounds.width) / 2.0f - textBounds.left, (boundingBox.y - textBounds.height) / 2.0f - textBounds.top };

		sf::RenderTexture renderTexture;
		if (!renderTexture.create(boundingBox.x, boundingBox.y)) {
			std::cerr << "RenderTextureの作成に失敗しました。" << std::endl;
		}

		// RenderTexture にテキストを描画
		renderTexture.clear(sf::Color::Transparent);			// 背景を透明にクリア
		sfText.setPosition(pos.x, pos.y);	// テキストの位置を補正
		renderTexture.draw(sfText);								// テキストを描画
		renderTexture.display();								// 描画結果を確定

		// RenderTextureからsf::Textureを生成
		texture = new sf::Texture(renderTexture.getTexture());
	}
	else
	{
		std::cout << "error : This font size is not supported." << std::endl;
	}

	return texture;
}