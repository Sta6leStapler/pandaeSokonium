#include "Pathfinder.h"

// sf::Vector2iを比較するための構造体
struct CompareVector2i {
    bool operator()(const sf::Vector2i& a, const sf::Vector2i& b) const {
        // y座標が異なる場合はy座標で比較
        if (a.y != b.y) {
            return a.y < b.y;
        }
        // y座標が同じ場合はx座標で比較
        return a.x < b.x;
    }
};

std::vector<sf::Vector2i> Pathfinder::FindPath(
    const sf::Vector2i& start,
    const sf::Vector2i& end,
    const std::vector<std::string>& boardState,
    const std::vector<sf::Vector2i>& baggagePositions,
    const sf::Vector2i& boardSize)
{
	// 幅優先探索でstartからendまでの最短経路を見つける
	std::vector<sf::Vector2i> result{};

	// 上下左右の4方向
    const std::vector<sf::Vector2i> directions{
        { 1, 0 },
        { 0, 1 },
        { -1, 0 },
        { 0, -1 }
	};

    // 幅優先探索用の探索済みマス
    std::vector<std::vector<bool>> explored(boardSize.y, std::vector<bool>(boardSize.x, false));
	explored[start.y][start.x] = true;

    // 幅優先探索用のキュー
	std::queue<sf::Vector2i> q{};
    q.push(start);

    // 各マスへの最短経路を記録するマップ
    std::map<sf::Vector2i, sf::Vector2i, bool(*)(const sf::Vector2i&, const sf::Vector2i&)> cameFrom(
        [](const sf::Vector2i& a, const sf::Vector2i& b) {
            return (a.x == b.x) ? (a.y < b.y) : (a.x < b.x);
        });
    cameFrom[start] = start;

    // 探索処理
    bool found = false;
    while (!q.empty())
    {
        auto current = q.front();
        q.pop();
        if (current == end)
        {
            found = true;
            break;
        }
        for (const auto& direction : directions)
        {
            auto next = current + direction;
            if (!explored[next.y][next.x] &&
                boardState[next.y][next.x] != '#' && // 壁でない
                std::find(baggagePositions.begin(), baggagePositions.end(), next) == baggagePositions.end()) // 荷物がない
            {
                explored[next.y][next.x] = true;
                cameFrom[next] = current;
                q.push(next);
            }
        }
    }

    // 経路の復元
    if (found)
    {
        sf::Vector2i current = end;
        while (current != start)
        {
            result.emplace_back(current);
            current = cameFrom[current];
        }
        std::reverse(result.begin(), result.end());
    }

	return result;
}

std::vector<sf::Vector2i> Pathfinder::FindAllReachable(
    const sf::Vector2i& start,
    const std::vector<std::string>& boardState,
    const std::vector<sf::Vector2i>& baggagePositions,
    const sf::Vector2i& boardSize)
{
    std::vector<sf::Vector2i> result{};

	// 上下左右の4方向
    const std::vector<sf::Vector2i> directions{
        { 1, 0 },
        { 0, 1 },
        { -1, 0 },
        { 0, -1 }
    };

    // 幅優先探索用の探索済みマス
    std::vector<std::vector<bool>> explored(boardSize.y, std::vector<bool>(boardSize.x, false));
    explored[start.y][start.x] = true;

	// 幅優先探索用のキュー
    std::queue<sf::Vector2i> q{};
    q.push(start);

    // 探索処理
    while (!q.empty())
    {
		auto current = q.front();
		q.pop();

        for (const auto& direction : directions)
        {
			auto next = current + direction;
            if (!explored[next.y][next.x] &&
                (boardState[next.y][next.x] == ' ' || boardState[next.y][next.x] == '.') &&
                std::find(baggagePositions.begin(), baggagePositions.end(), next) == baggagePositions.end())
            {
				explored[next.y][next.x] = true;
                result.emplace_back(next);
				q.push(next);
            }
        }
    }

    return result;
}

