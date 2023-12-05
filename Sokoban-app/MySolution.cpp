#include "MySolution.h"
#include <queue>
#include <algorithm>

MySolution::MySolution(const sf::Vector2i& size, const int& baggageNum, const int& buildsNum, const int& runsNum)
	:mSize(size)
	,mBaggageNum(baggageNum)
	,mt(rd())
	,mBuildsNum(buildsNum)
	,mRunsNum(runsNum)
	,mSolveUpper(0)
{
	for (int i = 0; i < mSize.y; i++)
	{
		mBoardStr.emplace_back("");
		for (int j = 0; j < mSize.x; j++)
		{
			if (i == 0 || i == mSize.y - 1 || j == 0 || j == mSize.x - 1)
			{
				mBoardStr.back() += "#";
			}
			else
			{
				mBoardStr.back() += " ";
			}
		}
	}

	//               012345678901
	/*mBoardStr[0] =  "############";
	mBoardStr[1] =	"##  #  #   #";
	mBoardStr[2] =	"#          #";
	mBoardStr[3] =	"# $#  #    #";
	mBoardStr[4] =	"#  # ##### #";
	mBoardStr[5] =	"#          #";
	mBoardStr[6] =	"# ##########";
	mBoardStr[7] =	"#      #####";
	mBoardStr[8] =	"###### #####";
	mBoardStr[9] =	"#      $   #";
	mBoardStr[10] = "#@  #  #   #";
	mBoardStr[11] = "############";
	mBaggagesCoordinate.emplace_back(sf::Vector2i{ 7, 9 });
	mBaggagesCoordinate.emplace_back(sf::Vector2i{ 2, 3 });*/

	InitSettings();
	RunSimulation();
}

void MySolution::InitSettings()
{
	// 壁を適当に配置
	/*mBoardStr[4][5] = '#';
	mBoardStr[5][5] = '#';
	mBoardStr[6][5] = '#';
	mBoardStr[7][5] = '#';*/

	// プレイヤーの初期位置をランダムに決定
	sf::Vector2i initP(RollCoordinate());
	while (mBoardStr[initP.y][initP.x] == '#' && mBoardStr[initP.y][initP.x] == '$')
	{
		initP = RollCoordinate();
	}
	mBoardStr[initP.y][initP.x] = '@';

	// 盤面情報のコピー
	Board cpBoard = mBoardStr;
	std::vector<sf::Vector2i> cpBaggagePos;

	// 指定された数だけ荷物を配置
	while (mBaggagesCoordinate.size() < mBaggageNum)
	{
		// これ以上荷物を配置できない場合、全ての荷物を削除してやり直し
		if (existsFreeSpace(cpBoard, cpBaggagePos))
		{
			for (int i = 0; i < mBaggagesCoordinate.size(); i++)
			{
				mBoardStr[mBaggagesCoordinate.back().y][mBaggagesCoordinate.back().x] = ' ';
				mBaggagesCoordinate.pop_back();
			}

			for (int i = 0; i < cpBaggagePos.size(); i++)
			{
				cpBoard[cpBaggagePos.back().y][cpBaggagePos.back().x] = ' ';
				cpBaggagePos.pop_back();
			}
		}

		sf::Vector2i tmpCoordinate(RollCoordinate(2, mSize.x - 3, 2, mSize.y - 3));

		// 配置先の座標に被りがないことを確認
		if (mBoardStr[tmpCoordinate.y][tmpCoordinate.x] != '#' && initP != tmpCoordinate && std::find(mBaggagesCoordinate.begin(), mBaggagesCoordinate.end(), tmpCoordinate) == mBaggagesCoordinate.end())
		{
			// 詰みの荷物の配置がないことを確認する
			cpBoard[tmpCoordinate.y][tmpCoordinate.x] = '$';
			cpBaggagePos.emplace_back(tmpCoordinate);
			if (isArrangementValid(cpBoard, cpBaggagePos))
			{
				mBaggagesCoordinate.emplace_back(tmpCoordinate);
				mBoardStr[tmpCoordinate.y][tmpCoordinate.x] = '$';
			}
			else
			{
				cpBoard[tmpCoordinate.y][tmpCoordinate.x] = ' ';
				cpBaggagePos.pop_back();
			}
		}
	}
}

