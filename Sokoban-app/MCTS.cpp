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
	// �f�o�C�X���痐�����擾����
	std::random_device rd;
	// �����̃V�[�h��ݒ肷��
	std::mt19937 mt(rd());

	std::uniform_int_distribution<int> initX(1, mSize.x - 2), initY(1, mSize.y - 2);
	// �v���C���[�̏����ʒu�������_���Ɍ���
	sf::Vector2i initP(initX(mt), initY(mt));
	mBoardStr[initP.y][initP.x] = '@';

	// *���^�C���ɗאڂ���ǃ^�C�������^�C���ɂ���
	// *���^�C���ɏ��^�C���̂����ꂩ�ɉו���z�u����
	// �̓�����J��Ԃ�
	
	std::uniform_int_distribution<int> randLoops(mBaggageNum * 3, (mSize.x - 2) * (mSize.y - 2) - 1), randSteps(1, 100);
	// �J��Ԃ���
	int numLoops = randLoops(mt);

	// ���^�C���̍��W�̃��X�g
	std::vector<sf::Vector2i> floors = { initP };
	// �ו��̒u���Ă�����W�̃��X�g
	std::vector<sf::Vector2i> baggages;

	for (int i = 0; i < numLoops; ++i)
	{

		// �ו��̐������^�C���̐������ŁA���ו��̔z�u���I������Ă��āA���ו��̐�������ɒB���Ă��Ȃ��ꍇ
		if (floors.size() > baggages.size() + 1 && randSteps(mt) < randomRange01 && baggages.size() < mBaggageNum)
		{
			// �v���C���[�̏����ʒu�ȊO�̏��^�C�����烉���_���ɑI�сA�ו���z�u
			std::uniform_int_distribution<int> randIndex(1, floors.size() - 1);
			int index = randIndex(mt);
			baggages.emplace_back(sf::Vector2i{ floors[index].x, floors[index].y });
			mBoardStr[floors[index].y][floors[index].x] = '$';
		}
		// ���̊g��
		else
		{
			// ���^�C���������_���ɑI�����A����4�}�X�ɕǃ^�C����1�ȏ゠��΂��̕ǃ^�C���̒������I��
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
	// �f�o�C�X���痐�����擾����
	std::random_device rd;
	// �����̃V�[�h��ݒ肷��
	std::mt19937 mt(rd());
	std::uniform_int_distribution<int> randLoops(700, randomRange02);
	// �J��Ԃ���
	int numLoops = randLoops(mt);

	// �e��ϐ�
	std::vector<std::string> simField = mBoardStr;	// �V�~�����[�V�������̔Ֆ�
	sf::Vector2i pPoint = GetCoordinates('@', simField).front();	// ���z�v���C���[�̍��W
	std::vector<std::pair<sf::Vector2i, int>> baggagesPushCount;	// �ו��̌��݂̍��W�Ɖ����ꂽ��
	std::vector<sf::Vector2i> baggagesInit = GetCoordinates('$', simField);	// �ו��̍ŏ��̍��W
	std::vector<std::vector<bool>> walkedTiles(mSize.y, std::vector<bool>(mSize.x, true)); // ���z�v���C���[�Ɖו������񂾍��W
	walkedTiles[pPoint.y][pPoint.x] = false;
	for (const auto& item : baggagesInit)
	{
		walkedTiles[item.y][item.x] = false;
	}

	// baggagesInit�̏����l��ݒ�
	for (const auto& item : baggagesInit)
	{
		baggagesPushCount.emplace_back(std::make_pair(item, 0));
	}

	// �ŏ��Ɍ��߂��J��Ԃ��񐔂������z�v���C���[�̈ړ��������J��Ԃ�
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

	// 2��ȏ㉟���ꂽ�ו��̍ŏI�n�_���S�[���n�_�ɕϊ�����
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

	// �㏈��
	// *��x���͋[�v���C���[�ɉ�����Ȃ������ו��͕ǂɕϊ�
	// *��x�����͋[�v���C���[�ɉ�����Ȃ������ו��̈ړ������͑S�ď��^�C���ɕϊ�
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

	// ���z�v���C���[���ו������܂Ȃ������^�C���͂��ׂĕǂɕϊ�
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
	// �f�o�C�X���痐�����擾����
	std::random_device rd;
	// �����̃V�[�h��ݒ肷��
	std::mt19937 mt(rd());
	std::uniform_int_distribution<int> rand(0, 3);
	// �����𐶐����� 
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
	// �f�o�C�X���痐�����擾����
	std::random_device rd;
	// �����̃V�[�h��ݒ肷��
	std::mt19937 mt(rd());
	std::uniform_int_distribution<int> randX(0, mSize.x - 1);
	// �����𐶐����� 
	return randX(mt);
}

int MCTS::RollCoordinateY()
{
	// �f�o�C�X���痐�����擾����
	std::random_device rd;
	// �����̃V�[�h��ݒ肷��
	std::mt19937 mt(rd());
	std::uniform_int_distribution<int> randX(0, mSize.y - 1);
	// �����𐶐����� 
	return randX(mt);
}

std::vector<sf::Vector2i> MCTS::GetAroundWalls(const sf::Vector2i& point)
{
	std::vector<sf::Vector2i> result;

	// ��E�����̏��ɒ��ׂ�
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