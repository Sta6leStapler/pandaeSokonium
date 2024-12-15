#include "HUDHelper.h"
#include <queue>
#include <algorithm>

HUDHelper::HUDHelper(class Game* game)
	: mGame(game)
	, mBaggageNum(0)
{
	// 盤面の初期化
	mBoardStr = mGame->GetBoardData()[mGame->GetCurrentKey()];
	mInitialBoardStr = mBoardStr;
	// 盤面サイズの初期化
	mSize = mGame->GetBoardSize();
	// プレイヤー座標の初期化
	mPlayerPos = mGame->GetPlayer()->GetBoardCoordinate();
	mInitialPlayerPos = mPlayerPos;
	// 荷物の座標に関するメンバ変数の初期化
	for (const auto& baggage : mGame->GetBaggages())
	{
		sf::Vector2i tmpPos{ baggage->GetBoardCoordinate() };
		int tmpIndex = mBaggagesPos.size();
		mBaggagesPos.emplace(tmpIndex, tmpPos);

		if (mBoardStr[tmpPos.y][tmpPos.x] == '$')
		{
			mNonGoaledBaggages.emplace(tmpIndex, tmpPos);
		}
		else if (mBoardStr[tmpPos.y][tmpPos.x] == '*')
		{
			mGoaledBaggages.emplace(tmpIndex, tmpPos);
		}
	}
	mInitialBaggagesPos = mBaggagesPos;
	mBaggageNum = mBaggagesPos.size();
	// ゴール座標の初期化
	for (int y = 0; y < mSize.y; ++y)
	{
		for (int x = 0; x < mSize.x; ++x)
		{
			if (mBoardStr[y][x] == '.' || mBoardStr[y][x] == '*' || mBoardStr[y][x] == '+')
			{
				mGoals.emplace_back(sf::Vector2i{ x, y });
			}
		}
	}
	// プレイヤーと荷物を取り除いた盤面情報を初期化
	mBoardMapStr = mBoardStr;
	for (const auto& baggage : mBaggagesPos)
	{
		if (mBoardMapStr[baggage.second.y][baggage.second.x] == '$')
		{
			mBoardMapStr[baggage.second.y][baggage.second.x] = ' ';
		}
		else if (mBoardMapStr[baggage.second.y][baggage.second.x] == '*')
		{
			mBoardMapStr[baggage.second.y][baggage.second.x] = '.';
		}
	}
	if (mBoardMapStr[mPlayerPos.y][mPlayerPos.x] == '@')
	{
		mBoardMapStr[mPlayerPos.y][mPlayerPos.x] = ' ';
	}
	else if (mBoardMapStr[mPlayerPos.y][mPlayerPos.x] == '+')
	{
		mBoardMapStr[mPlayerPos.y][mPlayerPos.x] = '.';
	}
	// ここまで盤面の基礎的情報の初期化
	
	// 盤面構造の初期化
	mBoardStates.mGoaledBaggages = mGoaledBaggages;
	mBoardStates.mNonGoaledBaggages = mNonGoaledBaggages;
	mBoardStates.mMovableArea = GetMovableArea();
	mBoardStates.mDeadlockedBaggages = UpdateFreezeBaggagesMap();
	// 履歴に初期盤面を追加
	std::set<std::pair<int, int>> baggagesSet{};
	for (const auto& baggage : mBaggagesPos)
	{
		baggagesSet.emplace(std::pair<int, int>{ baggage.second.x, baggage.second.y });
	}
	mHistory.emplace(std::pair<std::pair<int, int>, std::set<std::pair<int, int>>>{ std::pair<int, int>{ mPlayerPos.x, mPlayerPos.y }, baggagesSet }, mBoardStates);

}

HUDHelper::~HUDHelper()
{
	
}

