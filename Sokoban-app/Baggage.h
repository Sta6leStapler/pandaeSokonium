#pragma once
#include "SFML/Graphics.hpp"
#include "State.h"
#include <unordered_map>
#include <cmath>

class Baggage
{
public:
	enum BState
	{
		OnGoal,
		OnFloor,
		Deadlock
	};

	Baggage(class Game* game, sf::Vector2i bCoordinate);
	virtual ~Baggage();

	// Gameクラスから更新処理を呼び出す
	void Update(float deltaTime);

	// アクターが持つ全てのコンポーネントを更新
	void UpdateComponents(float deltaTime);

	// Gameクラスから入力処理を呼び出す
	void ProcessInput(const sf::Event::KeyEvent* keyState);

	// コンポーネントの追加と削除
	void AddComponent(class BaggageComponent* component);
	void RemoveComponent(class BaggageComponent* component);

	// ゲッターとセッター
	State GetState() const { return mState; }
	void SetState(const State state) { mState = state; }

	const sf::Vector2f& GetPosition() const { return mPosition; }
	void SetPosition(const sf::Vector2f& pos) { mPosition = pos; }
	sf::Vector2f GetScale() const { return mScale; }
	void SetScale(const sf::Vector2f scale) { mScale = scale; }
	float GetRotation() const { return mRotation; }
	void SetRotation(const float rotation) { mRotation = rotation; }

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

	class BaggageComponent* mComponent;

	std::string mBoardName;

	// コンポーネントに使うテクスチャ
	std::unordered_map<BState, sf::Texture*> mTextures;

	class Game* mGame;

	// 以下はアクター特有のメンバ変数
	// 荷物の盤面上の座標
	sf::Vector2i mBoardCoordinate;

	// 荷物がゴール上にあるかどうか
	BState bState;
};