std::vector<sf::Vector2i> Pathfinder::FindAllTransportable(
    const sf::Vector2i& start,
    const std::vector<std::string>& boardState,
    const sf::Vector2i& playerPosition,
    const std::vector<sf::Vector2i>& baggagePositions,
    const sf::Vector2i& boardSize)
{
    // 上下左右の4方向
    const std::vector<sf::Vector2i> directions
    {
        { 0, -1 }, // 0: 上
        { 1,  0 }, // 1: 右
        { 0,  1 }, // 2: 下
        { -1, 0 }  // 3: 左
    };

    // ダイクストラ法で用いる状態を管理する構造体
    struct State
    {
        int cost;                 // その状態に至るまでの総コスト（プレイヤーの移動距離）
        sf::Vector2i baggagePos;  // 荷物の現在位置
        int pushedFromDirIndex;   // どの方向から押されてこの位置に来たか

        // 優先度付きキューでコストが小さい順に取り出すための比較演算子
        bool operator>(const State& other) const
        {
            return cost > other.cost;
        }
    };

    // 優先度付きキュー（最小ヒープ）
    std::priority_queue<State, std::vector<State>, std::greater<State>> pq;

    // 各マスにどの向きから押されたかを記録する (y, x, direction_index)
    std::vector<std::vector<std::vector<bool>>> explored(
        boardSize.y, std::vector<std::vector<bool>>(
            boardSize.x, std::vector<bool>(directions.size(), false)
        )
    );

    // 運搬対象外の荷物の位置を高速に参照できるようsetに格納
    std::set<sf::Vector2i, CompareVector2i> otherBaggages;
    for (const auto& pos : baggagePositions)
    {
        if (pos != start)
        {
            otherBaggages.insert(pos);
        }
    }

    // 探索の開始点をキューに追加
    // 最初の4方向へのプッシュを試みる
    for (int i = 0; i < directions.size(); ++i)
    {
        const auto& pushDir = directions[i];
        sf::Vector2i baggageNextPos = start + pushDir;
        sf::Vector2i playerStandPos = start - pushDir;

        // 荷物の移動先とプレイヤーの立ち位置が有効かチェック
        if (playerStandPos.x < 0 || playerStandPos.y < 0 || playerStandPos.x >= boardSize.x || playerStandPos.y >= boardSize.y ||
            baggageNextPos.x < 0 || baggageNextPos.y < 0 || baggageNextPos.x >= boardSize.x || baggageNextPos.y >= boardSize.y ||
            boardState[playerStandPos.y][playerStandPos.x] == '#' ||
            boardState[baggageNextPos.y][baggageNextPos.x] == '#' ||
            otherBaggages.count(baggageNextPos))
        {
            continue;
        }

        // プレイヤーが荷物を押す位置まで移動する経路を探す
        // この時、障害物となるのは他の荷物のみ
        std::vector<sf::Vector2i> initialPath = FindPath(
            playerPosition, playerStandPos, boardState,
            baggagePositions,
            boardSize
        );

        if (!initialPath.empty() || playerPosition == playerStandPos)
        {
            // パスが見つかれば、最初の状態としてキューに追加
            int initialCost = static_cast<int>(initialPath.size()) > 1 ? static_cast<int>(initialPath.size()) - 1 : 0;
            pq.push({ initialCost, baggageNextPos, i });
        }
    }

    // ダイクストラ法による探索
    std::vector<std::vector<bool>> isReachable(boardSize.y, std::vector<bool>(boardSize.x, false));

    while (!pq.empty())
    {
        State current = pq.top();
        pq.pop();

        // すでにこの状態（位置、押された方向）が探索済みならスキップ
        if (explored[current.baggagePos.y][current.baggagePos.x][current.pushedFromDirIndex])
        {
            continue;
        }
        // 探索済みの印をつける
        explored[current.baggagePos.y][current.baggagePos.x][current.pushedFromDirIndex] = true;
        isReachable[current.baggagePos.y][current.baggagePos.x] = true;

        // 荷物を押した後、プレイヤーは荷物があった場所に移動する
        sf::Vector2i playerCurrentPos = current.baggagePos - directions[current.pushedFromDirIndex];

        // 現在の荷物位置から、さらに4方向へのプッシュを試みる
        for (int i = 0; i < directions.size(); ++i)
        {
            const auto& nextPushDir = directions[i];
            sf::Vector2i nextBaggagePos = current.baggagePos + nextPushDir;
            sf::Vector2i nextPlayerStandPos = current.baggagePos - nextPushDir;

            // 先に、試みようとしている移動先が有効かチェック
            // プレイヤーおよび荷物の移動先が盤面の範囲外であったり、壁マスや別の荷物があるマスでなければOK
            if (nextPlayerStandPos.x < 0 || nextPlayerStandPos.y < 0 || nextPlayerStandPos.x >= boardSize.x || nextPlayerStandPos.y >= boardSize.y ||
                nextBaggagePos.x < 0 || nextBaggagePos.y < 0 || nextBaggagePos.x >= boardSize.x || nextBaggagePos.y >= boardSize.y ||
                boardState[nextPlayerStandPos.y][nextPlayerStandPos.x] == '#' ||
                boardState[nextBaggagePos.y][nextBaggagePos.x] == '#' ||
                otherBaggages.count(nextBaggagePos))
            {
                continue;
            }

            // プレイヤーが次のプッシュ位置まで移動する経路を探す
            // この時、現在動かしている荷物(current.baggagePos)も障害物になる
            std::vector<sf::Vector2i> baggageObstacles(otherBaggages.begin(), otherBaggages.end());
            baggageObstacles.emplace_back(current.baggagePos);

            std::vector<sf::Vector2i> path = FindPath(
                playerCurrentPos, nextPlayerStandPos, boardState, baggageObstacles, boardSize
            );

            // パスが存在した場合、コストを更新してキューにプッシュ
            if (!path.empty() || playerCurrentPos == nextPlayerStandPos)
            {
                int moveCost = static_cast<int>(path.size()) > 1 ? static_cast<int>(path.size()) - 1 : 0;
                int newTotalCost = current.cost + moveCost;
                pq.push({ newTotalCost, nextBaggagePos, i });
            }
        }
    }

    // --- 4. 結果の生成 ---
    std::vector<sf::Vector2i> result{};
    for (int y = 0; y < boardSize.y; ++y)
    {
        for (int x = 0; x < boardSize.x; ++x)
        {
            if (isReachable[y][x])
            {
                result.emplace_back(sf::Vector2i{ x, y });
            }
        }
    }

    return result;
}