void HUDHelper::Update()
{
	// プレイヤーおよび荷物の座標の更新に基づき各メンバ変数を更新
	// 更新を行う盤面関連のメンバ変数は以下
	//	mBoardStr
	//	mPlayerPos
	//	mBaggagesPos
	//	mGoaledBaggages
	//	mNonGoaledBaggages
	//	mBoardStates
	//	mHistory
	// プレイヤーと荷物の更新後の座標を取得
	// 必ずプレイヤーと荷物のアクターの更新処理の後に行う
	sf::Vector2i nextPlayerPos{ mGame->GetPlayer()->GetBoardCoordinate() }, prevPlayerPos{ mPlayerPos };
	std::vector<sf::Vector2i> nextBaggages{ mGame->GetBaggagesPos() };
	std::unordered_map<int, sf::Vector2i> prevBaggagePos{ mBaggagesPos };
	std::set<std::pair<int, int>> nextBaggagesSet{};
	for (const auto& baggage : nextBaggages)
	{
		nextBaggagesSet.emplace(std::pair<int, int>{ baggage.x, baggage.y });
	}

	// まず位置の変わった荷物があるかどうかを調べる
	bool isMoved = false;
	std::unordered_map<int, sf::Vector2i> currentBaggages{ mBaggagesPos };
	std::vector<std::pair<std::pair<int, sf::Vector2i>, sf::Vector2i>> changedBaggages{};
	std::unordered_map<int, sf::Vector2i> tmpBaggages{ currentBaggages };
	for (const auto& currentBaggage : tmpBaggages)
	{
		// 更新後の荷物のリストから、対応する荷物が存在するか調べる
		bool existsBaggage = false;
		for (const auto& nextBaggage : nextBaggages)
		{
			if (currentBaggage.second == nextBaggage)
			{
				existsBaggage = true;
				break;
			}
		}

		// 更新後の荷物の座標のリストに、現在の対象にしている荷物の座標が存在するならば、更新する荷物のリストから削除
		if (existsBaggage)
		{
			currentBaggages.erase(currentBaggage.first);
			nextBaggages.erase(std::find(nextBaggages.begin(), nextBaggages.end(), currentBaggage.second));
		}
		// そうでなければ、荷物の位置が変わったかどうかの判定を真にする
		else
		{
			isMoved = true;
		}
	}

	// 位置の更新のあった荷物の数と、荷物の移動前後の座標を取得
	// note 荷物の「本来の」移動前と移動後の位置の対応が正しくない可能性があるが、問題はないはず
	int count = 0;
	for (const auto& baggage : currentBaggages)
	{
		changedBaggages.emplace_back(std::pair<std::pair<int, sf::Vector2i>, sf::Vector2i>{ baggage, nextBaggages[count] });
		++count;
	}

	// パターン
	// 1.1	プレイヤーが移動しておらず、荷物も移動していない
	//		プレイヤーと荷物と盤面構造の更新処理は行わない
	// 1.2	プレイヤーは移動していないが、荷物は移動している (=実際はプレイヤーが移動した)
	//		荷物と盤面構造の更新処理を行う
	// 2.1.	プレイヤーが移動し、荷物は移動しておらず、プレイヤーのいる床エリアも変わっていない
	//		プレイヤーの更新処理を行う
	// 2.2.	プレイヤーが移動し、荷物は移動していないが、プレイヤーのいる床エリアが変わった
	//		プレイヤーと盤面構造の更新処理を行う
	// 3.	プレイヤーが移動し、荷物も移動した
	//		全ての更新処理を行う
	// 
	// 1.2は実質的に3と同じなので、荷物の移動から見る
	// 荷物の移動があれば、1.2と3の処理を行い、そうでなければ次の条件分岐に移る
	// 1.1と2の分岐をする (プレイヤーが移動しているかどうかを確認)
	// 2の中で2.1と2.2の分岐をする
	
	/////////////////////////////////////////////////////////////////////////////

	std::pair<int, int> nextPlayerPosPair{ nextPlayerPos.x, nextPlayerPos.y };
	std::set<std::pair<int, int>> nextBaggagesPairSet{};
	for (const auto& baggage : nextBaggagesSet)
	{
		nextBaggagesPairSet.emplace(baggage);
	}
	// 1.2, 3		荷物が移動しているかどうかで分岐
	// 荷物が移動している場合
	if (count > 0)
	{
		// 荷物やプレイヤーの配置が変わったのならば、盤面関連のメンバ変数の更新と詰み判定を行う
		// メンバ変数の更新
		mPlayerPos = nextPlayerPos;
		// 位置の変わった荷物のみ更新
		for (const auto& item : changedBaggages)
		{
			mBaggagesPos[item.first.first] = item.second;

			// ゴールに配置されているかどうかが変わる場合、そちらも更新
			// itemはfirst.secondが移動前の位置、secondが移動後の位置
			// ゴール -> 普通の床の場合
			// 移動前がゴール済み荷物のリストに存在し、移動後がゴールリストに存在しない
			if (mGoaledBaggages.count(item.first.first) && std::find(mGoals.begin(), mGoals.end(), item.second) == mGoals.end())
			{
				mGoaledBaggages.erase(item.first.first);
				mNonGoaledBaggages.emplace(item.first.first, item.second);
			}
			// 普通の床 -> ゴールの場合
			// 移動前が未ゴール荷物のリストに存在し、移動後がゴールリストに存在する
			else if (mNonGoaledBaggages.count(item.first.first) && std::find(mGoals.begin(), mGoals.end(), item.second) != mGoals.end())
			{
				mNonGoaledBaggages.erase(item.first.first);
				mGoaledBaggages.emplace(item.first.first, item.second);
			}
			// 普通の床 -> 普通の床の場合
			// 移動前が未ゴール荷物のリストに存在し、移動後がゴールリストに存在しない
			else if (mNonGoaledBaggages.count(item.first.first) && std::find(mGoals.begin(), mGoals.end(), item.second) == mGoals.end())
			{
				mNonGoaledBaggages[item.first.first] = item.second;
			}
			// ゴール -> ゴールの場合
			// 移動前がゴール済み荷物のリストに存在し、移動後がゴールリストに存在する
			else if (mGoaledBaggages.count(item.first.first) && std::find(mGoals.begin(), mGoals.end(), item.second) != mGoals.end())
			{
				mGoaledBaggages[item.first.first] = item.second;
			}

		}
		// プレイヤーと荷物の位置の変更を盤面データに反映させる
		UpdateBoardStr();

		// 履歴に現在のプレイヤーと荷物の位置の組み合わせがあるか調べ、無ければ履歴に追加
		// 既にあるならば、履歴から取得する
		if (!mHistory.count(std::pair<std::pair<int, int>, std::set<std::pair<int, int>>>{ nextPlayerPosPair, nextBaggagesPairSet }))
		{
			mBoardStates.mMovableArea = GetMovableArea();
			mBoardStates.mDeadlockedBaggages = UpdateFreezeBaggagesMap();
			mBoardStates.mGoaledBaggages = mGoaledBaggages;
			mBoardStates.mNonGoaledBaggages = mNonGoaledBaggages;
			mHistory.emplace(std::pair<std::pair<int, int>, std::set<std::pair<int, int>>>{ nextPlayerPosPair, nextBaggagesPairSet }, mBoardStates);
		}
		else;
		{
			mBoardStates = mHistory[std::pair<std::pair<int, int>, std::set<std::pair<int, int>>>{ nextPlayerPosPair, nextBaggagesPairSet }];
		}
	}
	// 荷物が移動していない場合
	// 2.1, 2.2	プレイヤーが移動しているかで分岐
	else if (mPlayerPos != nextPlayerPos)
	{
		// プレイヤーのいる床エリアが変わったかで分岐
		// 2.1	変わっていない場合
		if (isMovableArea(mPlayerPos, nextPlayerPos))
		{
			// メンバ変数の更新
			mPlayerPos = nextPlayerPos;
			// プレイヤーと荷物の位置の変更を盤面データに反映させる
			UpdateBoardStr();

			// 荷物が移動しておらず、プレイヤーも床エリアが変わっていないため、盤面構造 (mBoardStates) もそのまま
			// ただし、プレイヤーは移動しているので、履歴には追加する
			// 履歴に現在のプレイヤーと荷物の位置の組み合わせがあるか調べ、無ければ履歴に追加
			// 既にあるならば、履歴から取得する
			if (!mHistory.count(std::pair<std::pair<int, int>, std::set<std::pair<int, int>>>{ nextPlayerPosPair, nextBaggagesPairSet }))
			{
				mHistory.emplace(std::pair<std::pair<int, int>, std::set<std::pair<int, int>>>{ nextPlayerPosPair, nextBaggagesPairSet }, mBoardStates);
			}
			else
			{
				mBoardStates = mHistory[std::pair<std::pair<int, int>, std::set<std::pair<int, int>>>{ nextPlayerPosPair, nextBaggagesPairSet }];
			}

		}
		// 2.2	変わった場合
		else
		{
			// メンバ変数の更新
			mPlayerPos = nextPlayerPos;
			// プレイヤーと荷物の位置の変更を盤面データに反映させる
			UpdateBoardStr();

			// プレイヤーの床エリアが変わったので、盤面構造 (mBoardStates) を更新して履歴に追加
			// 履歴に現在のプレイヤーと荷物の位置の組み合わせがあるか調べ、無ければ履歴に追加
			// 既にあるならば、履歴から取得する
			if (!mHistory.count(std::pair<std::pair<int, int>, std::set<std::pair<int, int>>>{ nextPlayerPosPair, nextBaggagesPairSet }))
			{
				mBoardStates.mMovableArea = GetMovableArea();
				mBoardStates.mDeadlockedBaggages = UpdateFreezeBaggagesMap();
				mBoardStates.mGoaledBaggages = mGoaledBaggages;
				mBoardStates.mNonGoaledBaggages = mNonGoaledBaggages;
				mHistory.emplace(std::pair<std::pair<int, int>, std::set<std::pair<int, int>>>{ nextPlayerPosPair, nextBaggagesPairSet }, mBoardStates);
			}
			else
			{
				mBoardStates = mHistory[std::pair<std::pair<int, int>, std::set<std::pair<int, int>>>{ nextPlayerPosPair, nextBaggagesPairSet }];
			}
		}
	}
}

