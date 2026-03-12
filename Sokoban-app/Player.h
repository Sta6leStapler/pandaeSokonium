#pragma once

#include "SFML/Graphics.hpp"
#include "IActor.h"
#include "Pathfinder.h"

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

	// プレイヤーの内部状態
	enum class HighlightState
	{
		Idle,          // 通常待機
		Highlighting,  // 移動可能マスをハイライト中
		MovingOnPath   // 計算された経路上を移動中
	};

	Player(class Game* game);
	virtual ~Player();

	// アクターがアクティブ状態のとき更新処理を行う関数
	void Update(float deltaTime) override;

	// アクターが持つ全てのコンポーネントを更新
	void UpdateComponents(float deltaTime) override;

	// Gameクラスから入力処理を呼び出す
	void ProcessInput(const sf::Event* event, const std::map<sf::Keyboard::Key, float>& key_held_duration, const std::map<sf::Keyboard::Key, float>& auto_repeat_timer) override;

	// アクターが持つすべてのコンポーネントの入力処理を行う
	void ProcessInputComponents(const sf::Event* event, const std::map<sf::Keyboard::Key, float>& key_held_duration, const std::map<sf::Keyboard::Key, float>& auto_repeat_timer) override;

	// コンポーネントの追加と削除
	void AddComponent(class IComponent* component) override;
	void RemoveComponent(class IComponent* component) override;

	// アクター共通のゲッターとセッター
	class Game* GetGame() override { return mGame; }
	IActor::ActorState GetState() override { return mState; }
	void SetState(const ActorState state) override { this->mState = state; }
	sf::Vector2f GetPosition() override { return mPosition; }
	void SetPosition(const sf::Vector2f& pos) override { this->mPosition = pos; }
	sf::Vector2f GetScale() override { return mScale; }
	float GetRotation() override { return mRotation; }

	// 以下本アクター特有のメンバ関数
	// プレイヤーの位置を更新する
	void Reload();

	// プレイヤーの内部状態を更新する関数
	void SetIdleState() { mCurrentHighlightState = HighlightState::Idle; }
	void SetHighlightingState() { mCurrentHighlightState = HighlightState::Highlighting; }

	// プレイヤーの移動ルートを受け取る関数
	// 移動に支障がなければその通りに移動する
	void InputMovePath(const std::vector<sf::Vector2i>& path);

	// ゲッターとセッター
	Direction GetDirection() const { return mDirection; }
	void SetDirection(const Direction& direction) { mDirection = direction; }

	sf::Vector2f GetForward() const { return sf::Vector2f(std::cos(mRotation), -std::sin(mRotation)); }

	sf::Vector2i GetBoardCoordinate() const { return this->mBoardCoordinate; }
	void SetBoardCoordinate(const sf::Vector2i boardCoordinate);

	class MoveAnimationComponent* GetMoveAnimation() const { return mMoveAnimation; }

	sf::Texture* GetTextureForDirection(const Direction direction) const { return mTextures.at(direction); }

private:
	// このアクター専用のヘルパー関数
	// アイドル状態でプレイヤーがクリックされた場合
	void HandleInputIdle(const sf::Vector2i& clickedTile);
	// 移動可能なマスをハイライト中にプレイヤーがクリックされた場合
	void HandleInputHighlighting(const sf::Vector2i& clickedTile);

	// アクターの状態
	IActor::ActorState mState;

	// 変換
	sf::Vector2f mPosition;
	sf::Vector2f mScale;
	float mRotation;

	std::vector<IComponent*> mComponents;
	class SpriteComponent* mSpriteComponent;
	class MoveAnimationComponent* mMoveAnimation;

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

	// ハイライト状態管理用のメンバ変数を追加
	HighlightState mCurrentHighlightState;

	// 自動移動用の経路を保持する
	std::vector<sf::Vector2i> mMovementPath;

	// 経路移動の各ステップ間のタイマー
	float mPathMoveTimer;
	const float PATH_MOVE_INTERVAL = 0.04f; // 1マス移動するのにかかる時間(秒)

	// 先行入力の管理
	Direction mQueuedDirection;	// 次の向き
	float mQueuedRotation;		// 次の回転角
	bool mHasQueuedInput;		// 先行入力の有無

	// 移動入力の検知
	bool mDetection;
};