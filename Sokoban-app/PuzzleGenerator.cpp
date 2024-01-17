#include "PuzzleGenerator.h"
#include <iostream>
#include <fstream>

PuzzleGenerator::PuzzleGenerator(const sf::Vector2i& size, const int& baggageNum)
	:mSize(size)
	,mBaggageNum(baggageNum)
{

}

std::vector<std::string> PuzzleGenerator::GetBoard(const TYPE& type)
{
	switch (type)
	{
	case EPGSL:
		pgsl();
		break;
	case EMCTS:
		mcts();
		break;
	case EMySolution:
		mysolution(2, 10);
		break;
	default:
		break;
	}

	return mBoardStr;
}

void PuzzleGenerator::pgsl()
{
	
}

void PuzzleGenerator::mcts()
{
	MCTS* gen = new MCTS(mSize, mBaggageNum);
	mBoardStr = gen->GetBoard();
	delete(gen);
}

void PuzzleGenerator::mysolution(const int& buildsNum, const int& runsNum)
{
	MySolution* gen = new MySolution(mSize, mBaggageNum, buildsNum, runsNum, 0.0);
	mBoardStr = gen->GetBoard();
	delete(gen);
}