bool HUDHelper::isDeadlockedBaggage(const sf::Vector2i& pos)
{
	for (const auto& baggage : mBoardStates.mDeadlockedBaggages)
	{
		if (baggage.second == pos)
		{
			return true;
		}
	}

	return false;
}

HUDHelper::Direction HUDHelper::RollDirection()
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

sf::Vector2i HUDHelper::RollCoordinate()
{
	return sf::Vector2i{ RollCoordinateX(), RollCoordinateY() };
}

sf::Vector2i HUDHelper::RollCoordinate(const int& xMin, const int& xMax, const int& yMin, const int& yMax)
{
	return sf::Vector2i{ RollCoordinateX(xMin, xMax), RollCoordinateY(yMin, yMax) };
}

int HUDHelper::RollCoordinateX()
{
	std::uniform_int_distribution<int> randX(1, mSize.x - 2);
	// 乱数を生成する 
	return randX(mt);
}

int HUDHelper::RollCoordinateX(const int& xMin, const int& xMax)
{
	std::uniform_int_distribution<int> randX(xMin, xMax);
	// 乱数を生成する 
	return randX(mt);
}

int HUDHelper::RollCoordinateY()
{
	std::uniform_int_distribution<int> randX(1, mSize.y - 2);
	// 乱数を生成する 
	return randX(mt);
}

