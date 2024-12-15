#pragma once

#include "SFML/Graphics.hpp"

class IComponent
{
public:
	// デストラクタ
	virtual ~IComponent() = default;

	// デルタタイムでこのコンポーネントを更新する
	virtual void Update(float deltaTime) = 0;

	// コンポーネント特有の入力処理
	virtual void ProcessInput(const sf::Event* event) = 0;

	// ゲッターとセッター
	virtual int GetUpdateOrder() = 0;
};