bool Pathfinder::isValidPath(
    const sf::Vector2i& start,
    const std::vector<sf::Vector2i>& path,
    const std::vector<std::string>& boardState,
    const std::vector<sf::Vector2i>& baggagePositions,
    const sf::Vector2i& boardSize
)
{
    // プレイヤーと荷物の現在地
    sf::Vector2i currentPlayerPosition(start);
    std::vector<sf::Vector2i> currentBaggagePositions(baggagePositions);
    
    // 経路に沿って順に判定
    for (const auto& next : path)
    {
        // まず、移動先が盤面内かつ、壁でないかどうかを判定する
        if (next.x >= 0 && next.x < boardSize.x && next.y >= 0 && next.y < boardSize.y &&
            boardState[next.y][next.x] != '#')
        {
            auto iter = std::find(currentBaggagePositions.begin(), currentBaggagePositions.end(), next);
            
            // 荷物がある場合は、荷物を押しだした先が妥当かどうかの判定もする
            if (iter != currentBaggagePositions.end())
            {
                sf::Vector2i nextBaggagePos = *iter + (*iter - currentPlayerPosition);
                
                if (nextBaggagePos.x < 0 || nextBaggagePos.x >= boardSize.x ||
                    nextBaggagePos.y < 0 || nextBaggagePos.y >= boardSize.y ||
                    boardState[nextBaggagePos.y][nextBaggagePos.x] == '#')
                {
                    return false;
                }
                *iter = nextBaggagePos;
            }

            currentPlayerPosition = next;
        }
        else
        {
            return false;
        }
    }

    return true;
}

