#pragma once

#include "SFML/Graphics.hpp"
#include <string>
#include <vector>

#include "PGSL.h"
#include "MCTS.h"
#include "MySolution.h"

class PuzzleGenerator
{
public:
	enum TYPE
	{
		EPGSL,
		EMCTS,
		EMySolution
	};

	PuzzleGenerator(const sf::Vector2i& size, const int& baggageNum);
	virtual ~PuzzleGenerator() {}

	std::vector<std::string> GetBoard(const TYPE& type);

private:
	void pgsl();
	void mcts();
	void mysolution(const int& buildsNum, const int& runsNum);
	
	std::vector<std::string> mBoardStr;
	sf::Vector2i mSize;
	int mBaggageNum;	// â◊ï®ÇÃêî
};

