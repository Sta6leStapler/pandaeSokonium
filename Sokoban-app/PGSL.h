#pragma once

#include "SFML/Graphics.hpp"
#include <string>
#include <vector>

class PGSL
{
public:
	PGSL(const sf::Vector2i& size, const int& baggageNum);
private:

	sf::Vector2i size;	// sizeは3*3領域を敷き詰める数
	std::vector<std::string> mBoardStr;	// 盤面の情報を表す
	int mBaggageNum;	// 荷物の数
	std::vector<std::vector<std::string>> templates;	// 盤面の構成に用いるテンプレートのリスト
};