int HUDHelper::RollCoordinateY(const int& yMin, const int& yMax)
{
	std::uniform_int_distribution<int> randX(yMin, yMax);
	// 乱数を生成する 
	return randX(mt);
}

template <typename T>
T HUDHelper::GetRandomElement(const std::vector<T>& vec)
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
std::pair<T, U> HUDHelper::GetRandomElement(const std::unordered_map<T, U>& map)
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

std::vector<sf::Vector2i> HUDHelper::GetRandomDirections()
{
	std::vector<sf::Vector2i> output = directions;
	std::shuffle(output.begin(), output.end(), mt);
	return output;
}

bool HUDHelper::isArrangementValid()
{
	// 各々の荷物が詰みかどうか確認する
	for (const auto baggagePos : mBaggagesPos)
	{
		if (isFreezeBaggage(baggagePos.second))
		{
			return false;
		}
	}

	return true;
}

std::unordered_map<int, sf::Vector2i> HUDHelper::UpdateFreezeBaggagesMap()
{
	std::unordered_map<int, sf::Vector2i> result{};

	// 詰みの荷物をリストに追加していく
	for (const auto baggagePos : mBaggagesPos)
	{
		if (isFreezeBaggage(baggagePos.second))
		{
			result.emplace(baggagePos.first, baggagePos.second);
		}
	}

	return result;
}

