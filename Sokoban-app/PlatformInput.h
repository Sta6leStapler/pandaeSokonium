#pragma once

#include "SFML/System/Vector2.hpp"
#include "SFML/Window/Event.hpp"

#include <algorithm>
#include <cmath>

// プラットフォームごとの入力・ウィンドウサイズ差異を吸収するヘルパー
namespace PlatformInput
{
	// 保存・リスタート等のコマンド修飾キーが押されているか判定する
	// macOSではCmdキー（system）とCtrlキーの両方を受け付ける
	inline bool IsCommandModifierPressed(const sf::Event::KeyEvent& key)
	{
#if defined(__APPLE__)
		return key.system || key.control;
#else
		return key.control;
#endif
	}

	// デスクトップ解像度に応じてアスペクト比を維持した初期ウィンドウサイズを計算する
	// 1600x900が収まる環境では論理サイズをそのまま返す
	inline sf::Vector2u ComputeInitialWindowSize(const sf::Vector2f& logicalSize, unsigned int desktopWidth, unsigned int desktopHeight)
	{
		const unsigned int defaultW = static_cast<unsigned int>(logicalSize.x);
		const unsigned int defaultH = static_cast<unsigned int>(logicalSize.y);
		if (desktopWidth == 0 || desktopHeight == 0)
		{
			return sf::Vector2u{ defaultW, defaultH };
		}

		// タスクバー・Dock・タイトルバーの余白を確保
		const float maxW = static_cast<float>(desktopWidth) * 0.92f;
		const float maxH = static_cast<float>(desktopHeight) * 0.88f;
		const float scale = std::min(1.0f, std::min(maxW / logicalSize.x, maxH / logicalSize.y));
		if (scale >= 1.0f)
		{
			return sf::Vector2u{ defaultW, defaultH };
		}

		const unsigned int scaledW = std::max(640u, static_cast<unsigned int>(std::floor(logicalSize.x * scale)));
		const unsigned int scaledH = std::max(360u, static_cast<unsigned int>(std::floor(logicalSize.y * scale)));
		return sf::Vector2u{ scaledW, scaledH };
	}
}
