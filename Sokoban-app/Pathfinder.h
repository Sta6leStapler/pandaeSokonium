#pragma once
#include <SFML/System/Vector2.hpp>
#include <vector>
#include <string>
#include <map>
#include <set>
#include <queue>

class Pathfinder
{
public:
    // メモ: このクラスでは経路を表す際、次に移動するマスの座標のリストで表現する。
    // つまり、startからendまでの経路を表すリストには、startの次のマスからendのマスまでの座標のリストが格納される。

    // startからendまでの最短経路を返す (経路がなければ空)
    static std::vector<sf::Vector2i> FindPath(
        const sf::Vector2i& start,
        const sf::Vector2i& end,
        const std::vector<std::string>& boardState,
        const std::vector<sf::Vector2i>& baggagePositions,
        const sf::Vector2i& boardSize);

    // startからプレイヤーが移動可能なすべてのマスを返す
    static std::vector<sf::Vector2i> FindAllReachable(
        const sf::Vector2i& start,
        const std::vector<std::string>& boardState,
        const std::vector<sf::Vector2i>& baggagePositions,
        const sf::Vector2i& boardSize);

    // startの位置の荷物を運搬可能なすべてのマスを返す
    static std::vector<sf::Vector2i> FindAllTransportable(
        const sf::Vector2i& start,
        const std::vector<std::string>& boardState,
        const sf::Vector2i& playerPosition,
        const std::vector<sf::Vector2i>& baggagePositions,
        const sf::Vector2i& boardSize);

    static bool isValidPath(
        const sf::Vector2i& start,
        const std::vector<sf::Vector2i>& path,
        const std::vector<std::string>& boardState,
        const std::vector<sf::Vector2i>& baggagePositions,
        const sf::Vector2i& boardSize
    );

    static std::vector<sf::Vector2i> FindTransportPath(
        const sf::Vector2i& baggageStart,
        const sf::Vector2i& baggageEnd,
        const sf::Vector2i& playerStart,
        const std::vector<std::string>& boardState,
        const std::vector<sf::Vector2i>& initialBaggagePositions,
        const sf::Vector2i& boardSize
    );

    static std::map<int, std::vector<sf::Vector2i>> FindAllDirectionTransportPath(
        const sf::Vector2i& baggageStart,
        const sf::Vector2i& baggageEnd,
        const sf::Vector2i& playerStart,
        const std::vector<std::string>& boardState,
        const std::vector<sf::Vector2i>& initialBaggagePositions,
        const sf::Vector2i& boardSize
    );
};