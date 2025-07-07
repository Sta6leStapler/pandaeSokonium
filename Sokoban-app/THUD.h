#pragma once

#include "SFML/Graphics.hpp"
#include "TGUI/TGUI.hpp"
#include "TGUI/Backend/SFML-Graphics.hpp"

#include "IUIScreen.h"

class THUD : public IUIScreen
{
public:
	THUD(class Game* game, sf::RenderWindow* window);
	~THUD();

	// テキストウィンドウに表示する内容のタグ情報
	enum TextIndex
	{
		EMoveCount,
		ETime,
		EBoardState,
		EMovableBaggages,
		EGoaledBaggages,
		EDeadlockedBaggages,
		ESystemAction
	};

	// ボタンやテキストウィンドウのサイズや位置を指定する定数
	const sf::Vector2u Small_Button_Size{ 150, 40 };
	const sf::Vector2u Medium_Button_Size{ 200, 50 };
	const sf::Vector2u Large_Button_Size{ 300, 70 };

	const sf::Vector2u Button_Initial_Pos_Left_Top{ 20, 20 };

	const sf::Vector2u Button_Mergin{ 20, 20 };

	const int ListBox_Item_Height = 32;

	const float Icon_Image_Scale = 0.7f;

	// インターフェースからオーバーライドした関数
	void Update(float deltaTime) override;
	void Draw(sf::RenderWindow* window) override;
	void ProcessInput(const sf::Event* event, const sf::Vector2i& mousePos) override;

	// 状態をクロージングにする
	void Close() { mState = UIState::EClosing; }

	// DPIスケールの変更があった場合にボタンサイズを調整する
	void SetDPIScale(const float dpi_scale);

	// 状態を取得
	UIState GetState() override { return mState; }

private:
	class Game* mGame;

	// tgui::Guiクラス
	std::unique_ptr<tgui::Gui> mGui;

	// UIのテーマ
	std::unique_ptr<tgui::Theme> mTheme;

	// テキストUI用の変数
	tgui::ListBox::Ptr mListBox;
	std::unordered_map<TextIndex, std::string> mTextInfo;

	// 各種制御用変数
	sf::Vector2i mButtonSize;
	sf::Vector2i mButtonInitialPos;
	sf::Vector2i mButtonMergin;
	sf::Vector2i mListBoxSize;
	sf::Vector2i mListBoxPos;
	unsigned int mListBoxItemHeight;
	float mIconImageScale;

	// 状態
	UIState mState;
};