std::vector<sf::Vector2i> HUDHelper::GetCandidates()
{
	// 押すことのできる荷物の候補を取得
	std::vector<sf::Vector2i> result{};
	for (const auto& baggage : mBaggagesPos)
	{
		std::set<std::pair<int, int>> targetPosSet{};

		// 荷物の上下と左右に移動の余裕があるかを調べる
		if ((mBoardStr[baggage.second.y][baggage.second.x + 1] == ' ' ||
			mBoardStr[baggage.second.y][baggage.second.x + 1] == '.' ||
			mBoardStr[baggage.second.y][baggage.second.x + 1] == '@' ||
			mBoardStr[baggage.second.y][baggage.second.x + 1] == '+') &&
			(mBoardStr[baggage.second.y][baggage.second.x - 1] == ' ' ||
			mBoardStr[baggage.second.y][baggage.second.x - 1] == '.' ||
			mBoardStr[baggage.second.y][baggage.second.x - 1] == '@' ||
			mBoardStr[baggage.second.y][baggage.second.x - 1] == '+'))
		{
			targetPosSet.emplace(std::pair<int, int>{ baggage.second.y, baggage.second.x + 1 });
			targetPosSet.emplace(std::pair<int, int>{ baggage.second.y, baggage.second.x - 1 });
		}
		if ((mBoardStr[baggage.second.y + 1][baggage.second.x] == ' ' ||
			mBoardStr[baggage.second.y + 1][baggage.second.x] == '.' ||
			mBoardStr[baggage.second.y + 1][baggage.second.x] == '@' ||
			mBoardStr[baggage.second.y + 1][baggage.second.x] == '+') &&
			(mBoardStr[baggage.second.y - 1][baggage.second.x] == ' ' ||
			mBoardStr[baggage.second.y - 1][baggage.second.x] == '.' ||
			mBoardStr[baggage.second.y - 1][baggage.second.x] == '@' ||
			mBoardStr[baggage.second.y - 1][baggage.second.x] == '+'))
		{
			targetPosSet.emplace(std::pair<int, int>{ baggage.second.y + 1, baggage.second.x });
			targetPosSet.emplace(std::pair<int, int>{ baggage.second.y - 1, baggage.second.x });
		}

		if (targetPosSet.empty())
		{
			continue;
		}

		std::vector<std::vector<bool>> explored(mSize.y, std::vector<bool>(mSize.x, false));
		explored[mPlayerPos.y][mPlayerPos.x] = true;

		std::queue<std::pair<int, int>> q{};
		q.push(std::pair<int, int>{ mPlayerPos.x, mPlayerPos.y });

		while (!q.empty())
		{
			std::pair<int, int> current{ q.front() };
			q.pop();

			if (targetPosSet.count(current))
			{
				result.emplace_back(baggage.second);
				break;
			}

			for (const auto& dir : directions)
			{
				std::pair<int, int> next{ current.first + dir.x, current.second + dir.y };

				if (!explored[next.second][next.first])
				{
					if (mBoardStr[next.second][next.first] == ' ' || mBoardStr[next.second][next.first] == '.' || mBoardStr[next.second][next.first] == '@' || mBoardStr[next.second][next.first] == '+')
					{
						explored[next.second][next.first] = true;
						q.push(next);
					}
				}
			}
		}
	}
	
	return result;
}

