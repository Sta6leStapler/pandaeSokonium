#pragma once

#include "SFML/Graphics.hpp"
#include <string>
#include <vector>
#include <random>
#include <unordered_map>
#include <map>

#include <iostream>

using Board = std::vector<std::string>;
using Route = std::vector<sf::Vector2i>;
using CandidateItems = std::unordered_map<int, std::vector<sf::Vector2i>>;
using CandidateRoutes = std::unordered_map<int, std::vector<Route>>;

class MySolution
{
public:
	MySolution(const sf::Vector2i& size, const int& baggageNum, const int& buildNum, const int& runsNum);

	// ゲッターセッター
	Board GetBoard() { return mBoardStr; }
	int GetBaggageNum() { return mBaggageNum; }
	std::vector<std::pair<sf::Vector2i, sf::Vector2i>> GetInitToGoalList() { return mInitToGoalList; }
	int GetSolveUpper() { return mSolveUpper; }

private:
	enum Direction
	{
		ENorth,
		EEast,
		ESouth,
		EWest
	};

	// 荷物とエージェントを配置するフェーズ
	void InitSettings();
	
	// 仮想プレイのシミュレーションを行うフェーズ
	void RunSimulation();

	// シミュレーションの終了判定を行う関数
	bool isLoopEnd(const Board& board, const std::vector<bool>& isMoved, const std::vector<std::vector<unsigned int>>& pPassCount, const std::vector<std::vector<unsigned int>>& bPassCount);

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

	// ある座標の周囲のマスを調べる関数
	std::vector<sf::Vector2i> GetAroundWalls(const sf::Vector2i& point);

	// 詰みの荷物の配置がないことを確認する関数
	bool isArrangementValid(const Board& board, const std::vector<sf::Vector2i>& bPositions);
	
	// 荷物が配置できるか調べる関数
	bool existsFreeSpace(const Board& board, const std::vector<sf::Vector2i>& bPositions);

	// 押せる荷物の候補と押し始められる位置の全ての候補を求める関数
	void SetCandidates(const sf::Vector2i& pPos, const std::vector<sf::Vector2i>& bPositions, const Board& board, CandidateItems& outList);

	// プレイヤーとある荷物の可能な移動経路の全ての候補を求める関数
	void SetRoutes(const sf::Vector2i& pPos, const std::vector<sf::Vector2i>& bPositions, const Board& board, std::unordered_map<int, std::vector<Route>>& outRoutes, const CandidateItems& initPositions, const std::vector<std::vector<unsigned int>>& pPassed, const std::vector<std::vector<unsigned int>>& bPassed);

	// プレイヤーの現在地から荷物の横までの移動経路を求める関数
	Route FindRouteToBaggage(const sf::Vector2i& pCoordinate, const sf::Vector2i& goal, const Board& board, const std::vector<std::vector<unsigned int>>& pPassed, const std::vector<std::vector<unsigned int>>& bPassed);

	// プレイヤーの移動経路を受け取り、シミュレートを行う関数
	void MoveOnPath(
		const Route& route,
		Board& board,
		sf::Vector2i& pPos,
		sf::Vector2i& bPos,
		std::vector<std::vector<unsigned int>>& pPassCount,
		std::vector<std::vector<unsigned int>>& bPassCount);

	// 経路を評価し、適当なものを選択する関数
	std::pair<int, Route> GetBestRoute(
		const std::unordered_map<int, std::vector<Route>>& routes,
		const Board& board,
		const sf::Vector2i& pPos,
		const std::vector<sf::Vector2i> bPositions,
		const std::vector<std::vector<unsigned int>>& pPassCount,
		const std::vector<std::vector<unsigned int>>& bPassCount);

	// 探索関連の変数
	const std::vector<sf::Vector2i> directions = {
		sf::Vector2i(1, 0),   // 右
		sf::Vector2i(0, 1),  // 下
		sf::Vector2i(-1, 0),   // 左
		sf::Vector2i(0, -1)   // 上
	};

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

	// 座標のデータ構造の変換
	std::pair<int, int> Vec2iToPair(const sf::Vector2i& pos)
	{
		return std::make_pair(pos.x, pos.y);
	}
	sf::Vector2i PairToVec2i(const std::pair<int, int>& pos)
	{
		return sf::Vector2i{ pos.first, pos.second };
	}

	// 乱数関連の変数
	std::random_device rd;
	std::mt19937 mt;

	// 盤面関連の変数
	sf::Vector2i mSize;
	Board mBoardStr;	// 盤面の情報を表す
	int mBaggageNum;	// 荷物の数
	std::vector<sf::Vector2i> mBaggagesCoordinate;	// 荷物の置いてある座標のリスト

	// 盤面の生成に関する変数
	const int mBuildsNum;
	const int mRunsNum;

	// 解の上界
	int mSolveUpper;

	// 荷物とゴールの対応
	std::vector<std::pair<sf::Vector2i, sf::Vector2i>> mInitToGoalList;
};