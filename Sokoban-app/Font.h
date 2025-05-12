#pragma once

#include <string>
#include <unordered_set>
#include "SFML/Graphics.hpp"

class Font
{
public:
	Font();
	~Font();

	// ファイルのロード/アンロード
	bool Load(const std::string& fileName);
	void Unload();

	// 文字列をテクスチャに描画
	class sf::Texture* RenderText(const std::string& text, const sf::Color& color = sf::Color::White, int pointSize = 30);
	class sf::Texture* RenderTextOnCenter(const std::string& text, const sf::Vector2f& boundingBox, const sf::Color& color = sf::Color::White, int pointSize = 30);

	// ゲッターセッター
	sf::Font* GetFont() const { return mFont.get(); }

private:
	// フォントデータ
	std::unique_ptr<sf::Font> mFont;
	// ポイントサイズ
	std::unordered_set<int> mFontSize;
};