void MySolution::RunSimulation()
{
	// 盤面情報のコピー
	Board cpBoard = mBoardStr;

	// 仮想荷物の現在地
	std::vector<sf::Vector2i> vbPoints;

	// シミュレートを実行
	for (int count = 0; count < mBuildsNum; count++)
	{
		// 盤面上で移動した回数をカウントする変数
		// プレイヤー
		std::vector<std::vector<unsigned int>> PPassCount(mSize.y, std::vector<unsigned int>(mSize.x, 0));
		// 荷物
		std::vector<std::vector<unsigned int>> BPassCount(mSize.y, std::vector<unsigned int>(mSize.x, 0));

		// 盤面情報のコピー
		cpBoard = mBoardStr;

		// 仮想プレイヤーの現在地
		sf::Vector2i vpPoint(0, 0);
		for (int i = 1; i < mSize.y; i++)
		{
			for (int j = 1; j < mSize.x - 1; j++)
			{
				if (mBoardStr[i][j] == '@')
				{
					vpPoint.x = j;
					vpPoint.y = i;
				}
			}
		}

		// 仮想荷物の現在地
		vbPoints = mBaggagesCoordinate;

		// プレイヤーの初期位置と荷物の初期位置の座標のみ移動した回数のカウントを増やす
		PPassCount[vpPoint.y][vpPoint.x]++;
		for (const auto& item : vbPoints)
		{
			BPassCount[item.y][item.x]++;
		}

		// 荷物を1回以上移動させたかどうかのフラグ
		std::vector<bool> isMoved(mBaggageNum, false);

		// プレイヤーと荷物の移動
		for (int i = 0; i < mRunsNum || isLoopEnd(cpBoard, isMoved, PPassCount, BPassCount); i++)
		{
			// 押せる荷物と押し始められる位置のリストを求める
			CandidateItems movableBaggages{};
			SetCandidates(vpPoint, vbPoints, cpBoard, movableBaggages);

			if (movableBaggages.empty())
			{
				break;
			}

			// 押す荷物と押し始める位置のリストから、プレイヤーの移動経路を求める
			std::unordered_map<int, std::vector<Route>> pRoutes{};
			SetRoutes(vpPoint, vbPoints, cpBoard, pRoutes, movableBaggages, PPassCount, BPassCount);

			if (pRoutes.empty())
			{
				break;
			}

			// 経路を選択する
			//auto bId = GetRandomElement(pRoutes).first;
			std::pair<int, Route> pRoute(GetBestRoute(pRoutes, cpBoard, vpPoint, vbPoints, PPassCount, BPassCount));//(bid, GetRandomElement(pRoutes[bId]));
			isMoved[pRoute.first] = true;

			// プレイヤーの移動経路に沿ってプレイヤーと荷物を移動させる
			MoveOnPath(pRoute.second, cpBoard, vpPoint, vbPoints[pRoute.first], PPassCount, BPassCount);

			// 解の上界を求めるために、経路の長さだけ加算
			mSolveUpper += pRoute.second.size();
		}

		// 移動した回数を基に、地形を形成
		for (int i = 0; i < mSize.y; i++)
		{
			for (int j = 0; j < mSize.x; j++)
			{
				if ((PPassCount[i][j] == 0 && BPassCount[i][j] == 0) && mBoardStr[i][j] == ' ')
				{
					mBoardStr[i][j] = '#';
				}
			}
		}
	}

	// ゴール位置の設定
	for (const auto& vbPoint : vbPoints)
	{
		if (mBoardStr[vbPoint.y][vbPoint.x] == ' ')
		{
			mBoardStr[vbPoint.y][vbPoint.x] = '.';
		}
		else if (mBoardStr[vbPoint.y][vbPoint.x] == '$')
		{
			mBoardStr[vbPoint.y][vbPoint.x] = '*';
		}
		else if (mBoardStr[vbPoint.y][vbPoint.x] == '@')
		{
			mBoardStr[vbPoint.y][vbPoint.x] = '+';
		}
	}

	// 荷物とゴールの対応を作る
	for (int i = 0; i < mBaggageNum; i++)
	{
		mInitToGoalList.emplace_back(std::make_pair(mBaggagesCoordinate[i], vbPoints[i]));
	}
}

bool MySolution::isLoopEnd(const Board& board, const std::vector<bool>& isMoved, const std::vector<std::vector<unsigned int>>& pPassCount, const std::vector<std::vector<unsigned int>>& bPassCount)
{
	// 全ての荷物を動かしており、かつ荷物の周囲3方が壁で囲まれていないか
	// TODO 動かせない荷物がある場合
	// いずれも満たしている場合は終了できる
	
	if (std::find(isMoved.begin(), isMoved.end(), false) == isMoved.end())
	{
		return true;
	}

	for (int x = 1; x < mSize.x - 1; x++)
	{
		for (int y = 1; y < mSize.y - 1; y++)
		{
			if (board[y][x] == '$')
			{
				int wallCount = 0;
				for (const auto& dir : directions)
				{
					if (pPassCount[y + dir.y][x + dir.x] == 0 && bPassCount[y + dir.y][x + dir.x] == 0)
					{
						++wallCount;
					}
				}

				if (wallCount == 3)
				{
					return true;
				}
			}
		}
	}

	return false;
}

