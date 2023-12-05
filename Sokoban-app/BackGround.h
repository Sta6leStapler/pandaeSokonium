#pragma once

#include "SFML/Graphics.hpp"
#include "State.h"
#include <cmath>

class BackGround
{
public:
	BackGround(class Game* game);
	virtual ~BackGround();

	// Gameクラスから更新処理を呼び出す
	void Update(float deltaTime);

	// アクターが持つ全てのコンポーネントを更新
	void UpdateComponents(float deltaTime);

	// Gameクラスから入力処理を呼び出す
	void ProcessInput(const sf::Event::KeyEvent* keyState);

	// コンポーネントの追加と削除
	void AddComponent(class BackGroundComponent* component);
	void RemoveComponent(class BackGroundComponent* component);

	// ゲッターとセッター
	State GetState() const { return mState; }
	void SetState(const State state) { mState = state; }

	const sf::Vector2f& GetPosition() const { return mPosition; }
	void SetPosition(const sf::Vector2f& pos) { mPosition = pos; }
	sf::Vector2f GetScale() const { return mScale; }
	void SetScale(sf::Vector2f scale) { mScale = scale; }
	float GetRotation() const { return mRotation; }
	void SetRotation(float rotation) { mRotation = rotation; }

	sf::Vector2f GetForward() const { return sf::Vector2f(std::cos(mRotation), -std::sin(mRotation)); }

	class Game* GetGame() { return mGame; }

private:
	// アクターの状態
	State mState;

	// 変換
	sf::Vector2f mPosition;
	sf::Vector2f mScale;
	float mRotation;

	std::vector<class BackGroundComponent*> mComponents;

	class Game* mGame;

};