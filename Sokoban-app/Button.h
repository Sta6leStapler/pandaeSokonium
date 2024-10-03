#pragma once

#include <string>
#include <functional>
#include "SFML/Graphics.hpp"
#include "Font.h"

class Button
{
public:
	// コンストラクタは、ボタン名称と、フォントと、コールバック関数と、
	// ボタンの位置および寸法を受け取る
	Button(const std::string& name, class Font* font, std::function<void()> onClick, const sf::Vector2i& pos, const sf::Vector2i& dims);
	~Button();

	// 表示テキストを設定し、テクスチャを生成する
	void SetName(const std::string& name);

	// 座標がボタンの範囲内ならtrueを返す
	bool ContainsPoint(const sf::Vector2i& pt) const;

	// ボタンが押されたときに呼び出される
	void OnClick();

	// ゲッターセッター


private:
	std::function<void()> mOnClick;
	std::string mName;
	class sf::Texture* mNameTex;
	class Font* mFont;
	sf::Vector2i mPosition;
	sf::Vector2i mDimensions;
	bool mHighlighted;
};