std::vector<sf::Vector2i> Pathfinder::FindTransportPath(
    const sf::Vector2i& baggageStart,
    const sf::Vector2i& baggageEnd,
    const sf::Vector2i& playerStart,
    const std::vector<std::string>& boardState,
    const std::vector<sf::Vector2i>& initialBaggagePositions,
    const sf::Vector2i& boardSize)
{
    // --- 1. 初期設定 ---
    const std::vector<sf::Vector2i> directions
    {
        { 0, -1 }, { 1, 0 }, { 0, 1 }, { -1, 0 }
    };

    // ダイクストラ法で用いる状態ノード
    struct Node
    {
        int cost;
        sf::Vector2i baggagePos;
        int pushedFromDirIndex;

        bool operator>(const Node& other) const { return cost > other.cost; }
    };

    std::priority_queue<Node, std::vector<Node>, std::greater<Node>> pq;

    // 経路復元のために、どの状態から来たかを記録するマップ
    // Key: {荷物位置, 押された方向}, Value: 親ノードの状態
    std::map<sf::Vector2i, std::map<int, Node>, CompareVector2i> cameFrom;

    // 各状態への最小コストを記録するマップ
    std::map<sf::Vector2i, std::map<int, int>, CompareVector2i> costSoFar;

    // 運搬対象外の荷物
    std::set<sf::Vector2i, CompareVector2i> otherBaggages;
    for (const auto& pos : initialBaggagePositions)
    {
        if (pos != baggageStart) {
            otherBaggages.insert(pos);
        }
    }

    // --- 2. 探索の開始点をキューに追加 ---
    for (int i = 0; i < directions.size(); ++i)
    {
        const auto& pushDir = directions[i];
        sf::Vector2i baggageNextPos = baggageStart + pushDir;
        sf::Vector2i playerStandPos = baggageStart - pushDir;

        // 最初のプッシュが可能か検証
        if (playerStandPos.x < 0 || playerStandPos.y < 0 || /* ...境界等のチェック... */
            boardState[playerStandPos.y][playerStandPos.x] == '#' ||
            boardState[baggageNextPos.y][baggageNextPos.x] == '#' ||
            otherBaggages.count(baggageNextPos))
        {
            continue;
        }

        std::vector<sf::Vector2i> baggageObs(otherBaggages.begin(), otherBaggages.end());
        std::vector<sf::Vector2i> path = FindPath(playerStart, playerStandPos, boardState, initialBaggagePositions, boardSize);

        if (!path.empty() || playerStart == playerStandPos)
        {
            int initialCost = path.empty() ? 0 : static_cast<int>(path.size());
            pq.push({ initialCost, baggageNextPos, i });
            costSoFar[baggageNextPos][i] = initialCost;
            // 親ノードは開始位置として記録
            cameFrom[baggageNextPos][i] = { 0, baggageStart, -1 };
        }
    }

    // --- 3. ダイクストラ法による探索 ---
    Node goalNode = { -1, {-1, -1}, -1 };
    while (!pq.empty())
    {
        Node current = pq.top();
        pq.pop();

        if (current.cost > costSoFar[current.baggagePos][current.pushedFromDirIndex])
        {
            continue; // よりコストの大きい経路なのでスキップ
        }

        // ゴールに到達したら終了
        if (current.baggagePos == baggageEnd)
        {
            goalNode = current;
            break;
        }

        sf::Vector2i playerCurrentPos = current.baggagePos - directions[current.pushedFromDirIndex];

        // 次の4方向へのプッシュを試みる
        for (int i = 0; i < directions.size(); ++i)
        {
            sf::Vector2i nextBaggagePos = current.baggagePos + directions[i];
            sf::Vector2i nextPlayerStandPos = current.baggagePos - directions[i];

            // ... 移動先の検証 ...
            if (/* ...境界等のチェック... */
                boardState[nextPlayerStandPos.y][nextPlayerStandPos.x] == '#' ||
                boardState[nextBaggagePos.y][nextBaggagePos.x] == '#' ||
                otherBaggages.count(nextBaggagePos))
            {
                continue;
            }

            std::vector<sf::Vector2i> baggageObs(otherBaggages.begin(), otherBaggages.end());
            baggageObs.emplace_back(current.baggagePos); // 今の荷物位置も障害物

            std::vector<sf::Vector2i> path = FindPath(playerCurrentPos, nextPlayerStandPos, boardState, baggageObs, boardSize);

            if (!path.empty() || playerCurrentPos == nextPlayerStandPos)
            {
                int moveCost = path.empty() ? 0 : static_cast<int>(path.size());
                int newTotalCost = current.cost + moveCost + 1; // +1は荷物を押すコスト

                if (costSoFar[nextBaggagePos].find(i) == costSoFar[nextBaggagePos].end() || newTotalCost < costSoFar[nextBaggagePos][i])
                {
                    costSoFar[nextBaggagePos][i] = newTotalCost;
                    pq.push({ newTotalCost, nextBaggagePos, i });
                    cameFrom[nextBaggagePos][i] = current;
                }
            }
        }
    }

    // --- 4. 経路の復元 ---
    if (goalNode.cost == -1)
    {
        return {}; // ゴールに到達できなかった
    }

    std::vector<sf::Vector2i> playerPathResult{};
    std::vector<Node> baggagePath;
    Node current = goalNode;

    // cameFromを逆に辿って荷物の移動経路を作成
    while (current.pushedFromDirIndex != -1)
    {
        baggagePath.emplace_back(current);
        current = cameFrom[current.baggagePos][current.pushedFromDirIndex];
    }
    std::reverse(baggagePath.begin(), baggagePath.end());

    // 荷物の移動経路からプレイヤーの完全な移動経路を生成
    sf::Vector2i currentPlayerPos = playerStart;
    sf::Vector2i lastBaggagePos = baggageStart;

    for (const auto& move : baggagePath)
    {
        sf::Vector2i playerStandPos = lastBaggagePos - directions[move.pushedFromDirIndex];

        std::vector<sf::Vector2i> baggageObs(otherBaggages.begin(), otherBaggages.end());
        baggageObs.emplace_back(lastBaggagePos);

        std::vector<sf::Vector2i> segment = FindPath(currentPlayerPos, playerStandPos, boardState, baggageObs, boardSize);

        playerPathResult.insert(playerPathResult.end(), segment.begin(), segment.end());
        playerPathResult.emplace_back(lastBaggagePos); // 荷物を押す動き

        currentPlayerPos = lastBaggagePos;
        lastBaggagePos = move.baggagePos;
    }

    return playerPathResult;
}

