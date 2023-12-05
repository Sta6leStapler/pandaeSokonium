#include "MCTS.h"
#include <iostream>
#include <fstream>
#include <random>

MCTS::MCTS(const sf::Vector2i& size, const int& baggageNum)
	:mSize(size)
	,mBaggageNum(baggageNum)
	,randomRange01(20)
	,randomRange02(2000)
	, mState(boardState{ 0, 0, 0, 0, 0, 0, 0 })
{
	for (int i = 0; i < mSize.y; i++)
	{
		mBoardStr.emplace_back("");
		for (int j = 0; j < mSize.x; j++)
		{
			mBoardStr.back() += "#";
		}
	}

	InitFaze();
	SimFaze();
}

void MCTS::InitFaze()
{
	// デバイスから乱数を取得する
	std::random_device rd;
	// 乱数のシードを設定する
	std::mt19937 mt(rd());

	std::uniform_int_distribution<int> initX(1, mSize.x - 2), initY(1, mSize.y - 2);
	// プレイヤーの初期位置をランダムに決定
	sf::Vector2i initP(initX(mt), initY(mt));
	mBoardStr[initP.y][initP.x] = '@';

	// *床タイルに隣接する壁タイルを床タイルにする
	// *床タイルに床タイルのいずれかに荷物を配置する
	// の動作を繰り返す
	
	std::uniform_int_distribution<int> randLoops(mBaggageNum * 3, (mSize.x - 2) * (mSize.y - 2) - 1), randSteps(1, 100);
	// 繰り返し回数
	int numLoops = randLoops(mt);

	// 床タイルの座標のリスト
	std::vector<sf::Vector2i> floors = { initP };
	// 荷物の置いてある座標のリスト
	std::vector<sf::Vector2i> baggages;

	for (int i = 0; i < numLoops; ++i)
	{

		// 荷物の数が床タイルの数未満で、かつ荷物の配置が選択されていて、かつ荷物の数が上限に達していない場合
		if (floors.size() > baggages.size() + 1 && randSteps(mt) < randomRange01 && baggages.size() < mBaggageNum)
		{
			// プレイヤーの初期位置以外の床タイルからランダムに選び、荷物を配置
			std::uniform_int_distribution<int> randIndex(1, floors.size() - 1);
			int index = randIndex(mt);
			baggages.emplace_back(sf::Vector2i{ floors[index].x, floors[index].y });
			mBoardStr[floors[index].y][floors[index].x] = '$';
		}
		// 床の拡張
		else
		{
			// 床タイルをランダムに選択し、周囲4マスに壁タイルが1つ以上あればその壁タイルの中から一つ選ぶ
			std::uniform_int_distribution<int> randIndex(0, floors.size() - 1);
			int index = randIndex(mt);
			std::vector<sf::Vector2i> candidates = GetAroundWalls(floors[index]);
			while (candidates.empty())
			{
				index = randIndex(mt);
				candidates = GetAroundWalls(floors[index]);
			}

			std::uniform_int_distribution<int> randCandidates(0, candidates.size() - 1);
			int WtoF = randCandidates(mt);
			floors.emplace_back(sf::Vector2i{ candidates[WtoF].x, candidates[WtoF].y });
			mBoardStr[candidates[WtoF].y][candidates[WtoF].x] = ' ';
		}
	}
}

