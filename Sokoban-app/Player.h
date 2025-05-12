#pragma once

#include "SFML/Graphics.hpp"
#include "IActor.h"

#include <unordered_map>
#include <cmath>

class Player : public IActor
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

	// アクターがアクティブ状態のとき更新処理を行う関数
	void Update(float deltaTime) override;

	// アクターが持つ全てのコンポーネントを更新
	void UpdateComponents(float deltaTime) override;

	// Gameクラスから入力処理を呼び出す
	void ProcessInput(const sf::Event* event) override;

	// アクターが持つすべてのコンポーネントの入力処理を行う
	void ProcessInputComponents(const sf::Event* event) override;

	// コンポーネントの追加と削除
	void AddComponent(class IComponent* component) override;
	void RemoveComponent(class IComponent* component) override;

	// アクター共通のゲッターとセッター
	class Game* GetGame() override { return mGame; }
	IActor::ActorState GetState() override { return mState; }
	void SetState(const ActorState state) override { this->mState = state; }
	sf::Vector2f GetPosition() override { return mPosition; }
	sf::Vector2f GetScale() override { return mScale; }
	float GetRotation() override { return mRotation; }

	// 以下本アクター特有のメンバ関数
	// プレイヤーの位置を更新する
	void Reload();

	// ゲッターとセッター
	Direction GetDirection() const { return mDirection; }
	void SetDirection(const Direction& direction) { mDirection = direction; }

	sf::Vector2f GetForward() const { return sf::Vector2f(std::cos(mRotation), -std::sin(mRotation)); }

	sf::Vector2i GetBoardCoordinate() const { return this->mBoardCoordinate; }
	void SetBoardCoordinate(const sf::Vector2i boardCoordinate);

private:
	// アクターの状態
	IActor::ActorState mState;

	// 変換
	sf::Vector2f mPosition;
	sf::Vector2f mScale;
	float mRotation;

	std::vector<IComponent*> mComponents;
	class SpriteComponent* mSpriteComponent;

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