std::map<int, std::vector<sf::Vector2i>> Pathfinder::FindAllDirectionTransportPath(
    const sf::Vector2i& baggageStart,
    const sf::Vector2i& baggageEnd,
    const sf::Vector2i& playerStart,
    const std::vector<std::string>& boardState,
    const std::vector<sf::Vector2i>& initialBaggagePositions,
    const sf::Vector2i& boardSize)
{
    // --- 1. 初期設定 ---
    // 荷物を押す方向ベクトルとキーの定義
    const std::vector<sf::Vector2i> directions{
        { -1,  0 }, // 0: 荷物が左へ移動 (プレイヤーは右から押す)
        {  0, -1 }, // 1: 荷物が上へ移動 (プレイヤーは下から押す)
        {  1,  0 }, // 2: 荷物が右へ移動 (プレイヤーは左から押す)
        {  0,  1 }  // 3: 荷物が下へ移動 (プレイヤーは上から押す)
    };

    // ダイクストラ法で用いる状態ノード
    struct Node {
        int cost;
        sf::Vector2i baggagePos;
        int pushedFromDirIndex;

        bool operator>(const Node& other) const {
            return cost > other.cost;
        }
    };

    // 優先度付きキューと各種記録用マップ
    std::priority_queue<Node, std::vector<Node>, std::greater<Node>> pq;
    std::map<sf::Vector2i, std::map<int, Node>, CompareVector2i> cameFrom;
    std::map<sf::Vector2i, std::map<int, int>, CompareVector2i> costSoFar;

    // explored[y][x][direction] が true の場合、その状態は探索済み
    std::vector<std::vector<std::vector<bool>>> explored(
        boardSize.y, std::vector<std::vector<bool>>(
            boardSize.x, std::vector<bool>(directions.size(), false)
        )
    );

    // 運搬対象外の荷物
    std::set<sf::Vector2i, CompareVector2i> otherBaggages;
    for (const auto& pos : initialBaggagePositions) {
        if (pos != baggageStart) {
            otherBaggages.insert(pos);
        }
    }

    // --- 2. 探索の開始点をキューに追加 ---
    for (int i = 0; i < directions.size(); ++i) {
        const auto& pushDir = directions[i];
        sf::Vector2i playerStandPos = baggageStart - pushDir;
        sf::Vector2i baggageNextPos = baggageStart + pushDir;

        if (playerStandPos.x < 0 || playerStandPos.y < 0 || playerStandPos.x >= boardSize.x || playerStandPos.y >= boardSize.y ||
            baggageNextPos.x < 0 || baggageNextPos.y < 0 || baggageNextPos.x >= boardSize.x || baggageNextPos.y >= boardSize.y ||
            boardState[playerStandPos.y][playerStandPos.x] == '#' ||
            boardState[baggageNextPos.y][baggageNextPos.x] == '#' ||
            otherBaggages.count(baggageNextPos)) {
            continue;
        }

        std::vector<sf::Vector2i> path = FindPath(playerStart, playerStandPos, boardState, initialBaggagePositions, boardSize);

        if (!path.empty() || playerStart == playerStandPos) {
            int initialCost = static_cast<int>(path.size()) + 1;
            pq.push({ initialCost, baggageNextPos, i });
            costSoFar[baggageNextPos][i] = initialCost;
            cameFrom[baggageNextPos][i] = { 0, baggageStart, -1 };
        }
    }

    // --- 3. ダイクストラ法による探索 ---
    std::map<int, Node> goalNodes;

    while (!pq.empty()) {
        Node current = pq.top();
        pq.pop();

        // この状態（荷物位置、押された方向）が探索済みならスキップ
        if (explored[current.baggagePos.y][current.baggagePos.x][current.pushedFromDirIndex]) {
            continue;
        }
        // これからこの状態を処理するので、探索済みの印をつける
        explored[current.baggagePos.y][current.baggagePos.x][current.pushedFromDirIndex] = true;


        // ゴールに到達した場合
        if (current.baggagePos == baggageEnd) {
            if (goalNodes.find(current.pushedFromDirIndex) == goalNodes.end() || current.cost < goalNodes[current.pushedFromDirIndex].cost) {
                goalNodes[current.pushedFromDirIndex] = current;
            }
        }

        sf::Vector2i playerCurrentPos = current.baggagePos - directions[current.pushedFromDirIndex];

        for (int i = 0; i < directions.size(); ++i) {
            const auto& nextPushDir = directions[i];
            sf::Vector2i nextBaggagePos = current.baggagePos + nextPushDir;
            sf::Vector2i nextPlayerStandPos = current.baggagePos - nextPushDir;

            if (nextPlayerStandPos.x < 0 || nextPlayerStandPos.y < 0 || nextPlayerStandPos.x >= boardSize.x || nextPlayerStandPos.y >= boardSize.y ||
                nextBaggagePos.x < 0 || nextBaggagePos.y < 0 || nextBaggagePos.x >= boardSize.x || nextBaggagePos.y >= boardSize.y ||
                boardState[nextPlayerStandPos.y][nextPlayerStandPos.x] == '#' ||
                boardState[nextBaggagePos.y][nextBaggagePos.x] == '#' ||
                otherBaggages.count(nextBaggagePos)) {
                continue;
            }

            std::vector<sf::Vector2i> baggageObstacles(otherBaggages.begin(), otherBaggages.end());
            baggageObstacles.emplace_back(current.baggagePos);

            std::vector<sf::Vector2i> path = FindPath(playerCurrentPos, nextPlayerStandPos, boardState, baggageObstacles, boardSize);

            if (!path.empty() || playerCurrentPos == nextPlayerStandPos) {
                int moveCost = static_cast<int>(path.size());
                int newTotalCost = current.cost + moveCost + 1;

                if (!costSoFar.count(nextBaggagePos) || !costSoFar[nextBaggagePos].count(i) || newTotalCost < costSoFar[nextBaggagePos][i]) {
                    costSoFar[nextBaggagePos][i] = newTotalCost;
                    pq.push({ newTotalCost, nextBaggagePos, i });
                    cameFrom[nextBaggagePos][i] = current;
                }
            }
        }
    }

    // --- 4. 経路の復元 ---
    std::map<int, std::vector<sf::Vector2i>> resultPaths;
    if (goalNodes.empty()) {
        return {};
    }

    for (auto const& [dirIndex, goalNode] : goalNodes) {
        std::vector<sf::Vector2i> playerPathResult;
        std::vector<Node> baggagePath;
        Node current = goalNode;

        while (current.pushedFromDirIndex != -1) {
            baggagePath.emplace_back(current);
            current = cameFrom[current.baggagePos][current.pushedFromDirIndex];
        }
        std::reverse(baggagePath.begin(), baggagePath.end());

        sf::Vector2i currentPlayerPos = playerStart;
        sf::Vector2i lastBaggagePos = baggageStart;

        for (const auto& move : baggagePath) {
            sf::Vector2i playerStandPos = lastBaggagePos - directions[move.pushedFromDirIndex];
            std::vector<sf::Vector2i> currentObstacles(otherBaggages.begin(), otherBaggages.end());
            currentObstacles.emplace_back(lastBaggagePos);
            std::vector<sf::Vector2i> segment = FindPath(currentPlayerPos, playerStandPos, boardState, currentObstacles, boardSize);
            playerPathResult.insert(playerPathResult.end(), segment.begin(), segment.end());
            playerPathResult.emplace_back(lastBaggagePos);
            currentPlayerPos = lastBaggagePos;
            lastBaggagePos = move.baggagePos;
        }
        resultPaths[dirIndex] = playerPathResult;
    }

    return resultPaths;
}