void MCTS::SimFaze()
{
	// デバイスから乱数を取得する
	std::random_device rd;
	// 乱数のシードを設定する
	std::mt19937 mt(rd());
	std::uniform_int_distribution<int> randLoops(700, randomRange02);
	// 繰り返し回数
	int numLoops = randLoops(mt);

	// 各種変数
	std::vector<std::string> simField = mBoardStr;	// シミュレーション中の盤面
	sf::Vector2i pPoint = GetCoordinates('@', simField).front();	// 仮想プレイヤーの座標
	std::vector<std::pair<sf::Vector2i, int>> baggagesPushCount;	// 荷物の現在の座標と押された回数
	std::vector<sf::Vector2i> baggagesInit = GetCoordinates('$', simField);	// 荷物の最初の座標
	std::vector<std::vector<bool>> walkedTiles(mSize.y, std::vector<bool>(mSize.x, true)); // 仮想プレイヤーと荷物が踏んだ座標
	walkedTiles[pPoint.y][pPoint.x] = false;
	for (const auto& item : baggagesInit)
	{
		walkedTiles[item.y][item.x] = false;
	}

	// baggagesInitの初期値を設定
	for (const auto& item : baggagesInit)
	{
		baggagesPushCount.emplace_back(std::make_pair(item, 0));
	}

	// 最初に決めた繰り返し回数だけ仮想プレイヤーの移動処理を繰り返す
	for (int i = 0; i < numLoops; ++i)
	{
		Direction currentD = RollDirection();
		moveVirtualPlayer(currentD, simField, pPoint, baggagesPushCount);
		walkedTiles[pPoint.y][pPoint.x] = false;
		for (const auto& item : baggagesPushCount)
		{
			walkedTiles[item.first.y][item.first.x] = false;
		}
	}

	// 2回以上押された荷物の最終地点をゴール地点に変換する
	for (const auto& item : baggagesPushCount)
	{
		if (item.second > 1)
		{
			switch (mBoardStr[item.first.y][item.first.x])
			{
			case ' ':
				mBoardStr[item.first.y][item.first.x] = '.';
				break;
			case '$':
				mBoardStr[item.first.y][item.first.x] = '*';
				break;
			case '@':
				mBoardStr[item.first.y][item.first.x] = '+';
				break;
			default:
				break;
			}
		}
	}

	// 後処理
	// *一度も模擬プレイヤーに押されなかった荷物は壁に変換
	// *一度しか模擬プレイヤーに押されなかった荷物の移動部分は全て床タイルに変換
	for (int i = 0; i < baggagesPushCount.size(); i++)
	{
		if (baggagesPushCount[i].second == 0)
		{
			mBoardStr[baggagesInit[i].y][baggagesInit[i].x] = '#';
		}
		else if (baggagesPushCount[i].second == 1)
		{
			mBoardStr[baggagesInit[i].y][baggagesInit[i].x] = ' ';
		}
	}

	// 仮想プレイヤーも荷物も踏まなかったタイルはすべて壁に変換
	for (int i = 0; i < walkedTiles.size(); i++)
	{
		for (int j = 0; j < walkedTiles[i].size(); j++)
		{
			if (walkedTiles[j][i])
			{
				mBoardStr[j][i] = '#';
			}
		}
	}
}

MCTS::Direction MCTS::RollDirection()
{
	// デバイスから乱数を取得する
	std::random_device rd;
	// 乱数のシードを設定する
	std::mt19937 mt(rd());
	std::uniform_int_distribution<int> rand(0, 3);
	// 乱数を生成する 
	switch (rand(mt))
	{
	case 0:
		return ENorth;
		break;
	case 1:
		return EEast;
		break;
	case 2:
		return ESouth;
		break;
	case 3:
		return EWest;
		break;
	default:
		break;
	}

	return ENorth;
}

int MCTS::RollCoordinateX()
{
	// デバイスから乱数を取得する
	std::random_device rd;
	// 乱数のシードを設定する
	std::mt19937 mt(rd());
	std::uniform_int_distribution<int> randX(0, mSize.x - 1);
	// 乱数を生成する 
	return randX(mt);
}

int MCTS::RollCoordinateY()
{
	// デバイスから乱数を取得する
	std::random_device rd;
	// 乱数のシードを設定する
	std::mt19937 mt(rd());
	std::uniform_int_distribution<int> randX(0, mSize.y - 1);
	// 乱数を生成する 
	return randX(mt);
}

std::vector<sf::Vector2i> MCTS::GetAroundWalls(const sf::Vector2i& point)
{
	std::vector<sf::Vector2i> result;

	// 上右左下の順に調べる
	if (point.y > 1)
	{
		if (mBoardStr[point.y - 1][point.x] == '#')
		{
			result.emplace_back(sf::Vector2i{ point.x, point.y - 1 });
		}
	}

	if (point.x > 1)
	{
		if (mBoardStr[point.y][point.x - 1] == '#')
		{
			result.emplace_back(sf::Vector2i{ point.x - 1, point.y });
		}
	}

	if (point.y < mSize.y - 2)
	{
		if (mBoardStr[point.y + 1][point.x] == '#')
		{
			result.emplace_back(sf::Vector2i{ point.x, point.y + 1 });
		}
	}

	if (point.x < mSize.x - 2)
	{
		if (mBoardStr[point.y][point.x + 1] == '#')
		{
			result.emplace_back(sf::Vector2i{ point.x + 1, point.y });
		}
	}

	return result;
}

