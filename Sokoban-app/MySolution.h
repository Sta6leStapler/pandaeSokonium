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
struct History
{
	Board mBoard;
	std::vector<std::vector<unsigned int>> mPPass;
	std::vector<std::vector<unsigned int>> mBPass;
	int mSolveUpper;
};

struct Rating
{
	double totalRate;
	int m2x2Spaces;				// 2x2の数
	int m2x3Spaces;				// 2x3の数
	int m3x3Spaces;				// 3x3の数
	int mSwitchCount;			// 運搬する荷物を切り替えた回数
	int mDisCenterToGoal;		// 
	int mGoalArround;			// 
	int mDeadEnd;				// 周囲の壁が3つ以上の荷物の数
	int mCornerEnd;				// 角となるような地形にある荷物の数 (座標的な角ではない)
	int mSides;					// 盤面の外周の辺に配置されている荷物の数
	int mCorners;				// 盤面の外周の角に配置されている荷物の数
	int mChangedCandidates;		// 移動前後で動かせるかどうかが変わった荷物の数
};

class MySolution
{
public:
	MySolution(
		const sf::Vector2i& size,
		const int& baggageNum,
		const int& buildNum,
		const int& runsNum,
		const double& initialWallRatio,
		const double& visitedRatio,
		const int& strategyIndex);

	// ゲッターセッター
	Board GetBoard() { return mBoardStr; }
	int GetBaggageNum() { return mBaggageNum; }
	std::vector<std::pair<sf::Vector2i, sf::Vector2i>> GetInitToGoalList() { return mInitToGoalList; }
	int GetSolveUpper() { return mSolveUpper; }
	// 解の取得
	std::vector<sf::Vector2i> GetSolutionPos() { return mSolutionPos; }
	std::string GetSolutionDirection() { return mSolutionDirection; }

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

	// 壁タイルをランダムに配置する関数
	void SetWallTiles(const int& wallCount);

	// 床タイルの周囲3方向以上が壁タイルで囲まれているかどうかの判定を行う関数
	bool hasEnclosedFloor(const Board& board);

	// 盤面が2つ以上に分断されていないかを判定する関数
	bool isBoardPartitioned(const Board& board);

	// 床タイルをランダムに訪問済みに初期化する関数
	void SetVisitedStatus(std::vector<std::vector<unsigned int>>& visitedCount, const Board& board);

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
	std::vector<sf::Vector2i> GetRandomDirections();

	// ある座標の周囲のマスを調べる関数
	std::vector<sf::Vector2i> GetAroundWalls(const sf::Vector2i& point);

	// 詰みの荷物の配置がないことを確認する関数
	bool isArrangementValid(const Board& board, const std::vector<sf::Vector2i>& bPositions);
	
	// 荷物が配置できるか調べる関数
	bool existsFreeSpace(const Board& board, const std::vector<sf::Vector2i>& bPositions);

	bool existsDuplicateHistory(const std::vector<History> history, const Board& board);

	// 押せる荷物の候補と押し始められる位置の全ての候補を求める関数
	void SetCandidates(const sf::Vector2i& pPos, const std::vector<sf::Vector2i>& bPositions, const Board& board, CandidateItems& outList, const int& prevBaggageIndex);
	void SetCandidates(const sf::Vector2i& pPos, const std::vector<sf::Vector2i>& bPositions, const Board& board, std::vector<int>& outList);

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
		std::vector<std::vector<unsigned int>>& bPassCount,
		const bool& isSave);

	// 経路を評価し、適当なものを選択する関数
	std::pair<int, Route> GetBestRoute(
		const std::unordered_map<int, std::vector<Route>>& routes,
		const Board& board,
		const sf::Vector2i& pPos,
		const std::vector<sf::Vector2i> bPositions,
		const std::vector<std::vector<unsigned int>>& pPassCount,
		const std::vector<std::vector<unsigned int>>& bPassCount,
		const int& switchCount,
		const CandidateItems& movableBaggages);

	// 選択された評価関数の値を求める関数
	void SetEvaluationValue(Rating& rates, const int& switchCount);

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

	// 以下はメンバ変数 //
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

	// 盤面関連の変数
	sf::Vector2i mSize;
	Board mBoardStr;	// 盤面の情報を表す
	int mBaggageNum;	// 荷物の数
	std::vector<sf::Vector2i> mBaggagesCoordinate;	// 荷物の置いてある座標のリスト

	// 盤面の生成に関する変数
	const int mBuildsNum;
	const int mRunsNum;
	double mInitialWallRatio;	// 初期設定で壁タイルを配置する割合
	double mVisitedRatio;		// 訪問済みに初期化する割合
	const int mStrategyIndex;		// 経路選択の評価関数のインデックス
	std::vector<std::vector<History>> mBoardHistory;	// 盤面の履歴

	// 生成時に得られる解を記録する変数
	std::vector<sf::Vector2i> mSolutionPos;
	std::string mSolutionDirection;

	// 解の上界
	int mSolveUpper;

	// 荷物とゴールの対応
	std::vector<std::pair<sf::Vector2i, sf::Vector2i>> mInitToGoalList;
};