#include "Button.h"

Button::Button(const std::string& name, class Font* font, std::function<void()> onClick, const sf::Vector2i& pos, const sf::Vector2i& dims)
	: mName(name)
	, mFont(font)
	, mPosition(pos)
	, mDimensions(dims)
{
	
}

Button::~Button()
{
	
}

void Button::SetName(const std::string& name)
{
	mNameTex = mFont->RenderText(name);
}

bool Button::ContainsPoint(const sf::Vector2i& pt) const
{
	bool no = pt.x < (mPosition.x - mDimensions.x / 2.0f) ||
		pt.x >(mPosition.x + mDimensions.x / 2.0f) ||
		pt.y < (mPosition.y - mDimensions.y / 2.0f) ||
		pt.y >(mPosition.y + mDimensions.y / 2.0f);

	return !no;
}

void Button::OnClick()
{
	if (mOnClick)
	{
		mOnClick();
	}
}