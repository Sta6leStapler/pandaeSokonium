#pragma once

#include "SFML/Graphics.hpp"

class IActor
{
public:
	virtual ~IActor() = default;

	// Gameクラスから更新処理を呼び出す
	virtual void Update(float deltaTime) = 0;

	// アクターが持つ全てのコンポーネントを更新
	virtual void UpdateComponents(float deltaTime) = 0;

	// Gameクラスから入力処理を呼び出す
	virtual void ProcessInput(const sf::Event* event) = 0;

	// アクターが持つ全てのコンポーネントの入力処理を行う
	virtual void ProcessInputComponents(const sf::Event* event) = 0;

	// コンポーネントの追加と削除
	virtual void AddComponent(class IComponent* component) = 0;
	virtual void RemoveComponent(class IComponent* component) = 0;

	// 状態がアクティブかクロージングか待機中か管理
	enum class ActorState { EActive, EClosing, EPending };

	// ゲッターとセッター
	virtual class Game* GetGame() = 0;
	virtual ActorState GetState() = 0;
	virtual void SetState(const ActorState state) = 0;
	virtual sf::Vector2f GetPosition() = 0;
	virtual sf::Vector2f GetScale() = 0;
	virtual float GetRotation() = 0;
};