bool HUDHelper::isFreezeBaggage(const sf::Vector2i& baggagePos)
{
	// あくまでこの荷物自身が詰みかどうかを判別する関数であり、荷物全体の配置から詰み判定を行うわけではない
	// 荷物の周囲の地形から詰み判定を行う処理と、荷物がゴールに移動できるかどうかで詰み判定を行う処理の2ステップからなる

	// ゴール位置にある荷物は詰みではない
	if (std::find(mGoals.begin(), mGoals.end(), baggagePos) != mGoals.end())
	{
		// ここでfalseを返すのは、荷物がゴールに到達できる以上、詰みであると断定できないから
		// 詰みでないことを保証するものではない
		// NOTE よって、詰み判定の処理の変更に伴い、ここの処理を変える可能性はある
		return false;
	}

	// 盤面の角の荷物は動かせない
	if (baggagePos == sf::Vector2i{ 1, 1 } && baggagePos == sf::Vector2i{ mSize.x, 1 } || baggagePos == sf::Vector2i{ 1, mSize.y } || baggagePos == sf::Vector2i{ mSize.x, mSize.y })
	{
		return true;
	}

	// 荷物の周囲の地形を見て、動かせるかどうかを調べる
	// TODO 荷物を動かせない形状の中に、1つでもゴール状態でない荷物が紛れていたらOUT
	for (int i = 0; i < directions.size(); i++)
	{
		// $$ ## #$ #$ $#
		// $$ $$ $# $$ ## の形状
		if ((mBoardStr[baggagePos.y + directions[i].y][baggagePos.x + directions[i].x] == '#' || mBoardStr[baggagePos.y + directions[i].y][baggagePos.x + directions[i].x] == '$') &&
			(mBoardStr[baggagePos.y + directions[i].y + directions[(i + 1) % 4].y][baggagePos.x + directions[i].x + directions[(i + 1) % 4].x] == '#' || mBoardStr[baggagePos.y + directions[i].y + directions[(i + 1) % 4].y][baggagePos.x + directions[i].x + directions[(i + 1) % 4].x] == '$') &&
			(mBoardStr[baggagePos.y + directions[(i + 1) % 4].y][baggagePos.x + directions[(i + 1) % 4].x] == '#' || mBoardStr[baggagePos.y + directions[(i + 1) % 4].y][baggagePos.x + directions[(i + 1) % 4].x] == '$'))
		{
			return true;
		}

		// $#
		// #  の形状
		if (mBoardStr[baggagePos.y + directions[i].y][baggagePos.x + directions[i].x] == '#' &&
			mBoardStr[baggagePos.y + directions[(i + 1) % 4].y][baggagePos.x + directions[(i + 1) % 4].x] == '#')
		{
			return true;
		}

		//  $#
		// #$  の形状
		if (mBoardStr[baggagePos.y + directions[i].y][baggagePos.x + directions[i].x] == '#' &&
			mBoardStr[baggagePos.y + directions[(i + 1) % 4].y][baggagePos.x + directions[(i + 1) % 4].x] == '$' &&
			mBoardStr[baggagePos.y + directions[(i + 1) % 4].y + directions[(i + 2) % 4].y][baggagePos.x + directions[(i + 1) % 4].x + directions[(i + 2) % 4].x] == '#')
		{
			return true;
		}

		// #$
		//  $# の形状
		if (mBoardStr[baggagePos.y + directions[i].y + directions[(i + 1) % 4].y][baggagePos.x + directions[i].x + directions[(i + 1) % 4].x] == '#' &&
			mBoardStr[baggagePos.y + directions[(i + 1) % 4].y][baggagePos.x + directions[(i + 1) % 4].x] == '$' &&
			mBoardStr[baggagePos.y + directions[(i + 2) % 4].y][baggagePos.x + directions[(i + 2) % 4].x] == '#')
		{
			return true;
		}

		// #
		// $$
		//  $# の形状
		if (mBoardStr[baggagePos.y + directions[i].y + directions[(i + 1) % 4].y][baggagePos.x + directions[i].x + directions[(i + 1) % 4].x] == '#' &&
			mBoardStr[baggagePos.y + directions[(i + 1) % 4].y][baggagePos.x + directions[(i + 1) % 4].x] == '$' &&
			mBoardStr[baggagePos.y + directions[(i + 2) % 4].y][baggagePos.x + directions[(i + 2) % 4].x] == '$' &&
			mBoardStr[baggagePos.y + directions[(i + 2) % 4].y + directions[(i + 3) % 4].y][baggagePos.x + directions[(i + 2) % 4].x + directions[(i + 3) % 4].x] == '#')
		{
			return true;
		}
	}

	// 荷物の移動可能な範囲内にゴールがあるか調べる
	// ゴールに到達できないのならば詰みで、そうでなければ詰みかどうかはわからない
	// 探索用の盤面 (荷物とプレイヤーを取り除いた盤面を用いる)
	bool nonExistsRoute = true;
	Board currentBoard = mBoardMapStr;
	std::set<std::pair<int, int>> targetPosSet{};

	// 荷物の上下と左右に移動の余裕があるかを調べる
	// その場所にプレイヤーが移動できるとは限らない
	if ((currentBoard[baggagePos.y][baggagePos.x + 1] == ' ' ||
		currentBoard[baggagePos.y][baggagePos.x + 1] == '.' ||
		currentBoard[baggagePos.y][baggagePos.x + 1] == '@' ||
		currentBoard[baggagePos.y][baggagePos.x + 1] == '+') &&
		(currentBoard[baggagePos.y][baggagePos.x - 1] == ' ' ||
		currentBoard[baggagePos.y][baggagePos.x - 1] == '.' ||
		currentBoard[baggagePos.y][baggagePos.x - 1] == '@' ||
		currentBoard[baggagePos.y][baggagePos.x - 1] == '+'))
	{
		targetPosSet.emplace(std::pair<int, int>{ baggagePos.x + 1, baggagePos.y });
		targetPosSet.emplace(std::pair<int, int>{ baggagePos.x - 1, baggagePos.y });
	}
	if ((currentBoard[baggagePos.y + 1][baggagePos.x] == ' ' ||
		currentBoard[baggagePos.y + 1][baggagePos.x] == '.' ||
		currentBoard[baggagePos.y + 1][baggagePos.x] == '@' ||
		currentBoard[baggagePos.y + 1][baggagePos.x] == '+') &&
		(currentBoard[baggagePos.y - 1][baggagePos.x] == ' ' ||
		currentBoard[baggagePos.y - 1][baggagePos.x] == '.' ||
		currentBoard[baggagePos.y - 1][baggagePos.x] == '@' ||
		currentBoard[baggagePos.y - 1][baggagePos.x] == '+'))
	{
		targetPosSet.emplace(std::pair<int, int>{ baggagePos.x, baggagePos.y + 1 });
		targetPosSet.emplace(std::pair<int, int>{ baggagePos.x, baggagePos.y - 1 });
	}

	// 荷物の上下もしくは左右に移動可能な余裕があれば続行
	if (!targetPosSet.empty())
	{
		// 探索用の盤面にプレイヤーと荷物を設置する
		currentBoard[mPlayerPos.y][mPlayerPos.x] = '@';
		currentBoard[baggagePos.y][baggagePos.x] = '$';

		// 荷物の運搬をシミュレートするためのプレイヤーの初期位置を取得する
		// 具体的には、荷物の運搬開始位置にプレイヤーが移動できるかを調べる
		bool existsRoute = false;
		for (const auto& targetPos : targetPosSet)
		{
			if (isMovableArea(mPlayerPos, sf::Vector2i{ targetPos.first, targetPos.second }, currentBoard))
			{
				existsRoute = true;
				break;
			}
		}

		// 荷物の上下左右の余裕にプレイヤーが移動可能ならば続行
		if (existsRoute)
		{
			// 方向を加味した探索履歴
			// ex. explored[4][7][1] = true なら、(7,4)の位置にある荷物を下方向に動かす動きを既にしたことを表す
			std::vector<std::vector<std::vector<bool>>> explored(mSize.y, std::vector<std::vector<bool>>(mSize.x, std::vector<bool>(directions.size(), false)));
			
			// 幅優先探索に用いるキューの要素
			struct CurrentState
			{
				Board mBoard;
				sf::Vector2i mPlayer;
				sf::Vector2i mBaggage;

				// 比較演算子の定義
				bool operator<(const CurrentState& other) const
				{
					if (mPlayer.x < other.mPlayer.x) return true;
					if (mPlayer.y < other.mPlayer.y) return true;
					if (other.mPlayer.x < mPlayer.x) return false;
					if (other.mPlayer.y < mPlayer.y) return false;
					if (mBaggage.x < other.mBaggage.x) return true;
					if (mBaggage.y < other.mBaggage.y) return true;
					if (other.mBaggage.x < mBaggage.x) return false;
					return mBaggage.y < other.mBaggage.y;
				}
			};

			std::queue<CurrentState> q{};
			q.push(CurrentState{ currentBoard, mPlayerPos, baggagePos });

			while (!q.empty())
			{
				CurrentState current = q.front();
				q.pop();

				if (std::find(mGoals.begin(), mGoals.end(), current.mBaggage) != mGoals.end())
				{
					// ここでfalseを返すのは、荷物がゴールに到達できる以上、詰みであると断定できないから
					// 詰みでないことを保証するものではない
					// NOTE よって、詰み判定の処理の変更に伴い、ここの処理を変える可能性はある
					nonExistsRoute = false;
				}

				for (const auto& dir : directions)
				{
					sf::Vector2i nextBaggagePos{ current.mBaggage + dir };
					if (!explored[current.mBaggage.y][current.mBaggage.x][GetDirectionIndex(dir)])
					{
						// 荷物の次の移動先が壁マスでなく、かつプレイヤーが運搬開始位置に移動できるのならば、キューに追加
						if (current.mBoard[nextBaggagePos.y][nextBaggagePos.x] != '#')
						{
							if (isMovableArea(current.mPlayer, current.mBaggage - dir, current.mBoard))
							{
								Board nextBoard{ current.mBoard };
								nextBoard[current.mPlayer.y][current.mPlayer.x] = ' ';
								nextBoard[current.mBaggage.y][current.mBaggage.x] = '@';
								nextBoard[nextBaggagePos.y][nextBaggagePos.x] = '$';
								CurrentState next{ nextBoard, current.mBaggage, nextBaggagePos };

								explored[current.mBaggage.y][current.mBaggage.x][GetDirectionIndex(dir)] = true;
								q.push(next);
							}
						}
					}
				}
			}
		}
	}

	if (nonExistsRoute)
	{
		return true;
	}

	return false;
}

