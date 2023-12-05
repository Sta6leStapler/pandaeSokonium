#pragma once

#include "SFML/Graphics.hpp"
#include "State.h"
#include <unordered_map>
#include <cmath>

class Player
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

	Player(class Game* game);
	virtual ~Player();

	// Gameクラスから更新処理を呼び出す
	void Update(float deltaTime);

	// アクターが持つ全てのコンポーネントを更新
	void UpdateComponents(float deltaTime);

	// Gameクラスから入力処理を呼び出す
	void ProcessInput(const sf::Event::KeyEvent* keyState);

	// コンポーネントの追加と削除
	void AddComponent(class PlayerComponent* component);
	void RemoveComponent(class PlayerComponent* component);

	// プレイヤーの位置を更新する
	void Reload();

	// ゲッターとセッター
	State GetState() const { return mState; }
	void SetState(const State state) { mState = state; }

	Direction GetDirection() const { return mDirection; }
	void SetDirection(const Direction& direction) { mDirection = direction; }

	const sf::Vector2f& GetPosition() const { return mPosition; }
	void SetPosition(const sf::Vector2f& pos) { mPosition = pos; }
	sf::Vector2f GetScale() const { return mScale; }
	void SetScale(sf::Vector2f scale) { mScale = scale; }
	float GetRotation() const { return mRotation; }
	void SetRotation(float rotation) { mRotation = rotation; }

	sf::Vector2f GetForward() const { return sf::Vector2f(std::cos(mRotation), -std::sin(mRotation)); }

	class Game* GetGame() { return mGame; }

	sf::Vector2i GetBoardCoordinate() const { return mBoardCoordinate; }
	void SetBoardCoordinate(const sf::Vector2i boardCoordinate);

private:
	// アクターの状態
	class State mState;

	// 変換
	sf::Vector2f mPosition;
	sf::Vector2f mScale;
	float mRotation;

	class PlayerComponent* mComponent;

	std::string mBoardName;

	// コンポーネントに使うテクスチャ
	std::unordered_map<Direction, sf::Texture*> mTextures;

	class Game* mGame;

	// 以下はアクター特有のメンバ変数
	// プレイヤーの盤面上の座標
	sf::Vector2i mBoardCoordinate;
	// プレイヤーの向き
	Player::Direction mDirection, prevDirection;

	// 直前のフレームのキー入力
	sf::Event::KeyEvent prevKeys;

	// 移動入力のクールダウン
	float mMoveCooldown;

	// 移動入力の検知
	bool mDetection;
};