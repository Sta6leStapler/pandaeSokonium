#pragma once

#include "SFML/Graphics.hpp"
#include "IActor.h"
#include "Baggage.h"

#include <unordered_map>
#include <cmath>

class GameBoard : IActor
{
public:
	// 向きを表現する列挙型
	enum Direction
	{
		ENorth,
		EEast,
		EWest,
		ESouth
	};

	GameBoard(class Game* game);
	virtual ~GameBoard();

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
	sf::Vector2f GetScale() override { return mScale; }
	float GetRotation() override { return mRotation; }

	// 以下本アクター特有のメンバ関数
	// 盤面を更新する
	void Reload();
	// ハイライト中のタイルのリストを取得
	std::vector<sf::Vector2i> GetMoveHighlightedTiles() const { return mMoveHighlightedTiles; }
	std::vector<sf::Vector2i> GetPushHighlightedTiles() const { return mPushHighlightedTiles; }
	// Gameクラスから呼ばれ、プレイヤーが移動可能なタイルのハイライトを設定する
	void SetMoveHighlightedTiles(const std::vector<sf::Vector2i>& tiles);
	// プレイヤーが移動可能なタイルのハイライトをすべてクリアする
	void ClearMoveHighlights();
	// Gameクラスから呼ばれ、荷物を運搬可能なタイルのハイライトを設定する
	void SetPushHighlightedTiles(const std::vector<sf::Vector2i>& tiles, class Baggage* baggage);
	// 荷物が運搬可能なタイルのハイライトをすべてクリアする
	void ClearPushHighlights();
	// Gameクラスから呼ばれ、荷物を運搬可能な方向を設定する
	void SetPushDirections(const std::vector<int>& indexes, class Baggage* baggage);
	// 荷物が運搬可能な方向の表示をすべてクリアする
	void ClearPushDirections();

	// ゲッターとセッター
	std::string GetBoardName() const { return this->mBoardName; }
	sf::Vector2f GetForward() const { return sf::Vector2f(std::cos(mRotation), -std::sin(mRotation)); }
	sf::Texture* GetBoardTexture() const;

private:
	// アクターの状態
	IActor::ActorState mState;

	// 変換
	sf::Vector2f mPosition;
	sf::Vector2f mScale;
	float mRotation;

	std::vector<class IComponent*> mComponents;
	class SpriteComponent* mBoardSpriteComponent;
	class SpriteComponent* mHighlightSpriteComponent;

	std::string mBoardName;

	// コンポーネントに使うテクスチャ
	std::unordered_map<std::string, sf::Texture*> mTextures;

	class Game* mGame;

	// アクター特有のメンバ変数
	// ハイライト中のタイルのリスト
	std::vector<sf::Vector2i> mMoveHighlightedTiles;
	std::vector<sf::Vector2i> mPushHighlightedTiles;
	// ハイライト用テクスチャ
	sf::Texture* mMoveHighlightTexture;
	sf::Texture* mPushHighlightTexture;
	// 矢印のテクスチャ
	std::map<Direction, sf::Texture*> mIdleArrowTextures;
	std::map<Direction, sf::Texture*> mOnCursorArrowTextures;

	Baggage* mHighlightingBaggage;
};