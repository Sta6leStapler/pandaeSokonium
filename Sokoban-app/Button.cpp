#include "Button.h"

#include "Font.h"

Button::Button(const std::string& name, Font* font, std::function<void()> onClick, const sf::Vector2f& pos, const sf::Vector2f& dims)
	: mOnClick(onClick)
	, mName(name)
	, mFont(font)
	, mPosition(pos)
	, mDimensions(dims)
	, mHighlighted(false)
	, mNameTex(nullptr)
{
	
}

Button::~Button()
{
	delete mNameTex;
	// フォントはUIクラスから指定されているので、ここでは消さない
	mFont = nullptr;
}

void Button::SetName(const std::string& name, const sf::Color& color, int pointSize)
{
	mNameTex = mFont->RenderText(name, color, pointSize);
}

void Button::SetNameCenter(const std::string& name, const sf::Vector2f& boundingBox, const sf::Color& color, int pointSize)
{
	mNameTex = mFont->RenderTextOnCenter(name, boundingBox, color, pointSize);
}

bool Button::ContainsPoint(const sf::Vector2f& pt) const
{
	bool no = pt.x < (mPosition.x) ||
		pt.x > (mPosition.x + mDimensions.x) ||
		pt.y < (mPosition.y) ||
		pt.y > (mPosition.y + mDimensions.y);

	return !no;
}

void Button::OnClick()
{
	if (mOnClick)
	{
		mOnClick();
	}
}