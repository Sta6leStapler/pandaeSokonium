#pragma once

#include "SFML/Window/Event.hpp"
#include "IComponent.h"

class IActor
{
	// デストラクタ
	virtual ~IActor() = default;

	// ゲームクラスの機能を使う時に呼び出す関数
	virtual class Game* GetGame() = 0;

	// Gameクラスから更新処理を呼び出す
	virtual void Update(float deltaTime) = 0;

	// アクターが持つ全てのコンポーネントを更新
	virtual void UpdateComponents(float deltaTime) = 0;

	// Gameクラスから入力処理を呼び出す
	virtual void ProcessInput(const sf::Event::KeyEvent* keyState) = 0;

	// コンポーネントの追加と削除
	virtual void AddComponent(class IComponent* component) = 0;
	virtual void RemoveComponent(class IComponent* component) = 0;

};

