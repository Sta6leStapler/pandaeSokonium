#pragma once

#include <string>
#include <functional>
#include "SFML/Graphics.hpp"

class Button
{
public:
	// コンストラクタは、ボタン名称と、フォントと、コールバック関数と、
	// ボタンの位置および寸法を受け取る
	Button(const std::string& name, class Font* font, std::function<void()> onClick, const sf::Vector2f& pos, const sf::Vector2f& dims);
	~Button();

	// 表示テキストを設定し、テクスチャを生成する
	void SetName(const std::string& name, const sf::Color& color = sf::Color::White, int pointSize = 30);
	void SetNameCenter(const std::string& name, const sf::Vector2f& boundingBox, const sf::Color& color = sf::Color::White, int pointSize = 30);

	// 座標がボタンの範囲内ならtrueを返す
	bool ContainsPoint(const sf::Vector2f& pt) const;

	// ボタンが押されたときに呼び出される
	void OnClick();

	// ゲッターセッター
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
	sf::Vector2f mDimensions;	// ボタンのサイズ
	bool mHighlighted;
};