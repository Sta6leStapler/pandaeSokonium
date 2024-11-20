#pragma once

#include "SFML/Window/Event.hpp"

class IComponent
{
	// デストラクタ
	virtual ~IComponent() = default;

	// デルタタイムでこのコンポーネントを更新する
	virtual void Update(float deltaTime) = 0;

	// 更新順序を取得
	virtual int GetUpdateOrder() = 0;
};

