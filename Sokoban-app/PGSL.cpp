#include "PGSL.h"
#include <iostream>
#include <fstream>

PGSL::PGSL(const sf::Vector2i& size, const int& baggageNum)
	: size(size)
	, mBaggageNum(baggageNum)
{
	for (int i = 0; i < size.y * 3; i++)
	{
		std::string line = "";
		for (int j = 0; j < size.x * 3; j++)
		{
			line += "E";
		}
		mBoardStr.push_back(line);
	}

	std::string filename = "Assets/templates.txt";
	std::ifstream file(filename);
	if (!file.is_open())
	{
		std::cerr << "Failed to open file" << std::endl;
		return;
	}

	std::vector<std::string> lines;
	std::string line;
	while (std::getline(file, line))
	{
		lines.push_back(line);
	}
}