bool HUDHelper::isMovableArea(const sf::Vector2i& pos1, const sf::Vector2i& pos2)
{
	std::vector<std::vector<bool>> explored(mSize.y, std::vector<bool>(mSize.x, false));
	explored[pos1.y][pos1.x] = true;

	std::queue<std::pair<int, int>> q{};
	q.push(std::pair<int, int>{ pos1.x, pos1.y });

	while (!q.empty())
	{
		std::pair<int, int> current{ q.front() };
		q.pop();

		if (current == std::pair<int, int>{ pos2.x, pos2.y })
		{
			return true;
		}

		for (const auto& dir : directions)
		{
			std::pair<int, int> next{ current.first + dir.x, current.second + dir.y };

			if (!explored[next.second][next.first])
			{
				if (mBoardStr[next.second][next.first] == ' ' || mBoardStr[next.second][next.first] == '.' || mBoardStr[next.second][next.first] == '@' || mBoardStr[next.second][next.first] == '+')
				{
					explored[next.second][next.first] = true;
					q.push(next);
				}
			}
		}
	}

	return false;
}

bool HUDHelper::isMovableArea(const sf::Vector2i& pos1, const sf::Vector2i& pos2, const Board& board)
{
	std::vector<std::vector<bool>> explored(mSize.y, std::vector<bool>(mSize.x, false));
	explored[pos1.y][pos1.x] = true;

	std::queue<std::pair<int, int>> q{};
	q.push(std::pair<int, int>{ pos1.x, pos1.y });

	while (!q.empty())
	{
		std::pair<int, int> current{ q.front() };
		q.pop();

		if (current == std::pair<int, int>{ pos2.x, pos2.y })
		{
			return true;
		}

		for (const auto& dir : directions)
		{
			std::pair<int, int> next{ current.first + dir.x, current.second + dir.y };

			if (!explored[next.second][next.first])
			{
				if (board[next.second][next.first] == ' ' || board[next.second][next.first] == '.' || board[next.second][next.first] == '@' || board[next.second][next.first] == '+')
				{
					explored[next.second][next.first] = true;
					q.push(next);
				}
			}
		}
	}

	return false;
}

