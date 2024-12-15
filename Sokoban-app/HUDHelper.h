#pragma once

#include <vector>
#include <unordered_map>
#include <set>
#include <string>
#include <memory>
#include <random>
#include "SFML/Graphics.hpp"

using Board = std::vector<std::string>;
struct BoardStates
{
	std::vector<sf::Vector2i> mMovableArea;	// プレイヤーの移動可能な座標のリスト
	std::unordered_map<int, sf::Vector2i> mDeadlockedBaggages;	// 詰み状態が確定している荷物
	std::unordered_map<int, sf::Vector2i> mGoaledBaggages;	// ゴール状態の荷物
	std::unordered_map<int, sf::Vector2i> mNonGoaledBaggages;	// ゴール状態でない荷物
};

class HUDHelper
{
public:
	HUDHelper(class Game* game);
	~HUDHelper();

	// 盤面の更新処理
	void Update();

	// 詰みの荷物の配置がないことを確認する関数
	// 1つでも詰み状態の荷物があればfalseを返す
	// 
	bool isArrangementValid();

	// 詰みの荷物の座標のリストを返す関数
	std::unordered_map<int, sf::Vector2i> GetDeadlockedBaggages() const { return mBoardStates.mDeadlockedBaggages; }

	// プレイヤーが移動可能な座標のリストを返す関数
	std::vector<sf::Vector2i> GetMovableArea() const { return mBoardStates.mMovableArea; }

	// ゴール状態の荷物を返す関数
	std::unordered_map<int, sf::Vector2i> GetGoaledBaggages() const { return mBoardStates.mGoaledBaggages; }

	// ゴール状態でない荷物を返す関数
	std::unordered_map<int, sf::Vector2i> GetNonGoaledBaggages() const { return mBoardStates.mNonGoaledBaggages; }

	// 指定された荷物が詰みかどうかを返す関数
	bool isDeadlockedBaggage(const sf::Vector2i& pos);

	// 押せる荷物の候補を求める関数
	std::vector<sf::Vector2i> GetCandidates();

private:
	enum Direction
	{
		ENorth,
		EEast,
		ESouth,
		EWest
	};

	// ランダムなパラメータを取得する関数
	Direction RollDirection();
	sf::Vector2i RollCoordinate();
	sf::Vector2i RollCoordinate(const int& xMin, const int& xMax, const int& yMin, const int& yMax);
	int RollCoordinateX();
	int RollCoordinateX(const int& xMin, const int& xMax);
	int RollCoordinateY();
	int RollCoordinateY(const int& yMin, const int& yMax);
	template <typename T>
	T GetRandomElement(const std::vector<T>& vec);
	template<typename T, typename U>
	std::pair<T, U> GetRandomElement(const std::unordered_map<T, U>& map);
	std::vector<sf::Vector2i> GetRandomDirections();

	// パブリックなメンバ関数内で使うヘルパー関数
	// 方向をインデックスに変換する関数
	int GetDirectionIndex(const sf::Vector2i& dir)
	{
		if (dir == sf::Vector2i{ 1, 0 })
		{
			return 0;
		}
		else if (dir == sf::Vector2i{ 0, 1 })
		{
			return 1;
		}
		else if (dir == sf::Vector2i{ -1, 0 })
		{
			return 2;
		}
		else if (dir == sf::Vector2i{ 0, -1 })
		{
			return 3;
		}

		return -1;
	}
	// 指定された荷物が詰み状態かどうか
	bool isFreezeBaggage(const sf::Vector2i& baggagePos);
	// 2つの座標が同じ床エリアにあるかを判定する関数
	bool isMovableArea(const sf::Vector2i& pos1, const sf::Vector2i& pos2);
	bool isMovableArea(const sf::Vector2i& pos1, const sf::Vector2i& pos2, const Board& board);
	// プレイヤーの移動可能な範囲を取得する関数
	std::vector<sf::Vector2i> GetMovableArea();
	// デッドロック状態の荷物を更新する関数
	std::unordered_map<int, sf::Vector2i> UpdateFreezeBaggagesMap();
	// 更新したプレイヤーの座標と荷物の座標から、盤面を再構成
	void UpdateBoardStr();

	// 以下メンバ変数
	// Gameクラス
	class Game* mGame;
	
	// 探索関連の変数
	const std::vector<sf::Vector2i> directions =
	{
		sf::Vector2i(1, 0),   // 右
		sf::Vector2i(0, 1),  // 下
		sf::Vector2i(-1, 0),   // 左
		sf::Vector2i(0, -1)   // 上
	};

	// 乱数関連の変数
	std::random_device rd;
	std::mt19937 mt;

	// 盤面構造の変数
	BoardStates mBoardStates;
	std::map<std::pair<std::pair<int, int>, std::set<std::pair<int, int>>>, BoardStates> mHistory;

	// 盤面関連の変数
	sf::Vector2i mSize;
	Board mBoardStr;	// 盤面の情報を表す
	Board mInitialBoardStr;	//初期盤面
	Board mBoardMapStr;	// 盤面の地形 (プレイヤーと荷物を排除した盤面)
	int mBaggageNum;	// 荷物の数
	sf::Vector2i mPlayerPos;	// プレイヤーの座標
	sf::Vector2i mInitialPlayerPos;	//プレイヤーの初期位置
	std::vector<sf::Vector2i> mGoals;	// ゴール座標
	// TODO インデックスを付ける理由
	std::unordered_map<int, sf::Vector2i> mBaggagesPos;	// 荷物の置いてある座標のリスト
	std::unordered_map<int, sf::Vector2i> mGoaledBaggages;
	std::unordered_map<int, sf::Vector2i> mNonGoaledBaggages;
	std::unordered_map<int, sf::Vector2i> mInitialBaggagesPos;	// 荷物の初期位置
};