MySolution::Direction MySolution::RollDirection()
{
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

sf::Vector2i MySolution::RollCoordinate()
{
	return sf::Vector2i{ RollCoordinateX(), RollCoordinateY() };
}

sf::Vector2i MySolution::RollCoordinate(const int& xMin, const int& xMax, const int& yMin, const int& yMax)
{
	return sf::Vector2i{ RollCoordinateX(xMin, xMax), RollCoordinateY(yMin, yMax) };
}

int MySolution::RollCoordinateX()
{
	std::uniform_int_distribution<int> randX(1, mSize.x - 2);
	// 乱数を生成する 
	return randX(mt);
}

int MySolution::RollCoordinateX(const int& xMin, const int& xMax)
{
	std::uniform_int_distribution<int> randX(xMin, xMax);
	// 乱数を生成する 
	return randX(mt);
}

int MySolution::RollCoordinateY()
{
	std::uniform_int_distribution<int> randX(1, mSize.y - 2);
	// 乱数を生成する 
	return randX(mt);
}

int MySolution::RollCoordinateY(const int& yMin, const int& yMax)
{
	std::uniform_int_distribution<int> randX(yMin, yMax);
	// 乱数を生成する 
	return randX(mt);
}

template <typename T>
T MySolution::GetRandomElement(const std::vector<T>& vec)
{
	if (vec.empty())
	{
		throw std::runtime_error("Vector is empty");
	}
	else if (vec.size() == 0)
	{
		throw std::runtime_error("Vector's size is zero");
	}

	std::uniform_int_distribution<size_t> dist(0, vec.size() - 1);
	return vec[dist(mt)];
}

template<typename T, typename U>
std::pair<T, U> MySolution::GetRandomElement(const std::unordered_map<T, U>& map)
{
	if (map.empty())
	{
		throw std::out_of_range("The map is empty.");
	}
	else if (map.size() == 0)
	{
		throw std::runtime_error("Vector's size is zero");
	}

	std::uniform_int_distribution<std::size_t> distribution(0, map.size() - 1);
	std::size_t randomIndex = distribution(mt);
	auto it = map.begin();
	std::advance(it, randomIndex);

	return std::make_pair(it->first, it->second);
}

std::vector<sf::Vector2i> MySolution::GetAroundWalls(const sf::Vector2i& point)
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

bool MySolution::isArrangementValid(const Board& board, const std::vector<sf::Vector2i>& bPositions)
{
	if (board[1][1] == '$' || board[board.size() - 2][1] == '$' || board[1][board[1].size() - 2] == '$' || board[board.size() - 2][board[1].size() - 2] == '$')
	{
		return false;
	}

	for (const auto& bPos : bPositions)
	{
		for (int i = 0; i < directions.size(); i++)
		{
			if ((board[bPos.y + directions[i].y][bPos.x + directions[i].x] == '#' || board[bPos.y + directions[i].y][bPos.x + directions[i].x] == '$') &&
				(board[bPos.y + directions[i].y + directions[(i + 1) % 4].y][bPos.x + directions[i].x + directions[(i + 1) % 4].x] == '#' || board[bPos.y + directions[i].y + directions[(i + 1) % 4].y][bPos.x + directions[i].x + directions[(i + 1) % 4].x] == '$') &&
				(board[bPos.y + directions[(i + 1) % 4].y][bPos.x + directions[(i + 1) % 4].x] == '#' || board[bPos.y + directions[(i + 1) % 4].y][bPos.x + directions[(i + 1) % 4].x] == '$'))
			{
				return false;
			}
		}
	}

	return true;
}

bool MySolution::existsFreeSpace(const Board& board, const std::vector<sf::Vector2i>& bPositions)
{
	for (int x = 2; x < mSize.x - 3; x++)
	{
		for (int y = 2; y < mSize.y - 3; y++)
		{
			if (board[y][x] != '@' && board[y][x] != '$')
			{
				// 1つでも配置できる場所があれば偽を出力
				// flagは配置できないことを確認し次第偽にし、次の位置を調べるかどうかを判定
				bool flag = true;
				for (int i = 0; i < 4; i++)
				{
					// 四方が壁や荷物で囲まれているか
					if ((board[y + directions[i].y][x + directions[i].x] == '$' || board[y + directions[i].y][x + directions[i].x] == '#') &&
						(board[y + directions[i].y + directions[(i + 1) % 4].y][x + directions[i].x + directions[(i + 1) % 4].x] == '$' || board[y + directions[i].y + directions[(i + 1) % 4].y][x + directions[i].x + directions[(i + 1) % 4].x] == '#') &&
						(board[y + directions[(i + 1) % 4].y][x + directions[(i + 1) % 4].x] == '$' || board[y + directions[(i + 1) % 4].y][x + directions[(i + 1) % 4].x] == '#'))
					{
						flag = false;
						break;
					}

					// 縦横に壁があるか
					if (board[y + directions[i].y][x + directions[i].x] == '#' &&
						board[y + directions[(i + 1) % 4].y][x + directions[(i + 1) % 4].x] == '#')
					{
						flag = false;
						break;
					}
				}

				if (flag)
				{
					return false;
				}
			}
		}
	}

	return true;
}

void MySolution::SetCandidates(const sf::Vector2i& pPos, const std::vector<sf::Vector2i>& bPositions, const Board& board, CandidateItems& outList)
{
	// 荷物ごとに、プレイヤーが押し始める位置に移動できる座標のリストを求める
	// 全ての荷物を壁とみなした盤面を考える
	Board baseBoard = board;
	for (const auto& bPos : bPositions)
	{
		baseBoard[bPos.y][bPos.x] = '#';
	}

	// 全ての荷物に対して調べる
	for (size_t i = 0; i < bPositions.size(); i++)
	{
		// 対象の荷物の座標のみを荷物に戻す
		Board tmpBoard = baseBoard;
		tmpBoard[bPositions[i].y][bPositions[i].x] = '$';

		// 探索したかどうかを保持する変数
		std::vector<std::vector<bool>> unexplored(mSize.y, std::vector<bool>(mSize.x, true));
		// 荷物の座標を探索済みにする
		unexplored[pPos.y][pPos.x] = false;

		std::queue<sf::Vector2i> q;
		q.emplace(pPos);

		while (!q.empty())
		{
			const sf::Vector2i current = q.front();
			q.pop();

			// 探索中の現在地の四方向を調べる
			for (const auto& dir : directions)
			{
				sf::Vector2i next(current.x + dir.x, current.y + dir.y);

				// 現在調べている方向が床タイルで、未探索なら探索を続ける
				if (tmpBoard[next.y][next.x] == ' ' && unexplored[next.y][next.x])
				{
					q.emplace(next);
					unexplored[next.y][next.x] = false;
				}
				// 現在調べている方向に荷物があり、その方向に押すことができるのならば該当荷物の座標のリストに追加
				else if (tmpBoard[next.y][next.x] == '$' && tmpBoard[next.y + dir.y][next.x + dir.x] == ' ' && next - dir == current)
				{
					if (outList.find(i) == outList.end())
					{
						outList.emplace(i, std::vector<sf::Vector2i>{ sf::Vector2i{ next.x - dir.x, next.y - dir.y } });
					}
					else
					{
						outList[i].emplace_back(sf::Vector2i{ next.x - dir.x, next.y - dir.y });
					}
					unexplored[next.y][next.x] = false;
				}
				else
				{
					unexplored[next.y][next.x] = false;
				}
			}
		}
	}
}

void MySolution::SetRoutes(
	const sf::Vector2i& pPos,
	const std::vector<sf::Vector2i>& bPositions,
	const Board& board,
	std::unordered_map<int, std::vector<Route>>& outRoutes,
	const CandidateItems& initPositions,
	const std::vector<std::vector<unsigned int>>& pPassed,
	const std::vector<std::vector<unsigned int>>& bPassed)
{
	// 荷物ごとに運び込むことが可能な全ての座標への経路のリストを作る
	// プレイヤーが荷物の隣まで移動する段階では経路を作らず、荷物を1マス押すたびに経路として登録しておく
	// 荷物を押したときに、押した後の座標が既に作られた経路の中にあり、かつその経路より運び込むまでの距離が長い場合は、経路に追加しない
	std::map<std::pair<std::pair<int, int>, std::pair<int, int>>, Route> routes;
	Board baseBoard(board);
	for (const auto& bPos : bPositions)
	{
		baseBoard[bPos.y][bPos.x] = '#';
	}

	for (const auto& initPosition : initPositions)
	{
		// 対象の荷物以外を壁とみなした盤面を作る
		Board tmpBoard = baseBoard;
		tmpBoard[bPositions[initPosition.first].y][bPositions[initPosition.first].x] = '$';

		// ダイクストラ法の探索で用いる、盤面を表現するグラフ
		// 床タイルの座標をキーとし、その床タイルに隣接している床タイルと、その距離を表す連想配列を格納
		std::map<std::pair<int, int>, std::map<std::pair<int, int>, unsigned int>> graph;
		// 初期化
		for (int x = 1; x < mSize.x - 1; x++)
		{
			for (int y = 1; y < mSize.y - 1; y++)
			{
				if (tmpBoard[y][x] == ' ' || tmpBoard[y][x] == '@' || tmpBoard[y][x] == '$')
				{
					for (const auto& dir : directions)
					{
						if (tmpBoard[y + dir.y][x + dir.x] == ' ' || tmpBoard[y + dir.y][x + dir.x] == '@' || tmpBoard[y + dir.y][x + dir.x] == '$')
						{
							// 現在のタイルか、隣接しているタイルのいずれかが未到達部分の場合
							// 辺の重みを盤面の縦×横×4の数値に設定
							if ((pPassed[y][x] == 0 && bPassed[y][x] == 0) || (pPassed[y + dir.y][x + dir.x] == 0 && bPassed[y + dir.y][x + dir.x] == 0))
							{
								graph[std::make_pair(x, y)][std::make_pair(x + dir.x, y + dir.y)] = mSize.x * mSize.y * 4;
							}
							// 現在のタイルと、隣接しているタイルの両方が既に通った部分の場合
							// 辺の重みを1に設定
							else
							{
								graph[std::make_pair(x, y)][std::make_pair(x + dir.x, y + dir.y)] = 1;
							}
						}
					}
				}
			}
		}

		// スタート地点からの各床タイルへの距離を表す変数
		// ただし、同じタイルであっても、移動元のタイルごとに場合分けする
		// first -> 移動元のタイル
		// second -> 移動先のタイル
		// 距離は移動先のタイルまでの距離
		std::map<std::pair<std::pair<int, int>, std::pair<int, int>>, unsigned int> distances;
		//初期化
		for (const auto& fromPos : graph)
		{
			for (const auto& toPos : fromPos.second)
			{
				distances[std::make_pair(fromPos.first, toPos.first)] = UINT_MAX;
				distances[std::make_pair(toPos.first, fromPos.first)] = UINT_MAX;
			}
		}

		for (const auto& pos : initPosition.second)
		{
			distances[std::make_pair(std::make_pair(bPositions[initPosition.first].x, bPositions[initPosition.first].y), std::make_pair(pos.x, pos.y))] = 0;
		}

		// 座標と最短距離を保持する優先度付きキュー
		// 1. 目標の床タイルまでの重みを加味した距離
		// 2. プレイヤーの座標
		// 3. 目標の床タイルの座標
		// 4. 1つ前の床タイルの座標
		// 5. 目標の床タイルへ移動するまでのプレイヤーの経路
		// 6. 目標の床タイルへ移動させたときの盤面の状態
		// 7. 目標の床タイルへ移動させたときのプレイヤーの踏破状況
		// 8. 目標の床タイルへ移動させたときの荷物の踏破状況
		struct Elem
		{
			int mLen;
			sf::Vector2i pPos;
			sf::Vector2i bPos;
			sf::Vector2i prevPos;
			Route pRoute;
			Board currentBoard;
			std::vector<std::vector<unsigned int>> currentPPassed;
			std::vector<std::vector<unsigned int>> currentBPassed;

			bool operator<(const Elem& other) const
			{
				return mLen < other.mLen;
			}
		};

		std::priority_queue<Elem> pq;
		Elem initElem{
			0,
			pPos,
			bPositions[initPosition.first],
			sf::Vector2i{ -1, -1 },//initPositions.second.front(),
			Route{},//FindRouteToBaggage(pPos, pos, tmpBoard, pPassed, bPassed),
			tmpBoard,
			pPassed,
			bPassed };
		//MoveOnPath(initElem.pRoute, initElem.currentBoard, initElem.pPos, initElem.bPos, initElem.currentPPassed, initElem.currentBPassed);
		pq.push(initElem);
		distances[std::make_pair(Vec2iToPair(initElem.bPos), Vec2iToPair(initElem.prevPos))] = 1;

		while (!pq.empty())
		{
			// C++の優先度付きキューはデフォルトが降順なので、符号を入れ替える
			// キューに追加するときは負の値にして、取り出すときは正の値にする
			Elem currentElem = pq.top();
			pq.pop();

			// 既に最短経路が更新されている場合
			if (-currentElem.mLen > distances[std::make_pair(Vec2iToPair(currentElem.bPos), Vec2iToPair(currentElem.prevPos))])
			{
				continue;
			}

			// 荷物の隣接部分を調べる
			for (const auto& neighbor : graph[Vec2iToPair(currentElem.bPos)])
			{
				// プレイヤーは荷物を目的の隣接部分へ移動させるための位置に移動できるか？
				sf::Vector2i neighborPos{ neighbor.first.first, neighbor.first.second };
				Route toNextPos(FindRouteToBaggage(currentElem.pPos, currentElem.bPos - (neighborPos - currentElem.bPos), currentElem.currentBoard, currentElem.currentPPassed, currentElem.currentBPassed));
				if ((currentElem.currentBoard[neighbor.first.second][neighbor.first.first] == ' ' ||
					currentElem.currentBoard[neighbor.first.second][neighbor.first.first] == '@') &&
					(currentElem.pPos == currentElem.bPos - (neighborPos - currentElem.bPos) ||
					!toNextPos.empty()))
				{
					// 次の地点までの距離
					int neighbor_dist = -currentElem.mLen + neighbor.second;
					// 現在地から次の地点までの位置と向き
					std::pair<std::pair<int, int>, std::pair<int, int>> path(neighbor.first, Vec2iToPair(currentElem.bPos));
					// 最短距離であれば、キューに追加
					if (neighbor_dist < distances[path])
					{
						Elem nextElem(currentElem);
						toNextPos.emplace_back(nextElem.bPos);
						nextElem.pRoute.insert(nextElem.pRoute.end(), toNextPos.begin(), toNextPos.end());
						nextElem.prevPos = nextElem.bPos;
						MoveOnPath(toNextPos, nextElem.currentBoard, nextElem.pPos, nextElem.bPos, nextElem.currentPPassed, nextElem.currentBPassed);
						nextElem.mLen = -neighbor_dist;
						distances[path] = neighbor_dist;
						pq.push(nextElem);

						// 経路を更新
						routes[path] = nextElem.pRoute;
					}
				}
			}
		}

		for (const auto& route : routes)
		{
			outRoutes[initPosition.first].emplace_back(route.second);
		}
		routes.clear();
	}
}

Route MySolution::FindRouteToBaggage(const sf::Vector2i& pCoordinate, const sf::Vector2i& goal, const Board& board, const std::vector<std::vector<unsigned int>>& pPassed, const std::vector<std::vector<unsigned int>>& bPassed)
{
	// プレイヤーの現在地点から目標地点までの経路を座標のリストで返す
	// 経路が見つからない場合は空の配列を返す
	Route result;

	// ダイクストラ法の探索で用いるグラフ
	std::map<std::pair<int, int>, std::map<std::pair<int, int>, unsigned int>> graph;
	// 初期化
	for (int x = 1; x < mSize.x - 1; x++)
	{
		for (int y = 1; y < mSize.y - 1; y++)
		{
			if (board[y][x] == ' ' || board[y][x] == '@')
			{
				for (const auto& dir : directions)
				{
					if (board[y + dir.y][x + dir.x] == ' ' || board[y + dir.y][x + dir.x] == '@')
					{
						// 現在のタイルか、隣接しているタイルのいずれかが未到達部分の場合
						// 辺の重みを盤面の縦×横の数値に設定
						if ((pPassed[y][x] == 0 && bPassed[y][x] == 0) || (pPassed[y + dir.y][x + dir.x] == 0 && bPassed[y + dir.y][x + dir.x] == 0))
						{
							graph[std::make_pair(x, y)][std::make_pair(x + dir.x, y + dir.y)] = mSize.x * mSize.y;
						}
						// 現在のタイルと、隣接しているタイルの両方が既に通った部分の場合
						// 辺の重みを1に設定
						else
						{
							graph[std::make_pair(x, y)][std::make_pair(x + dir.x, y + dir.y)] = 1;
						}
					}
				}
			}
		}
	}
	
	// ゴール地点が四方を壁と荷物に囲まれている場合は空のまま出力
	if (graph[std::make_pair(goal.x, goal.y)].empty())
	{
		return result;
	}

	// スタート地点からの各床タイルへの距離を表す変数
	std::map<std::pair<int, int>, unsigned int> distance;
	//初期化
	for (const auto& item : graph)
	{
		distance[item.first] = UINT_MAX;
	}
	// スタート地点の座標は距離を0としておく
	distance[std::make_pair(pCoordinate.x, pCoordinate.y)] = 0;
	
	std::priority_queue<std::pair<int, std::pair<int, int>>> pq;
	pq.push({ 0, std::make_pair(pCoordinate.x, pCoordinate.y) });

	while (!pq.empty())
	{
		int dist = -pq.top().first;
		std::pair<int, int> current = pq.top().second;
		pq.pop();

		// 既に最短経路が更新されている場合
		if (dist > distance[current])
		{
			continue;
		}

		for (const auto& neighbor : graph[current])
		{
			int neighbor_dist = dist + neighbor.second;
			if (neighbor_dist < distance[neighbor.first])
			{
				distance[neighbor.first] = neighbor_dist;
				pq.push({ -neighbor_dist, neighbor.first });
			}
		}
	}
	
	// 経路が見つからなかった場合は空のまま出力
	if (distance[std::make_pair(goal.x, goal.y)] == UINT_MAX)
	{
		return result;
	}

	// 最短経路を構築
	sf::Vector2i current{ goal.x, goal.y };

	while (current != pCoordinate)
	{
		result.emplace_back(current);
		std::pair<int, int> curPair(current.x, current.y);
		for (const auto& neighbor : graph[curPair])
		{
			sf::Vector2i neighborPos{ neighbor.first.first, neighbor.first.second };
			if (distance[curPair] == distance[neighbor.first] + neighbor.second && std::find(result.begin(), result.end(), neighborPos) == result.end())
			{
				current = neighborPos;
				break;
			}
		}
	}
	std::reverse(result.begin(), result.end());

	return result;
}

void MySolution::MoveOnPath(
	const Route& route,
	Board& board,
	sf::Vector2i& pPos,
	sf::Vector2i& bPos,
	std::vector<std::vector<unsigned int>>& pPassCount,
	std::vector<std::vector<unsigned int>>& bPassCount)
{
	for (const auto& nextPos : route)
	{
		// 次のタイルに何もなかった場合
		if (board[nextPos.y][nextPos.x] == ' ')
		{
			board[pPos.y][pPos.x] = ' ';
			board[nextPos.y][nextPos.x] = '@';
			pPassCount[nextPos.y][nextPos.x]++;
			pPos = nextPos;
		}
		// 次のタイルに荷物があった場合
		else if (board[nextPos.y][nextPos.x] == '$')
		{
			// 荷物の移動先の座標
			sf::Vector2i nextNextPos(nextPos.x + (nextPos.x - pPos.x), nextPos.y + (nextPos.y - pPos.y));
			if (board[nextNextPos.y][nextNextPos.x] == ' ')
			{
				board[pPos.y][pPos.x] = ' ';
				board[nextPos.y][nextPos.x] = '@';
				board[nextNextPos.y][nextNextPos.x] = '$';
				pPassCount[nextPos.y][nextPos.x]++;
				bPassCount[nextNextPos.y][nextNextPos.x]++;
				pPos = nextPos;
				bPos = nextNextPos;
			}
			else
			{
				// error
				std::cout << "error : in MoveOnPath 01" << std::endl << "( " << pPos.x << ", " << pPos.y << " ) -> ( " << nextPos.x << ", " << nextPos.y << " )" << std::endl;
			}
		}
		else
		{
			// OK
			//std::cout << "error : in MoveOnPath 02" << std::endl;
		}
	}
}

std::pair<int, Route> MySolution::GetBestRoute(
	const std::unordered_map<int, std::vector<Route>>& routes,
	const Board& board,
	const sf::Vector2i& pPos,
	const std::vector<sf::Vector2i> bPositions,
	const std::vector<std::vector<unsigned int>>& pPassCount,
	const std::vector<std::vector<unsigned int>>& bPassCount)
{
	struct Rating
	{
		double totalRate;
		int mSpaces;
		int mDisCenterToGoal;
		int mGoalArround;
		int mDeadEnd;
		int mCornerEnd;
		int mSides;
		int mCorners;
	};

	std::pair<int, Route> result;
	// 荷物ごとに荷物が持つ全経路のレートの連想配列を作る
	std::unordered_map<int, std::unordered_map<int, Rating>> mRates;
	std::pair<int, std::pair<int, double>> minRate(-1, { -1, DBL_MAX });

	// cerRouteは荷物のIDをキー、経路のリストを値として持つペア
	for (const auto& cerRoute : routes)
	{
		mRates[cerRoute.first] = std::unordered_map<int, Rating>{};
		auto bSize = cerRoute.second.size();
		for (int i = 0; i < bSize; i++)
		{
			// 盤面のコピーを作り、プレイヤーが経路通りに移動した後の盤面を作る
			Board cpBoard = board;
			sf::Vector2i cpPos = pPos;
			sf::Vector2i cbPos = bPositions[cerRoute.first];
			std::vector<std::vector<unsigned int>> cpPassCount = pPassCount;
			std::vector<std::vector<unsigned int>> cbPassCount = bPassCount;

			// プレイヤーの移動
			MoveOnPath(cerRoute.second[i], cpBoard, cpPos, cbPos, cpPassCount, cbPassCount);

			// 2x3スペースの数のカウント
			int countAreas = 0;
			int xSize = 2, ySize = 3;
			for (int x = 1; x < mSize.x - xSize; x++)
			{
				for (int y = 1; y < mSize.y - ySize; y++)
				{
					bool flag = true;
					for (int m = x; m < x + xSize; m++)
					{
						for (int n = y; n < y + ySize; n++)
						{
							if (cpPassCount[n][m] == 0 && cbPassCount[n][m] == 0)
							{
								flag = false;
								break;
							}
						}
					}

					if (flag)
					{
						countAreas++;
					}
				}
			}
			xSize = 3;
			ySize = 2;
			for (int x = 1; x < mSize.x - xSize; x++)
			{
				for (int y = 1; y < mSize.y - ySize; y++)
				{
					bool flag = true;
					for (int m = x; m < x + xSize; m++)
					{
						for (int n = y; n < y + ySize; n++)
						{
							if (cpPassCount[n][m] == 0 && cbPassCount[n][m] == 0)
							{
								flag = false;
								break;
							}
						}
					}

					if (flag)
					{
						countAreas++;
					}
				}
			}

			// 行き止まりの数のカウント
			// 角に押し込まれて動かせない荷物の数のカウント
			// 盤面の辺上にある荷物の数のカウント
			// 盤面の角にある荷物の数のカウント
			int deadEnds = 0, cornerEnds = 0, sides = 0, corners = 0;
			for (int x = 1; x < mSize.x - 1; x++)
			{
				for (int y = 1; y < mSize.y - 1; y++)
				{
					if (cpBoard[y][x] == '$')
					{
						// 荷物の周囲の壁を調べる
						int wallCount = 0;
						for (const auto& dir : directions)
						{
							if (cpPassCount[y + dir.y][x + dir.x] == 0 && cbPassCount[y + dir.y][x + dir.x] == 0)
							{
								++wallCount;
							}
						}

						if (wallCount >= 3)
						{
							++deadEnds;
						}

						int cornerCount = 0;
						// 荷物が角に押し込まれているか調べる
						for (int j = 0; j < 4; j++)
						{
							if (cpPassCount[y + directions[j].y][x + directions[j].x] == 0 && cbPassCount[y + directions[j].y][x + directions[j].x] == 0 &&
								cpPassCount[y + directions[(j + 1) % 4].y][x + directions[(j + 1) % 4].x] == 0 && cbPassCount[y + directions[(j + 1) % 4].y][x + directions[(j + 1) % 4].x] == 0)
							{
								++cornerCount;
							}
						}

						if (cornerCount == 1)
						{
							++cornerEnds;
						}

						// 荷物の位置が辺か角か調べる
						if (x == 1 || x == mSize.x - 1 || y == 1 || y == mSize.y)
						{
							sides++;
						}

						if (x == 1 && y == 1 || x == mSize.x - 1 && y == 1 || x == 1 && y == mSize.y - 1 || x == mSize.x - 1 && y == mSize.y - 1)
						{
							corners++;
						}
					}
				}
			}


			// cerRoute.firstは荷物のインデックス、iは経路のインデックス
			mRates[cerRoute.first][i].mSpaces = countAreas;
			mRates[cerRoute.first][i].mDisCenterToGoal = 0;
			mRates[cerRoute.first][i].mGoalArround = 0;
			mRates[cerRoute.first][i].mDeadEnd = deadEnds;
			mRates[cerRoute.first][i].mCornerEnd = cornerEnds;
			mRates[cerRoute.first][i].mSides = sides;
			mRates[cerRoute.first][i].mCorners = corners;
			mRates[cerRoute.first][i].totalRate =
				mRates[cerRoute.first][i].mSpaces +
				mRates[cerRoute.first][i].mDisCenterToGoal / (mSize.x * mSize.y) +
				mRates[cerRoute.first][i].mGoalArround +
				mRates[cerRoute.first][i].mDeadEnd +
				mRates[cerRoute.first][i].mCornerEnd +
				mRates[cerRoute.first][i].mSides +
				mRates[cerRoute.first][i].mCorners;

			// レートの最小値が更新できる場合は、更新
			if (minRate.second.second > mRates[cerRoute.first][i].totalRate)
			{
				minRate.second.second = mRates[cerRoute.first][i].totalRate;
			}
		}
	}

	// 最もレートが低い経路からランダムに選ぶ
	// 最もレートの低い全ての経路を経路の長さをキーとした連想配列に格納
	std::map<int, std::vector<std::pair<int, int>>> candidate;
	for (const auto& cerRoute : routes)
	{
		auto bSize = cerRoute.second.size();
		for (int i = 0; i < bSize; i++)
		{
			if (mRates[cerRoute.first][i].totalRate == minRate.second.second)
			{
				candidate[cerRoute.second.size()].emplace_back(std::make_pair(cerRoute.first, i));
			}
		}
	}

	// 経路長の長い経路からランダムに選ぶ
	int minKey = -1;
	for (const auto& item : candidate)
	{
		if (minKey < item.first)
		{
			minKey = item.first;
		}
	}
	
	std::uniform_int_distribution<int> rand(0, candidate[minKey].size() - 1);
	// 乱数を生成する 
	auto index = rand(mt);
	minRate.first = candidate[minKey][index].first;
	minRate.second.first = candidate[minKey][index].second;

	result = { minRate.first, routes.at(minRate.first)[minRate.second.first] };

	return result;
}