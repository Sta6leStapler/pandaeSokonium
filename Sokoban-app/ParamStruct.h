#pragma once
#include <string>
#include <cmath>
#include <SFML/System/Vector2.hpp>

// 盤面生成アルゴリズムに渡すパラメータをまとめた構造体
struct GenerationParameters {
    sf::Vector2i boardSize;     // 盤面の幅(x)と高さ(y)
    int baggageNum;             // 荷物の数 (b)
    int resetCount;             // 荷物とプレイヤーの配置リセット回数 (c)
    int transportCount;         // 荷物の運搬(通路を壁に変える)回数 (n)
    float wallRate;            // 初期壁マスの割合
    float visitedRate;         // 訪問済みタイルの割合
    int evalFuncIndex;          // 評価関数の種類

    // 現在のパラメータから計算量の相対的なスコアを算出する
    // [計算式] O(b * c * n * (xy * log(xy))^2)
    float calculateComplexityScore() const {
        float xy = static_cast<float>(boardSize.x * boardSize.y);
        if (xy <= 0) return 0;

        float b = static_cast<float>(baggageNum);
        float c = static_cast<float>(resetCount);
        float n = static_cast<float>(transportCount);
        float logXY = std::log2(xy);

        // (xy * log(xy))^2 の項が支配的
        return b * c * n * std::powf(xy * logXY, 2);
    }
};

// 各パラメータの制約や説明を定義するメタ情報
struct ParameterMeta {
    std::string key;          // 内部識別子
    std::string name;         // 表示名
    std::string description;  // ツールチップ用説明文
    float min;               // 最小値
    float max;               // 最大値
    bool isInteger;           // 整数として扱うか（ステップ値の制御用）
};