std::vector<sf::Vector2i> HUDHelper::GetMovableArea()
{
	std::vector<sf::Vector2i> result{ mPlayerPos };
	std::vector<std::vector<bool>> explored(mSize.y, std::vector<bool>(mSize.x, false));
	explored[mPlayerPos.y][mPlayerPos.x] = true;

	std::queue<std::pair<int, int>> q{};
	q.push(std::pair<int, int>{ mPlayerPos.x, mPlayerPos.y });

	while (!q.empty())
	{
		std::pair<int, int> current{ q.front() };
		q.pop();

		for (const auto& dir : directions)
		{
			std::pair<int, int> next{ current.first + dir.x, current.second + dir.y };

			if (!explored[next.second][next.first])
			{
				if (mBoardStr[next.second][next.first] == ' ' || mBoardStr[next.second][next.first] == '.' || mBoardStr[next.second][next.first] == '@' || mBoardStr[next.second][next.first] == '+')
				{
					explored[next.second][next.first] = true;
					q.push(next);
					result.emplace_back(sf::Vector2i{ next.first, next.second });
				}
			}
		}
	}

	return result;
}

void HUDHelper::UpdateBoardStr()
{
	// 一旦盤面上のプレイヤーと荷物を削除
	mBoardStr = mBoardMapStr;

	// 既に更新されているプレイヤーと荷物の位置に配置
	if (std::find(mGoals.begin(), mGoals.end(), mPlayerPos) == mGoals.end())
	{
		mBoardStr[mPlayerPos.y][mPlayerPos.x] = '@';
	}
	else
	{
		mBoardStr[mPlayerPos.y][mPlayerPos.x] = '+';
	}
	for (const auto& mBaggagePos : mGoaledBaggages)
	{
		mBoardStr[mBaggagePos.second.y][mBaggagePos.second.x] = '*';
	}
	for (const auto& mBaggagePos : mNonGoaledBaggages)
	{
		mBoardStr[mBaggagePos.second.y][mBaggagePos.second.x] = '$';
	}
	
}