void MCTS::moveVirtualPlayer(const Direction& dir, std::vector<std::string>& simField, sf::Vector2i& coordinate, std::vector<std::pair<sf::Vector2i, int>>& baggagesPushCount)
{
	switch (dir)
	{
	case ENorth:
		if (simField[coordinate.y - 1][coordinate.x] != '#')
		{
			if (simField[coordinate.y - 1][coordinate.x] == '$')
			{
				if (simField[coordinate.y - 2][coordinate.x] == ' ')
				{
					int baggageIndex = GetBaggageIndex(sf::Vector2i{ coordinate.x, coordinate.y - 1 }, baggagesPushCount);
					baggagesPushCount[baggageIndex].second++;
					baggagesPushCount[baggageIndex].first.y--;
					simField[coordinate.y - 1][coordinate.x] = '@';
					simField[coordinate.y - 2][coordinate.x] = '$';
					coordinate.y--;
				}
			}
			else
			{
				simField[coordinate.y - 1][coordinate.x] = '@';
				coordinate.y--;
			}
		}
		break;
	case EEast:
		if (simField[coordinate.y][coordinate.x - 1] != '#')
		{
			if (simField[coordinate.y][coordinate.x - 1] == '$')
			{
				if (simField[coordinate.y][coordinate.x - 2] == ' ')
				{
					int baggageIndex = GetBaggageIndex(sf::Vector2i{ coordinate.x - 1, coordinate.y }, baggagesPushCount);
					baggagesPushCount[baggageIndex].second++;
					baggagesPushCount[baggageIndex].first.x--;
					simField[coordinate.y][coordinate.x - 1] = '@';
					simField[coordinate.y][coordinate.x - 2] = '$';
					coordinate.x--;
				}
			}
			else
			{
				simField[coordinate.y][coordinate.x - 1] = '@';
				coordinate.x--;
			}
		}
		break;
	case ESouth:
		if (simField[coordinate.y + 1][coordinate.x] != '#')
		{
			if (simField[coordinate.y + 1][coordinate.x] == '$')
			{
				if (simField[coordinate.y + 2][coordinate.x] == ' ')
				{
					int baggageIndex = GetBaggageIndex(sf::Vector2i{ coordinate.x, coordinate.y + 1 }, baggagesPushCount);
					baggagesPushCount[baggageIndex].second++;
					baggagesPushCount[baggageIndex].first.y++;
					simField[coordinate.y + 1][coordinate.x] = '@';
					simField[coordinate.y + 2][coordinate.x] = '$';
					coordinate.y++;
				}
			}
			else
			{
				simField[coordinate.y + 1][coordinate.x] = '@';
				coordinate.y++;
			}
		}
		break;
	case EWest:
		if (simField[coordinate.y][coordinate.x + 1] != '#')
		{
			if (simField[coordinate.y][coordinate.x + 1] == '$')
			{
				if (simField[coordinate.y][coordinate.x + 2] == ' ')
				{
					int baggageIndex = GetBaggageIndex(sf::Vector2i{ coordinate.x + 1, coordinate.y }, baggagesPushCount);
					baggagesPushCount[baggageIndex].second++;
					baggagesPushCount[baggageIndex].first.x++;
					simField[coordinate.y][coordinate.x + 1] = '@';
					simField[coordinate.y][coordinate.x + 2] = '$';
					coordinate.x++;
				}
			}
			else
			{
				simField[coordinate.y][coordinate.x + 1] = '@';
				coordinate.x++;
			}
		}
		break;
	default:
		break;
	}
}

std::vector<sf::Vector2i> MCTS::GetCoordinates(const char& icon, const std::vector<std::string>& Field)
{
	std::vector<sf::Vector2i> result;

	for (int i = 0; i < Field.size(); i++)
	{
		for (int j = 0; j < Field[i].size(); j++)
		{
			if (Field[j][i] == icon)
			{
				result.emplace_back(sf::Vector2i{ i, j });
			}
		}
	}

	return result;
}

int MCTS::GetBaggageIndex(const sf::Vector2i& coordinate, const std::vector<std::pair<sf::Vector2i, int>>& baggagesPushCount)
{
	int result = 0;
	for (const auto& item : baggagesPushCount)
	{
		if (item.first == coordinate)
		{
			return result;
		}
		++result;
	}

	return result;
}

void MCTS::ReloadBoardState(const std::vector<std::string>& Field)
{

}