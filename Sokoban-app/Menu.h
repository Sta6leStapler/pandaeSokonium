#pragma once

#include "SFML/Graphics.hpp"
#include "State.h"
#include <unordered_map>
#include <cmath>

class Menu
{
public:
	// プレイヤーの向きを表現する列挙型
	enum Direction
	{
		ENorth,
		EEast,
		EWest,
		ESouth
	};

	Menu(class Game* game);
	virtual ~Menu();

	// Gameクラスから更新処理を呼び出す
	void Update(float deltaTime);

	// アクターが持つ全てのコンポーネントを更新
	void UpdateComponents(float deltaTime);

	// Gameクラスから入力処理を呼び出す
	void ProcessInput(const sf::Event::KeyEvent* keyState);

	// コンポーネントの追加と削除
	void AddComponent(class MenuComponent* component);
	void RemoveComponent(class MenuComponent* component);

	// プレイヤーの位置を更新する
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
	class State mState;

	// 変換
	sf::Vector2f mPosition;
	sf::Vector2f mScale;
	float mRotation;

	class MenuComponent* mComponent;

	std::string mBoardName;

	// コンポーネントに使うテクスチャ
	std::unordered_map<Direction, sf::Texture*> mTextures;

	class Game* mGame;

	// 以下はアクター特有のメンバ変数

	// 直前のフレームのキー入力
	sf::Event::KeyEvent prevKeys;

	// 移動入力のクールダウン
	float mMoveCooldown;

	// 移動入力の検知
	bool mDetection;
};