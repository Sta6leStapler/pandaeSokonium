#pragma once

#include "SFML/Graphics.hpp"
#include "State.h"
#include <unordered_map>
#include <cmath>

class GameBoard
{
public:
	GameBoard(class Game* game);
	virtual ~GameBoard();

	// Gameクラスから更新処理を呼び出す
	void Update(float deltaTime);

	// アクターが持つ全てのコンポーネントを更新
	void UpdateComponents(float deltaTime);

	// Gameクラスから入力処理を呼び出す
	void ProcessInput(const sf::Event::KeyEvent* keyState);

	// コンポーネントの追加と削除
	void AddComponent(class GameBoardComponent* component);
	void RemoveComponent(class GameBoardComponent* component);

	// 盤面を更新する
	void Reload();

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

	class GameBoardComponent* mGameBoardComponent;

	std::string mBoardName;

	// コンポーネントに使うテクスチャ
	std::unordered_map<std::string, sf::Texture*> mTextures;

	class Game* mGame;

};