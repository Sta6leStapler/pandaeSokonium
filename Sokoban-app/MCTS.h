#pragma once

#include "SFML/Graphics.hpp"
#include <string>
#include <vector>

// データ駆動型評価に関する構造体
struct boardState
{
	int tileMixingW;
	int tileMixingF;
	int blockCount;
	int baggageNum;
	int pathToGoalLength;
	int traffic01;
	int traffic02;
};

class MCTS
{
public:
	MCTS(const sf::Vector2i& size, const int& baggageNum);

	// ゲッターセッター
	std::vector<std::string> GetBoard() { return mBoardStr; }

private:
	enum Direction
	{
		ENorth,
		EEast,
		ESouth,
		EWest
	};

	void InitFaze();
	void SimFaze();

	// ランダムなパラメータを取得する関数
	Direction RollDirection();
	int RollCoordinateX();
	int RollCoordinateY();

	// ある座標の周囲のマスを調べる関数
	std::vector<sf::Vector2i> GetAroundWalls(const sf::Vector2i& point);

	// 仮想プレイヤーの移動を行う関数
	void moveVirtualPlayer(const Direction& dir, std::vector<std::string>& simField, sf::Vector2i& coordinate, std::vector<std::pair<sf::Vector2i, int>>& baggagesPushCount);

	// 盤面データから特定のオブジェクトの座標を返す
	std::vector<sf::Vector2i> GetCoordinates(const char& icon, const std::vector<std::string>& Field);

	// 荷物の座標からインデックスを取得
	int GetBaggageIndex(const sf::Vector2i& coordinate, const std::vector<std::pair<sf::Vector2i, int>>& baggagesPushCount);

	// 盤面データからデータ駆動型評価を求める
	void ReloadBoardState(const std::vector<std::string>& Field);

	sf::Vector2i mSize;
	std::vector<std::string> mBoardStr;	// 盤面の情報を表す
	int mBaggageNum;	// 荷物の数

	int randomRange01;	// 荷物を配置する割合の大きさ
	int randomRange02;	// シミュレーションの繰り返し回数

	boardState mState;
};