#include "MySolution.h"
#include <queue>
#include <algorithm>

MySolution::MySolution(
	const sf::Vector2i& size,
	const int& baggageNum,
	const int& buildsNum,
	const int& runsNum,
	const double& initialWallRatio,
	const double& visitedRatio,
	const int& strategyIndex)
	:mSize(size)
	,mBaggageNum(baggageNum)
	,mt(rd())
	,mBuildsNum(buildsNum)
	,mRunsNum(runsNum)
	,mInitialWallRatio(initialWallRatio)
	,mVisitedRatio(visitedRatio)
	,mStrategyIndex(strategyIndex)
	,mSolveUpper(0)
{
	if (mInitialWallRatio < 0.0)
	{
		mInitialWallRatio = 0.0;
	}
	else if (mInitialWallRatio > 1.0)
	{
		mInitialWallRatio = 1.0;
	}

	if (mVisitedRatio < 0.0)
	{
		mVisitedRatio = 0.0;
	}
	else if (mVisitedRatio > 1.0)
	{
		mVisitedRatio = 1.0;
	}

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
	// �ǃ^�C���̔z�u�������߂�
	int wallCount = ((mSize.x - 2) * (mSize.y - 2) - mBaggageNum - 1) * mInitialWallRatio;
	SetWallTiles(wallCount);

	// �Ֆʏ��̃R�s�[
	Board cpBoard = mBoardStr;
	std::vector<sf::Vector2i> cpBaggagePos;

	// �w�肳�ꂽ�������ו���z�u
	while (mBaggagesCoordinate.size() < mBaggageNum)
	{
		// ����ȏ�ו���z�u�ł��Ȃ��ꍇ�A�S�Ẳו����폜���Ă�蒼��
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

		// �z�u��̍��W�ɔ�肪�Ȃ����Ƃ��m�F
		if (mBoardStr[tmpCoordinate.y][tmpCoordinate.x] != '#' && std::find(mBaggagesCoordinate.begin(), mBaggagesCoordinate.end(), tmpCoordinate) == mBaggagesCoordinate.end())
		{
			// �l�݂̉ו��̔z�u���Ȃ����Ƃ��m�F����
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

	// �v���C���[�̏����ʒu�������_���Ɍ���
	sf::Vector2i initP{ -1, -1 };
	do
	{
		initP = RollCoordinate();
	} while (mBoardStr[initP.y][initP.x] == '#' || mBoardStr[initP.y][initP.x] == '$');
	mBoardStr[initP.y][initP.x] = '@';
}

void MySolution::RunSimulation()
{
	// �Ֆʏ��̃R�s�[
	Board cpBoard = mBoardStr;

	// ���z�ו��̌��ݒn
	std::vector<sf::Vector2i> vbPoints;

	// �V�~�����[�g�����s
	for (int count = 0; count < mBuildsNum; count++)
	{
		// �Ֆʏ�ňړ������񐔂��J�E���g����ϐ�
		// �v���C���[
		std::vector<std::vector<unsigned int>> PPassCount(mSize.y, std::vector<unsigned int>(mSize.x, 0));
		// �ו�
		std::vector<std::vector<unsigned int>> BPassCount(mSize.y, std::vector<unsigned int>(mSize.x, 0));

		// ���^�C�������炩���ߒ�߂�ꂽ���������K��ς݂ɂ���
		SetVisitedStatus(PPassCount, cpBoard);

		// �Ֆʏ��̃R�s�[
		cpBoard = mBoardStr;

		// ���z�v���C���[�̌��ݒn
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

		// ���z�ו��̌��ݒn
		vbPoints = mBaggagesCoordinate;

		// �v���C���[�̏����ʒu�Ɖו��̏����ʒu�̍��W�݈̂ړ������񐔂̃J�E���g�𑝂₷
		PPassCount[vpPoint.y][vpPoint.x]++;
		for (const auto& item : vbPoints)
		{
			BPassCount[item.y][item.x]++;
		}

		// �ו���1��ȏ�ړ����������ǂ����̃t���O
		std::vector<bool> isMoved(mBaggageNum, false);

		// ���̏�E�����Z�b�g
		mSolveUpper = 0;

		// �n�`�\�z�܂ł̔Ֆʂ̗���
		std::vector<History> currentHistory = { History{ cpBoard, PPassCount, BPassCount, mSolveUpper } };

		// ���O�ɉ^�񂾉ו��̃C���f�b�N�X
		int prevBaggageIndex = -1;

		// �v���C���[�Ɖו��̈ړ�
		for (int i = 0; i < mRunsNum || isLoopEnd(cpBoard, isMoved, PPassCount, BPassCount); i++)
		{
			// ������ו��Ɖ����n�߂���ʒu�̃��X�g�����߂�
			CandidateItems movableBaggages{};
			SetCandidates(vpPoint, vbPoints, cpBoard, movableBaggages, prevBaggageIndex);

			if (movableBaggages.empty())
			{
				break;
			}

			// �����ו��Ɖ����n�߂�ʒu�̃��X�g����A�v���C���[�̈ړ��o�H�����߂�
			std::unordered_map<int, std::vector<Route>> pRoutes{};
			SetRoutes(vpPoint, vbPoints, cpBoard, pRoutes, movableBaggages, PPassCount, BPassCount);

			if (pRoutes.empty())
			{
				break;
			}

			// �o�H��I������
			//auto bId = GetRandomElement(pRoutes).first;
			std::pair<int, Route> pRoute(GetBestRoute(pRoutes, cpBoard, vpPoint, vbPoints, PPassCount, BPassCount, i, movableBaggages));//(bid, GetRandomElement(pRoutes[bId]));
			isMoved[pRoute.first] = true;

			// ����^�ԉו��̃C���f�b�N�X���L�^
			prevBaggageIndex = pRoute.first;

			// �v���C���[�̈ړ��o�H�ɉ����ăv���C���[�Ɖו����ړ�������
			MoveOnPath(pRoute.second, cpBoard, vpPoint, vbPoints[pRoute.first], PPassCount, BPassCount, true);

			// ���̏�E�����߂邽�߂ɁA�o�H�̒����������Z
			mSolveUpper += pRoute.second.size();

			// �����ɏd�����Ȃ����m�F����
			if (existsDuplicateHistory(currentHistory, cpBoard))
			{
				currentHistory.emplace_back(History{ cpBoard, PPassCount, BPassCount, mSolveUpper });
			}
			// �d�����������ꍇ
			// �Ƃ肠�����A���݂͏d����������ו��̉^����1�O�̉ו��̉^���̍ŏI��Ԃ��o�͂Ƃ��ďI��
			else
			{
				cpBoard = currentHistory.back().mBoard;
				PPassCount = currentHistory.back().mPPass;
				BPassCount = currentHistory.back().mBPass;
				mSolveUpper = currentHistory.back().mSolveUpper;
				break;
			}
		}

		// �ړ������񐔂���ɁA�n�`���`��
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

		// �����ɒǉ�
		mBoardHistory.emplace_back(currentHistory);
	}

	// �S�[���ʒu�̐ݒ�
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


	// ���ʂȏ��^�C��������
	std::vector<sf::Vector2i> tileList;
	do
	{
		tileList.clear();
		for (int x = 1; x < mSize.x - 1; ++x)
		{
			for (int y = 1; y < mSize.y - 1; ++y)
			{
				if (mBoardStr[y][x] == ' ')
				{
					for (int i = 0; i < directions.size(); ++i)
					{
						if (mBoardStr[y + directions[i].y][x + directions[i].x] == '#' &&
							mBoardStr[y + directions[(i + 1) % 4].y][x + directions[(i + 1) % 4].x] == '#' &&
							mBoardStr[y + directions[(i + 2) % 4].y][x + directions[(i + 2) % 4].x] == '#')
						{
							tileList.emplace_back(sf::Vector2i{ x, y });
							mBoardStr[y][x] = '#';
							break;
						}
					}
				}
			}
		}
	} while (!tileList.empty());

	// �ו��ƃS�[���̑Ή������
	for (int i = 0; i < mBaggageNum; i++)
	{
		mInitToGoalList.emplace_back(std::make_pair(mBaggagesCoordinate[i], vbPoints[i]));
	}

	// ���̕\��
	std::cout << "Solution length : " << mSolveUpper << std::endl << "Solution : " << mSolutionDirection << std::endl;
}

void MySolution::SetWallTiles(const int& wallCount)
{
	// �Ֆʂ̃R�s�[
	Board cpBoard = mBoardStr;
	// �z�u���ꂽ�ǂ̐��̃J�E���g
	int count = 0;
	// �ǂ��K�萔�ɒB����܂Ŕz�u
	while (count < wallCount)
	{
		// �����_���ȍ��W���擾
		sf::Vector2i tmpPos = RollCoordinate();
		// �擾�������W�����^�C�����ǂ���
		if (cpBoard[tmpPos.y][tmpPos.x] == ' ')
		{
			// �ꎞ�I�ɕǂɏ���������
			cpBoard[tmpPos.y][tmpPos.x] = '#';

			// 3���ȏオ�ǂň͂܂�Ă��鏰�^�C�������݂��Ȃ����m�F
			if (hasEnclosedFloor(cpBoard))
			{
				cpBoard[tmpPos.y][tmpPos.x] = ' ';
				continue;
			}

			// �Ֆʂ�2�ȏ�ɕ��f����Ă��Ȃ����𔻒�
			if (isBoardPartitioned(cpBoard))
			{
				cpBoard[tmpPos.y][tmpPos.x] = ' ';
				continue;
			}

			// ������ɂ��Y�����Ȃ��ꍇ�́A�J�E���g�𑝂₷
			++count;
		}
	}
	mBoardStr = cpBoard;
}

bool MySolution::hasEnclosedFloor(const Board& board)
{
	// ���^�C���̎���3�����ȏオ�ǃ^�C���ň͂܂�Ă��邩�ǂ����̔�����s��
	for (int x = 1; x < mSize.x - 1; ++x)
	{
		for (int y = 1; y < mSize.y - 1; ++y)
		{
			// ���^�C���Ȃ璲�ׂ�
			if (board[y][x] == ' ')
			{
				// ����3�����𒲂ׂ�
				for (int i = 0; i < 4; ++i)
				{
					if (board[y + directions[i].y][x + directions[i].x] == '#' &&
						board[y + directions[(i + 1) % 4].y][x + directions[(i + 1) % 4].x] == '#' &&
						board[y + directions[(i + 2) % 4].y][x + directions[(i + 2) % 4].x] == '#'
						)
					{
						return true;
					}
				}
			}
		}
	}

	return false;
}

bool MySolution::isBoardPartitioned(const Board& board)
{
	// �Ֆʂ�2�ȏ�ɕ��f����Ă��Ȃ����𔻒�
	// ���D��T���Œ��ׂ�
	// ���T�����ǂ���
	std::vector<std::vector<bool>> isUnachieved(mSize.y, std::vector<bool>(mSize.x, true));
	std::queue<sf::Vector2i> q;
	// �K���ȏ��^�C����T��
	sf::Vector2i initPos{ -1, -1 };
	for (int y = 1; y < mSize.y - 1; ++y)
	{
		for (int x = 1; x < mSize.x - 1; ++x)
		{
			if (board[y][x] == ' ')
			{
				initPos = sf::Vector2i{ x, y };
				break;
			}
		}
		if (initPos.x != -1)
		{
			break;
		}
	}
	q.push(initPos);
	isUnachieved[initPos.y][initPos.x] = false;

	while (!q.empty())
	{
		sf::Vector2i current = q.front();
		q.pop();

		for (const auto& dir : directions)
		{
			sf::Vector2i next{ current.x + dir.x, current.y + dir.y };
			if (board[next.y][next.x] == ' ' && isUnachieved[next.y][next.x])
			{
				q.push(next);
				isUnachieved[next.y][next.x] = false;
			}
		}
	}
	
	// ���T���̏��^�C�����Ȃ����ǂ����m�F����
	for (int x = 1; x < mSize.x - 1; ++x)
	{
		for (int y = 1; y < mSize.y - 1; ++y)
		{
			// �������T���̃^�C��������ΐ^��Ԃ�
			if (board[y][x] == ' ' && isUnachieved[y][x])
			{
				return true;
			}
		}
	}

	return false;
}

void MySolution::SetVisitedStatus(std::vector<std::vector<unsigned int>>& visitedCount, const Board& board)
{
	// ���^�C���̃��X�g
	std::vector<sf::Vector2i> floors;
	for (int x = 0; x < mSize.x; ++x)
	{
		for (int y = 0; y < mSize.y; ++y)
		{
			if (board[y][x] == ' ' || board[y][x] == '@' || board[y][x] == '$' || board[y][x] == '*' || board[y][x] == '+')
			{
				floors.emplace_back(sf::Vector2i{ x, y });
			}
		}
	}

	int visitedAmount = static_cast<int>(floors.size() * mVisitedRatio);

	for (int i = 0; i < visitedAmount; ++i)
	{
		sf::Vector2i tmpPos = GetRandomElement(floors);
		++visitedCount[tmpPos.y][tmpPos.x];
		floors.erase(std::find(floors.begin(), floors.end(), tmpPos));
	}
}

bool MySolution::isLoopEnd(const Board& board, const std::vector<bool>& isMoved, const std::vector<std::vector<unsigned int>>& pPassCount, const std::vector<std::vector<unsigned int>>& bPassCount)
{
	// NOTE ����͂��̋@�\�͖�����
	// �S�Ẳו��𓮂����Ă���
	// �ו��̎���3�����ǂň͂܂�Ă��Ȃ�
	// TODO �����ʒu�œ������Ȃ��ו����Ȃ�
	// ��������������Ă���ꍇ�͏I���ł���

	return false;
	
	if (std::find(isMoved.begin(), isMoved.end(), false) == isMoved.end())
	{
		return true;
	}

	for (const auto& bPos : mBaggagesCoordinate)
	{
		for (int i = 0; i < 4; ++i)
		{
			// $$ ## #$ #$ $#
			// $$ $$ $# $$ ## �̌`��
			if (((pPassCount[bPos.y + directions[i].y][bPos.x + directions[i].x] == 0 &&
				bPassCount[bPos.y + directions[i].y][bPos.x + directions[i].x] == 0) ||
				mBoardStr[bPos.y + directions[i].y][bPos.x + directions[i].x] == '$' ||
				mBoardStr[bPos.y + directions[i].y][bPos.x + directions[i].x] == '*') &&
				((pPassCount[bPos.y + directions[i].y + directions[(i + 1) % 4].y][bPos.x + directions[i].x + directions[(i + 1) % 4].x] == 0 &&
				bPassCount[bPos.y + directions[i].y + directions[(i + 1) % 4].y][bPos.x + directions[i].x + directions[(i + 1) % 4].x] == 0) ||
				mBoardStr[bPos.y + directions[i].y + directions[(i + 1) % 4].y][bPos.x + directions[i].x + directions[(i + 1) % 4].x] == '$' ||
				mBoardStr[bPos.y + directions[i].y + directions[(i + 1) % 4].y][bPos.x + directions[i].x + directions[(i + 1) % 4].x] == '*') &&
				((pPassCount[bPos.y + directions[(i + 1) % 4].y][bPos.x + directions[(i + 1) % 4].x] == 0 &&
				bPassCount[bPos.y + directions[(i + 1) % 4].y][bPos.x + directions[(i + 1) % 4].x] == 0) ||
				mBoardStr[bPos.y + directions[(i + 1) % 4].y][bPos.x + directions[(i + 1) % 4].x] == '$' ||
				mBoardStr[bPos.y + directions[(i + 1) % 4].y][bPos.x + directions[(i + 1) % 4].x] == '*'))
			{
				return true;
			}

			// $#
			// #  �̌`��
			if ((pPassCount[bPos.y + directions[i].y][bPos.x + directions[i].x] == 0 &&
				bPassCount[bPos.y + directions[i].y][bPos.x + directions[i].x] == 0) &&
				(pPassCount[bPos.y + directions[(i + 1) % 4].y][bPos.x + directions[(i + 1) % 4].x] == 0 &&
				bPassCount[bPos.y + directions[(i + 1) % 4].y][bPos.x + directions[(i + 1) % 4].x] == 0))
			{
				return true;
			}

			//  $#
			// #$  �̌`��
			if ((pPassCount[bPos.y + directions[i].y][bPos.x + directions[i].x] == 0 &&
				bPassCount[bPos.y + directions[i].y][bPos.x + directions[i].x] == 0) &&
				(mBoardStr[bPos.y + directions[(i + 1) % 4].y][bPos.x + directions[(i + 1) % 4].x] == '$' ||
				mBoardStr[bPos.y + directions[(i + 1) % 4].y][bPos.x + directions[(i + 1) % 4].x] == '*') &&
				(pPassCount[bPos.y + directions[(i + 1) % 4].y + directions[(i + 2) % 4].y][bPos.x + directions[(i + 1) % 4].x + directions[(i + 2) % 4].x] == 0 &&
				bPassCount[bPos.y + directions[(i + 1) % 4].y + directions[(i + 2) % 4].y][bPos.x + directions[(i + 1) % 4].x + directions[(i + 2) % 4].x] == 0))
			{
				return true;
			}

			// #$
			//  $# �̌`��
			if ((pPassCount[bPos.y + directions[i].y + directions[(i + 1) % 4].y][bPos.x + directions[i].x + directions[(i + 1) % 4].x] == 0 &&
				bPassCount[bPos.y + directions[i].y + directions[(i + 1) % 4].y][bPos.x + directions[i].x + directions[(i + 1) % 4].x] == 0) &&
				(mBoardStr[bPos.y + directions[(i + 1) % 4].y][bPos.x + directions[(i + 1) % 4].x] == '$' ||
				mBoardStr[bPos.y + directions[(i + 1) % 4].y][bPos.x + directions[(i + 1) % 4].x] == '*') &&
				(pPassCount[bPos.y + directions[(i + 2) % 4].y][bPos.x + directions[(i + 2) % 4].x] == 0 &&
				bPassCount[bPos.y + directions[(i + 2) % 4].y][bPos.x + directions[(i + 2) % 4].x] == 0))
			{
				return true;
			}

			// #
			// $$
			//  $# �̌`��
			if ((pPassCount[bPos.y + directions[i].y + directions[(i + 1) % 4].y][bPos.x + directions[i].x + directions[(i + 1) % 4].x] == 0 &&
				bPassCount[bPos.y + directions[i].y + directions[(i + 1) % 4].y][bPos.x + directions[i].x + directions[(i + 1) % 4].x] == 0) &&
				(mBoardStr[bPos.y + directions[(i + 1) % 4].y][bPos.x + directions[(i + 1) % 4].x] == '$' ||
				mBoardStr[bPos.y + directions[(i + 1) % 4].y][bPos.x + directions[(i + 1) % 4].x] == '*') &&
				(mBoardStr[bPos.y + directions[(i + 2) % 4].y][bPos.x + directions[(i + 2) % 4].x] == '$' ||
				mBoardStr[bPos.y + directions[(i + 2) % 4].y][bPos.x + directions[(i + 2) % 4].x] == '*') &&
				(pPassCount[bPos.y + directions[(i + 2) % 4].y + directions[(i + 3) % 4].y][bPos.x + directions[(i + 2) % 4].x + directions[(i + 3) % 4].x] == 0 &&
				bPassCount[bPos.y + directions[(i + 2) % 4].y + directions[(i + 3) % 4].y][bPos.x + directions[(i + 2) % 4].x + directions[(i + 3) % 4].x] == 0)
				)
			{
				return true;
			}
		}
	}

	//*
	for (int x = 1; x < mSize.x - 1; x++)
	{
		for (int y = 1; y < mSize.y - 1; y++)
		{
			if (board[y][x] == '$' && (x > 1 && x < mSize.x - 1 || y > 1 && y < mSize.y - 1))
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
	//*/

	return false;
}

MySolution::Direction MySolution::RollDirection()
{
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
	// �����𐶐����� 
	return randX(mt);
}

int MySolution::RollCoordinateX(const int& xMin, const int& xMax)
{
	std::uniform_int_distribution<int> randX(xMin, xMax);
	// �����𐶐����� 
	return randX(mt);
}

int MySolution::RollCoordinateY()
{
	std::uniform_int_distribution<int> randX(1, mSize.y - 2);
	// �����𐶐����� 
	return randX(mt);
}

int MySolution::RollCoordinateY(const int& yMin, const int& yMax)
{
	std::uniform_int_distribution<int> randX(yMin, yMax);
	// �����𐶐����� 
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

std::vector<sf::Vector2i> MySolution::GetRandomDirections()
{
	std::vector<sf::Vector2i> output = directions;
	std::shuffle(output.begin(), output.end(), mt);
	return output;
}

std::vector<sf::Vector2i> MySolution::GetAroundWalls(const sf::Vector2i& point)
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

bool MySolution::isArrangementValid(const Board& board, const std::vector<sf::Vector2i>& bPositions)
{
	// �������Ȃ��ו������邩�ǂ����𔻒肷��
	// �Ֆʂ̊p�̉ו��͓������Ȃ�
	if (board[1][1] == '$' || board[board.size() - 2][1] == '$' || board[1][board[1].size() - 2] == '$' || board[board.size() - 2][board[1].size() - 2] == '$')
	{
		return false;
	}

	// �ו��̎��͂����āA�������邩�ǂ����𒲂ׂ�
	for (const auto& bPos : bPositions)
	{
		for (int i = 0; i < directions.size(); i++)
		{
			// $$ ## #$ #$ $#
			// $$ $$ $# $$ ## �̌`��
			if ((board[bPos.y + directions[i].y][bPos.x + directions[i].x] == '#' || board[bPos.y + directions[i].y][bPos.x + directions[i].x] == '$') &&
				(board[bPos.y + directions[i].y + directions[(i + 1) % 4].y][bPos.x + directions[i].x + directions[(i + 1) % 4].x] == '#' || board[bPos.y + directions[i].y + directions[(i + 1) % 4].y][bPos.x + directions[i].x + directions[(i + 1) % 4].x] == '$') &&
				(board[bPos.y + directions[(i + 1) % 4].y][bPos.x + directions[(i + 1) % 4].x] == '#' || board[bPos.y + directions[(i + 1) % 4].y][bPos.x + directions[(i + 1) % 4].x] == '$'))
			{
				return false;
			}

			// $#
			// #  �̌`��
			if (board[bPos.y + directions[i].y][bPos.x + directions[i].x] == '#' &&
				board[bPos.y + directions[(i + 1) % 4].y][bPos.x + directions[(i + 1) % 4].x] == '#')
			{
				return false;
			}

			//  $#
			// #$  �̌`��
			if (board[bPos.y + directions[i].y][bPos.x + directions[i].x] == '#' &&
				board[bPos.y + directions[(i + 1) % 4].y][bPos.x + directions[(i + 1) % 4].x] == '$' &&
				board[bPos.y + directions[(i + 1) % 4].y + directions[(i + 2) % 4].y][bPos.x + directions[(i + 1) % 4].x + directions[(i + 2) % 4].x] == '#')
			{
				return false;
			}

			// #$
			//  $# �̌`��
			if (board[bPos.y + directions[i].y + directions[(i + 1) % 4].y][bPos.x + directions[i].x + directions[(i + 1) % 4].x] == '#' &&
				board[bPos.y + directions[(i + 1) % 4].y][bPos.x + directions[(i + 1) % 4].x] == '$' &&
				board[bPos.y + directions[(i + 2) % 4].y][bPos.x + directions[(i + 2) % 4].x] == '#')
			{
				return false;
			}

			// #
			// $$
			//  $# �̌`��
			if (board[bPos.y + directions[i].y + directions[(i + 1) % 4].y][bPos.x + directions[i].x + directions[(i + 1) % 4].x] == '#' &&
				board[bPos.y + directions[(i + 1) % 4].y][bPos.x + directions[(i + 1) % 4].x] == '$' &&
				board[bPos.y + directions[(i + 2) % 4].y][bPos.x + directions[(i + 2) % 4].x] == '$' &&
				board[bPos.y + directions[(i + 2) % 4].y + directions[(i + 3) % 4].y][bPos.x + directions[(i + 2) % 4].x + directions[(i + 3) % 4].x] == '#')
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
				// 1�ł��z�u�ł���ꏊ������΋U���o��
				// flag�͔z�u�ł��Ȃ����Ƃ��m�F������U�ɂ��A���̈ʒu�𒲂ׂ邩�ǂ����𔻒�
				bool flag = true;
				for (int i = 0; i < 4; i++)
				{
					// �l�����ǂ�ו��ň͂܂�Ă��邩
					if ((board[y + directions[i].y][x + directions[i].x] == '$' || board[y + directions[i].y][x + directions[i].x] == '#') &&
						(board[y + directions[i].y + directions[(i + 1) % 4].y][x + directions[i].x + directions[(i + 1) % 4].x] == '$' || board[y + directions[i].y + directions[(i + 1) % 4].y][x + directions[i].x + directions[(i + 1) % 4].x] == '#') &&
						(board[y + directions[(i + 1) % 4].y][x + directions[(i + 1) % 4].x] == '$' || board[y + directions[(i + 1) % 4].y][x + directions[(i + 1) % 4].x] == '#'))
					{
						flag = false;
						break;
					}

					// �c���ɕǂ����邩
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

bool MySolution::existsDuplicateHistory(const std::vector<History> history, const Board& board)
{
	for (const auto& item : history)
	{
		if (item.mBoard == board)
		{
			return false;
		}
	}
	return true;
}

void MySolution::SetCandidates(const sf::Vector2i& pPos, const std::vector<sf::Vector2i>& bPositions, const Board& board, CandidateItems& outList, const int& prevBaggageIndex)
{
	// �ו����ƂɁA�v���C���[�������n�߂�ʒu�Ɉړ��ł�����W�̃��X�g�����߂�
	// �S�Ẳו���ǂƂ݂Ȃ����Ֆʂ��l����
	Board baseBoard = board;
	for (const auto& bPos : bPositions)
	{
		baseBoard[bPos.y][bPos.x] = '#';
	}

	// ���O�ɉ^�񂾉ו��ȊO�̑S�Ẳו��ɑ΂��Ē��ׂ�
	for (size_t i = 0; i < bPositions.size(); i++)
	{
		if (i != prevBaggageIndex)
		{
			// �Ώۂ̉ו��̍��W�݂̂��ו��ɖ߂�
			Board tmpBoard = baseBoard;
			tmpBoard[bPositions[i].y][bPositions[i].x] = '$';

			// �T���������ǂ�����ێ�����ϐ�
			std::vector<std::vector<bool>> unexplored(mSize.y, std::vector<bool>(mSize.x, true));
			// �ו��̍��W��T���ς݂ɂ���
			unexplored[pPos.y][pPos.x] = false;

			std::queue<sf::Vector2i> q;
			q.emplace(pPos);

			while (!q.empty())
			{
				const sf::Vector2i current = q.front();
				q.pop();

				// �T�����̌��ݒn�̎l�����𒲂ׂ�
				std::vector<sf::Vector2i> tmpDir = GetRandomDirections();
				for (const auto& dir : tmpDir)
				{
					sf::Vector2i next(current.x + dir.x, current.y + dir.y);

					// ���ݒ��ׂĂ�����������^�C���ŁA���T���Ȃ�T���𑱂���
					if (tmpBoard[next.y][next.x] == ' ' && unexplored[next.y][next.x])
					{
						q.emplace(next);
						unexplored[next.y][next.x] = false;
					}
					// ���ݒ��ׂĂ�������ɉו�������A���̕����ɉ������Ƃ��ł���̂Ȃ�ΊY���ו��̍��W�̃��X�g�ɒǉ�
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
}

void MySolution::SetCandidates(const sf::Vector2i& pPos, const std::vector<sf::Vector2i>& bPositions, const Board& board, std::vector<int>& outList)
{
	// �ו����ƂɁA�v���C���[�������n�߂�ʒu�Ɉړ��ł�����W�̃��X�g�����߂�
	// �S�Ẳו���ǂƂ݂Ȃ����Ֆʂ��l����
	Board baseBoard = board;
	for (const auto& bPos : bPositions)
	{
		baseBoard[bPos.y][bPos.x] = '#';
	}

	// �S�Ẳו��ɑ΂��Ē��ׂ�
	for (size_t i = 0; i < bPositions.size(); i++)
	{
		// �Ώۂ̉ו��̍��W�݂̂��ו��ɖ߂�
		Board tmpBoard = baseBoard;
		tmpBoard[bPositions[i].y][bPositions[i].x] = '$';

		// �T���������ǂ�����ێ�����ϐ�
		std::vector<std::vector<bool>> unexplored(mSize.y, std::vector<bool>(mSize.x, true));
		// �ו��̍��W��T���ς݂ɂ���
		unexplored[pPos.y][pPos.x] = false;

		std::queue<sf::Vector2i> q;
		q.emplace(pPos);

		while (!q.empty())
		{
			const sf::Vector2i current = q.front();
			q.pop();

			// �T�����̌��ݒn�̎l�����𒲂ׂ�
			std::vector<sf::Vector2i> tmpDir = GetRandomDirections();
			for (const auto& dir : tmpDir)
			{
				sf::Vector2i next(current.x + dir.x, current.y + dir.y);

				// ���ݒ��ׂĂ�����������^�C���ŁA���T���Ȃ�T���𑱂���
				if (tmpBoard[next.y][next.x] == ' ' && unexplored[next.y][next.x])
				{
					q.emplace(next);
					unexplored[next.y][next.x] = false;
				}
				// ���ݒ��ׂĂ�������ɉו�������A���̕����ɉ������Ƃ��ł���̂Ȃ�ΊY���ו��̍��W�̃��X�g�ɒǉ�
				else if (tmpBoard[next.y][next.x] == '$' && tmpBoard[next.y + dir.y][next.x + dir.x] == ' ' && next - dir == current)
				{
					if (std::find(outList.begin(), outList.end(), i) == outList.end())
					{
						outList.emplace_back(i);
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
	// �ו����Ƃɉ^�э��ނ��Ƃ��\�ȑS�Ă̍��W�ւ̌o�H�̃��X�g�����
	// �v���C���[���ו��ׂ̗܂ňړ�����i�K�ł͌o�H����炸�A�ו���1�}�X�������тɌo�H�Ƃ��ēo�^���Ă���
	// �ו����������Ƃ��ɁA��������̍��W�����ɍ��ꂽ�o�H�̒��ɂ���A�����̌o�H���^�э��ނ܂ł̋����������ꍇ�́A�o�H�ɒǉ����Ȃ�
	std::map<std::pair<std::pair<int, int>, std::pair<int, int>>, Route> routes;
	Board baseBoard(board);
	for (const auto& bPos : bPositions)
	{
		baseBoard[bPos.y][bPos.x] = '#';
	}

	for (const auto& initPosition : initPositions)
	{
		// �Ώۂ̉ו��ȊO��ǂƂ݂Ȃ����Ֆʂ����
		Board tmpBoard = baseBoard;
		tmpBoard[bPositions[initPosition.first].y][bPositions[initPosition.first].x] = '$';

		// �_�C�N�X�g���@�̒T���ŗp����A�Ֆʂ�\������O���t
		// ���^�C���̍��W���L�[�Ƃ��A���̏��^�C���ɗאڂ��Ă��鏰�^�C���ƁA���̋�����\���A�z�z����i�[
		std::map<std::pair<int, int>, std::map<std::pair<int, int>, unsigned int>> graph;
		// ������
		for (int x = 1; x < mSize.x - 1; x++)
		{
			for (int y = 1; y < mSize.y - 1; y++)
			{
				if (tmpBoard[y][x] == ' ' || tmpBoard[y][x] == '@' || tmpBoard[y][x] == '$')
				{
					std::vector<sf::Vector2i> tmpDir = GetRandomDirections();
					for (const auto& dir : tmpDir)
					{
						if (tmpBoard[y + dir.y][x + dir.x] == ' ' || tmpBoard[y + dir.y][x + dir.x] == '@' || tmpBoard[y + dir.y][x + dir.x] == '$')
						{
							// ���݂̃^�C�����A�אڂ��Ă���^�C���̂����ꂩ�������B�����̏ꍇ
							// �ӂ̏d�݂�Ֆʂ̏c�~���~4�̐��l�ɐݒ�
							if ((pPassed[y][x] == 0 && bPassed[y][x] == 0) || (pPassed[y + dir.y][x + dir.x] == 0 && bPassed[y + dir.y][x + dir.x] == 0))
							{
								graph[std::make_pair(x, y)][std::make_pair(x + dir.x, y + dir.y)] = mSize.x * mSize.y * 4;
							}
							// ���݂̃^�C���ƁA�אڂ��Ă���^�C���̗��������ɒʂ��������̏ꍇ
							// �ӂ̏d�݂�1�ɐݒ�
							else
							{
								graph[std::make_pair(x, y)][std::make_pair(x + dir.x, y + dir.y)] = 1;
							}
						}
					}
				}
			}
		}

		// �X�^�[�g�n�_����̊e���^�C���ւ̋�����\���ϐ�
		// �������A�����^�C���ł����Ă��A�ړ����̃^�C�����Ƃɏꍇ��������
		// first -> �ړ����̃^�C��
		// second -> �ړ���̃^�C��
		// �����͈ړ���̃^�C���܂ł̋���
		std::map<std::pair<std::pair<int, int>, std::pair<int, int>>, unsigned int> distances;
		//������
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

		// ���W�ƍŒZ������ێ�����D��x�t���L���[
		// 1. �ڕW�̏��^�C���܂ł̏d�݂�������������
		// 2. �v���C���[�̍��W
		// 3. �ڕW�̏��^�C���̍��W
		// 4. 1�O�̏��^�C���̍��W
		// 5. �ڕW�̏��^�C���ֈړ�����܂ł̃v���C���[�̌o�H
		// 6. �ڕW�̏��^�C���ֈړ��������Ƃ��̔Ֆʂ̏��
		// 7. �ڕW�̏��^�C���ֈړ��������Ƃ��̃v���C���[�̓��j��
		// 8. �ڕW�̏��^�C���ֈړ��������Ƃ��̉ו��̓��j��
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
			// C++�̗D��x�t���L���[�̓f�t�H���g���~���Ȃ̂ŁA���������ւ���
			// �L���[�ɒǉ�����Ƃ��͕��̒l�ɂ��āA���o���Ƃ��͐��̒l�ɂ���
			Elem currentElem = pq.top();
			pq.pop();

			// ���ɍŒZ�o�H���X�V����Ă���ꍇ
			if (-currentElem.mLen > distances[std::make_pair(Vec2iToPair(currentElem.bPos), Vec2iToPair(currentElem.prevPos))])
			{
				continue;
			}

			// �ו��̗אڕ����𒲂ׂ�
			for (const auto& neighbor : graph[Vec2iToPair(currentElem.bPos)])
			{
				// �v���C���[�͉ו���ړI�̗אڕ����ֈړ������邽�߂̈ʒu�Ɉړ��ł��邩�H
				sf::Vector2i neighborPos{ neighbor.first.first, neighbor.first.second };
				Route toNextPos(FindRouteToBaggage(currentElem.pPos, currentElem.bPos - (neighborPos - currentElem.bPos), currentElem.currentBoard, currentElem.currentPPassed, currentElem.currentBPassed));
				if ((currentElem.currentBoard[neighbor.first.second][neighbor.first.first] == ' ' ||
					currentElem.currentBoard[neighbor.first.second][neighbor.first.first] == '@') &&
					(currentElem.pPos == currentElem.bPos - (neighborPos - currentElem.bPos) ||
					!toNextPos.empty()))
				{
					// ���̒n�_�܂ł̋���
					int neighbor_dist = -currentElem.mLen + neighbor.second;
					// ���ݒn���玟�̒n�_�܂ł̈ʒu�ƌ���
					std::pair<std::pair<int, int>, std::pair<int, int>> path(neighbor.first, Vec2iToPair(currentElem.bPos));
					// �ŒZ�����ł���΁A�L���[�ɒǉ�
					if (neighbor_dist < distances[path])
					{
						Elem nextElem(currentElem);
						toNextPos.emplace_back(nextElem.bPos);
						nextElem.pRoute.insert(nextElem.pRoute.end(), toNextPos.begin(), toNextPos.end());
						nextElem.prevPos = nextElem.bPos;
						MoveOnPath(toNextPos, nextElem.currentBoard, nextElem.pPos, nextElem.bPos, nextElem.currentPPassed, nextElem.currentBPassed, false);
						nextElem.mLen = -neighbor_dist;
						distances[path] = neighbor_dist;
						pq.push(nextElem);

						// �o�H���X�V
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
	// �v���C���[�̌��ݒn�_����ڕW�n�_�܂ł̌o�H�����W�̃��X�g�ŕԂ�
	// �o�H��������Ȃ��ꍇ�͋�̔z���Ԃ�
	Route result;

	// �_�C�N�X�g���@�̒T���ŗp����O���t
	std::map<std::pair<int, int>, std::map<std::pair<int, int>, unsigned int>> graph;
	// ������
	for (int x = 1; x < mSize.x - 1; x++)
	{
		for (int y = 1; y < mSize.y - 1; y++)
		{
			if (board[y][x] == ' ' || board[y][x] == '@')
			{
				std::vector<sf::Vector2i> tmpDir = GetRandomDirections();
				for (const auto& dir : tmpDir)
				{
					if (board[y + dir.y][x + dir.x] == ' ' || board[y + dir.y][x + dir.x] == '@')
					{
						// ���݂̃^�C�����A�אڂ��Ă���^�C���̂����ꂩ�������B�����̏ꍇ
						// �ӂ̏d�݂�Ֆʂ̏c�~���̐��l�ɐݒ�
						if ((pPassed[y][x] == 0 && bPassed[y][x] == 0) || (pPassed[y + dir.y][x + dir.x] == 0 && bPassed[y + dir.y][x + dir.x] == 0))
						{
							graph[std::make_pair(x, y)][std::make_pair(x + dir.x, y + dir.y)] = mSize.x * mSize.y;
						}
						// ���݂̃^�C���ƁA�אڂ��Ă���^�C���̗��������ɒʂ��������̏ꍇ
						// �ӂ̏d�݂�1�ɐݒ�
						else
						{
							graph[std::make_pair(x, y)][std::make_pair(x + dir.x, y + dir.y)] = 1;
						}
					}
				}
			}
		}
	}
	
	// �S�[���n�_���l����ǂƉו��Ɉ͂܂�Ă���ꍇ�͋�̂܂܏o��
	if (graph[std::make_pair(goal.x, goal.y)].empty())
	{
		return result;
	}

	// �X�^�[�g�n�_����̊e���^�C���ւ̋�����\���ϐ�
	std::map<std::pair<int, int>, unsigned int> distance;
	//������
	for (const auto& item : graph)
	{
		distance[item.first] = UINT_MAX;
	}
	// �X�^�[�g�n�_�̍��W�͋�����0�Ƃ��Ă���
	distance[std::make_pair(pCoordinate.x, pCoordinate.y)] = 0;
	
	std::priority_queue<std::pair<int, std::pair<int, int>>> pq;
	pq.push({ 0, std::make_pair(pCoordinate.x, pCoordinate.y) });

	while (!pq.empty())
	{
		int dist = -pq.top().first;
		std::pair<int, int> current = pq.top().second;
		pq.pop();

		// ���ɍŒZ�o�H���X�V����Ă���ꍇ
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
	
	// �o�H��������Ȃ������ꍇ�͋�̂܂܏o��
	if (distance[std::make_pair(goal.x, goal.y)] == UINT_MAX)
	{
		return result;
	}

	// �ŒZ�o�H���\�z
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
	std::vector<std::vector<unsigned int>>& bPassCount,
	const bool& isSave)
{
	for (const auto& nextPos : route)
	{
		// ���̃^�C���ɉ����Ȃ������ꍇ
		if (board[nextPos.y][nextPos.x] == ' ')
		{
			// �o�H�̋L�^
			if (isSave)
			{
				mSolutionPos.emplace_back(nextPos);
				if (nextPos - pPos == sf::Vector2i{ 1, 0 })
				{
					mSolutionDirection += "r";
				}
				else if (nextPos - pPos == sf::Vector2i{ 0, 1 })
				{
					mSolutionDirection += "d";
				}
				else if (nextPos - pPos == sf::Vector2i{ -1, 0 })
				{
					mSolutionDirection += "l";
				}
				else if (nextPos - pPos == sf::Vector2i{ 0, -1 })
				{
					mSolutionDirection += "u";
				}
			}

			// �v���C���[�̈ړ�
			board[pPos.y][pPos.x] = ' ';
			board[nextPos.y][nextPos.x] = '@';
			pPassCount[nextPos.y][nextPos.x]++;
			pPos = nextPos;
		}
		// ���̃^�C���ɉו����������ꍇ
		else if (board[nextPos.y][nextPos.x] == '$')
		{
			// �ו��̈ړ���̍��W
			sf::Vector2i nextNextPos(nextPos.x + (nextPos.x - pPos.x), nextPos.y + (nextPos.y - pPos.y));
			if (board[nextNextPos.y][nextNextPos.x] == ' ')
			{
				// �o�H�̋L�^
				if (isSave)
				{
					mSolutionPos.emplace_back(nextPos);
					if (nextPos - pPos == sf::Vector2i{ 1, 0 })
					{
						mSolutionDirection += "R";
					}
					else if (nextPos - pPos == sf::Vector2i{ 0, 1 })
					{
						mSolutionDirection += "D";
					}
					else if (nextPos - pPos == sf::Vector2i{ -1, 0 })
					{
						mSolutionDirection += "L";
					}
					else if (nextPos - pPos == sf::Vector2i{ 0, -1 })
					{
						mSolutionDirection += "U";
					}
				}

				// �v���C���[�Ɖו��̈ړ�
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
	const std::vector<std::vector<unsigned int>>& bPassCount,
	const int& switchCount,
	const CandidateItems& movableBaggages)
{
	std::pair<int, Route> result;
	// �ו����Ƃɉו������S�o�H�̃��[�g�̘A�z�z������
	std::unordered_map<int, std::unordered_map<int, Rating>> mRates;
	// �ł����[�g�̏��Ȃ��o�H���i�[����
	// �ו��̃C���f�b�N�X�ƁA�o�H�̃C���f�b�N�X�ƁA���̃��[�g���i�[
	std::pair<int, std::pair<int, double>> minRate(-1, { -1, DBL_MAX });

	// cerRoute�͉ו���ID���L�[�A�o�H�̃��X�g��l�Ƃ��Ď��y�A
	for (const auto& cerRoute : routes)
	{
		// ���݂̉ו��̗v�f��ǉ�
		mRates[cerRoute.first] = std::unordered_map<int, Rating>{};
		// ���[�g�͂��̉ו��̌o�H�̐������L�^
		auto bSize = cerRoute.second.size();
		for (int i = 0; i < bSize; i++)
		{
			// �Ֆʂ̃R�s�[�����A�v���C���[���o�H�ʂ�Ɉړ�������̔Ֆʂ����
			Board cpBoard = board;
			sf::Vector2i cpPos = pPos;
			sf::Vector2i cbPos = bPositions[cerRoute.first];
			std::vector<std::vector<unsigned int>> cpPassCount = pPassCount;
			std::vector<std::vector<unsigned int>> cbPassCount = bPassCount;

			// �v���C���[�̈ړ�
			MoveOnPath(cerRoute.second[i], cpBoard, cpPos, cbPos, cpPassCount, cbPassCount, false);
			
			// 2x2�X�y�[�X�̐��̃J�E���g
			int count2x2Areas = 0;
			for (int x = 1; x < mSize.x - 2; x++)
			{
				for (int y = 1; y < mSize.y - 2; y++)
				{
					bool flag = true;
					for (int m = x; m < x + 2; m++)
					{
						for (int n = y; n < y + 2; n++)
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
						count2x2Areas++;
					}
				}
			}

			// 2x3�X�y�[�X�̐��̃J�E���g
			int count2x3Areas = 0;
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
						count2x3Areas++;
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
						count2x3Areas++;
					}
				}
			}

			// 3x3�X�y�[�X�̐��̃J�E���g
			int count3x3Areas = 0;
			for (int x = 1; x < mSize.x - 3; x++)
			{
				for (int y = 1; y < mSize.y - 3; y++)
				{
					bool flag = true;
					for (int m = x; m < x + 3; m++)
					{
						for (int n = y; n < y + 3; n++)
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
						count3x3Areas++;
					}
				}
			}

			// �s���~�܂�̐��̃J�E���g
			// �p�ɉ������܂�ē������Ȃ��ו��̐��̃J�E���g
			// �Ֆʂ̕ӏ�ɂ���ו��̐��̃J�E���g
			// �Ֆʂ̊p�ɂ���ו��̐��̃J�E���g
			int deadEnds = 0, cornerEnds = 0, sides = 0, corners = 0;
			for (int x = 1; x < mSize.x - 1; x++)
			{
				for (int y = 1; y < mSize.y - 1; y++)
				{
					if (cpBoard[y][x] == '$')
					{
						// �ו��̎��͂̕ǂ𒲂ׂ�
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
						// �ו����p�ɉ������܂�Ă��邩���ׂ�
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

						// �ו��̈ʒu���ӂ��p�����ׂ�
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

			// �v���C���[�Ɖו��̈ړ���̏�Ԃł̓�������ו��̌��̐������߂�
			// ������ו��Ɖ����n�߂���ʒu�̃��X�g�����߂�
			// �ו����Ƃ�0�ӏ�����A�ő�4�ӏ��܂Ńv���C���[�̊J�n�ʒu���^������
			// ���݉����Ă���ו��͏����āA�ו��̃C���f�b�N�X���L�[�Ƃ����A�v���C���[�̊J�n�ʒu�̃��X�g�����
			CandidateItems startPos;
			// �^�������ו��ȊO�͈ʒu���ς���Ă��Ȃ��̂ł��̂܂܃R�s�[
			std::vector<sf::Vector2i> cbPoints = bPositions;
			cbPoints[cerRoute.first] = cbPos;
			SetCandidates(cpPos, cbPoints, cpBoard, startPos, cerRoute.first);
			int changedMovable = 0;
			// �������邩�������Ȃ���������ւ���Ă��鐔���J�E���g
			for (const auto& item : startPos)
			{
				if (movableBaggages.count(item.first) == 0)
				{
					continue;
				}
				for (const auto& pos : item.second)
				{
					if (std::find(movableBaggages.at(item.first).begin(), movableBaggages.at(item.first).end(), pos) == movableBaggages.at(item.first).end())
					{
						--changedMovable;
					}
				}
			}
			for (const auto& item : movableBaggages)
			{
				if (startPos.count(item.first) == 0)
				{
					continue;
				}
				for (const auto& pos : item.second)
				{
					if (std::find(startPos[item.first].begin(), startPos[item.first].end(), pos) == startPos[item.first].end())
					{
						--changedMovable;
					}
				}
			}

			// cerRoute.first�͉ו��̃C���f�b�N�X�Ai�͌o�H�̃C���f�b�N�X
			mRates[cerRoute.first][i].m2x2Spaces = count2x2Areas;
			mRates[cerRoute.first][i].m2x3Spaces = count2x3Areas;
			mRates[cerRoute.first][i].m3x3Spaces = count3x3Areas;
			mRates[cerRoute.first][i].mSwitchCount = switchCount;
			mRates[cerRoute.first][i].mDisCenterToGoal = 0;
			mRates[cerRoute.first][i].mGoalArround = 0;
			mRates[cerRoute.first][i].mDeadEnd = deadEnds;
			mRates[cerRoute.first][i].mCornerEnd = cornerEnds;
			mRates[cerRoute.first][i].mSides = sides;
			mRates[cerRoute.first][i].mCorners = corners;
			mRates[cerRoute.first][i].mChangedCandidates = changedMovable;
			
			// �]���l���v�Z
			SetEvaluationValue(mRates[cerRoute.first][i], switchCount);

			// ���[�g�̍ŏ��l���X�V�ł���ꍇ�́A�X�V
			if (minRate.second.second > mRates[cerRoute.first][i].totalRate)
			{
				minRate.second.second = mRates[cerRoute.first][i].totalRate;
			}
		}
	}

	// �ł����[�g���Ⴂ�o�H���烉���_���ɑI��
	// �ł����[�g�̒Ⴂ�S�Ă̌o�H�̉ו��ƌo�H�̃C���f�b�N�X�̃y�A���o�H�̒������L�[�Ƃ����A�z�z��Ɋi�[
	std::map<int, std::vector<std::pair<int, int>>> candidate;

	// �o�H���֌W�Ȃ��ɑI�΂���
	std::vector<std::pair<int, int>> minRateRoutes;
	std::pair<int, int> choosedIndex;

	for (const auto& cerRoute : routes)
	{
		auto bSize = cerRoute.second.size();
		for (int i = 0; i < bSize; i++)
		{
			if (mRates[cerRoute.first][i].totalRate == minRate.second.second)
			{
				candidate[cerRoute.second.size()].emplace_back(std::make_pair(cerRoute.first, i));
				minRateRoutes.emplace_back(std::make_pair(cerRoute.first, i));
			}
		}
	}

	// 
	int minKey = -1;
	std::cout << "Current board : " << std::endl;
	for (const auto& line : board)
	{
		std::cout << line << std::endl;
	}
	std::cout << "All candidates : " << std::endl;
	for (const auto& item : candidate)
	{
		// �\�������Ă݂�
		for (const auto& indexes : item.second)
		{
			std::cout << "baggage : " << indexes.first << ", route : " << indexes.second << " > ";
			for (const auto& pos : routes.at(indexes.first)[indexes.second])
			{
				std::cout << "[ " << pos.x << ", " << pos.y << " ] -> ";
			}
			std::cout << std::endl;
		}

		// �o�H���̒����o�H���烉���_���ɑI�ԏꍇ��
		if (minKey < item.first)
		{
			minKey = item.first;
		}
	}
	
	/*
	std::uniform_int_distribution<int> rand(0, candidate[minKey].size() - 1);
	// �����𐶐����� 
	auto index = rand(mt);
	minRate.first = candidate[minKey][index].first;
	minRate.second.first = candidate[minKey][index].second;
	result = { minRate.first, routes.at(minRate.first)[minRate.second.first] };
	//*/

	choosedIndex = GetRandomElement(minRateRoutes);
	result = { choosedIndex.first, routes.at(choosedIndex.first)[choosedIndex.second] };

	return result;
}

void MySolution::SetEvaluationValue(Rating& rates, const int& switchCount)
{
	// �I�����ꂽ�]���֐��̒l�����߂�֐�

	// �ȉ�����
	// �ו��̐����Ֆʂ̍L���ɑ΂��đ����ꍇ��2x2��2x3�̐��͂��܂�d�v�ł͂Ȃ�
	// �t�ɉו��̐����Ֆʂ̍L���ɑ΂��ď��Ȃ��ꍇ��2x2��2x3�̐��͏��Ȃ���������
	// 3������ǂň͂܂�Ă���ו��͂��ł����e���Ȃ���������
	// �^������ו���؂�ւ����񐔂������Ă����ꍇ�͊O���̕ӂ�,�p�̒n�`�ɔz�u���ꂽ�ו������e����悤�ɂ���
	
	switch (mStrategyIndex)
	{
	case 0:
		// 2x2�̃X�y�[�X�̐�
		// ����3�������ǂň͂܂�Ă���ו��̐�
		// ���K��}�X�̊p�̉ו��̐�
		// �O���̕ǂɐڂ���ו��̐�
		// �O���̊p�ɔz�u����Ă���ו��̐�
		// �����̘a
		rates.totalRate =
			static_cast<double>(rates.m2x2Spaces) +
			//static_cast<double>(rates.m2x3Spaces) +
			//static_cast<double>(rates.m3x3Spaces) +
			static_cast<double>(rates.mDisCenterToGoal / (mSize.x * mSize.y)) +
			static_cast<double>(rates.mGoalArround) +
			static_cast<double>(rates.mDeadEnd) +
			static_cast<double>(rates.mCornerEnd) +
			static_cast<double>(rates.mSides) +
			static_cast<double>(rates.mCorners);
		break;
	case 1:
		// 2x3�̃X�y�[�X�̐�
		// ����3�������ǂň͂܂�Ă���ו��̐�
		// ���K��}�X�̊p�̉ו��̐�
		// �O���̕ǂɐڂ���ו��̐�
		// �O���̊p�ɔz�u����Ă���ו��̐�
		// �����̘a
		rates.totalRate =
			//static_cast<double>(rates.m2x2Spaces) +
			static_cast<double>(rates.m2x3Spaces) +
			//static_cast<double>(rates.m3x3Spaces) +
			static_cast<double>(rates.mDisCenterToGoal / (mSize.x * mSize.y)) +
			static_cast<double>(rates.mGoalArround) +
			static_cast<double>(rates.mDeadEnd) +
			static_cast<double>(rates.mCornerEnd) +
			static_cast<double>(rates.mSides) +
			static_cast<double>(rates.mCorners);
		break;
	case 2:
		// 2x2�̃X�y�[�X�̐�
		// 2x3�̃X�y�[�X�̐�
		// ����3�������ǂň͂܂�Ă���ו��̐�
		// ���K��}�X�̊p�̉ו��̐�
		// �O���̕ǂɐڂ���ו��̐�
		// �O���̊p�ɔz�u����Ă���ו��̐�
		// �����̘a
		rates.totalRate =
			static_cast<double>(rates.m2x2Spaces) +
			static_cast<double>(rates.m2x3Spaces) +
			//static_cast<double>(rates.m3x3Spaces) +
			static_cast<double>(rates.mDisCenterToGoal / (mSize.x * mSize.y)) +
			static_cast<double>(rates.mGoalArround) +
			static_cast<double>(rates.mDeadEnd) +
			static_cast<double>(rates.mCornerEnd) +
			static_cast<double>(rates.mSides) +
			static_cast<double>(rates.mCorners);
		break;
	case 3:
		// 2x2�̃X�y�[�X�̐��� ((�Ֆʂ̖ʐ� - �ו��̐�) / �Ֆʂ̖ʐ�) ���������l
		// ����3�������ǂň͂܂�Ă���ו��̐�
		// ���K��}�X�̊p�̉ו��̐��� (�ו��̐� / �^������ו���؂�ւ�����) ���|��������
		// �O���̕ǂɐڂ���ו��̐��� (�ו��̐� / �^������ו���؂�ւ�����) ���|��������
		// �O���̊p�ɔz�u����Ă���ו��̐��� (�ו��̐� / �^������ו���؂�ւ�����) ���|��������
		// �����̘a
		rates.totalRate =
			std::floor(
			static_cast<double>(rates.m2x2Spaces) * static_cast<double>(mSize.x * mSize.y - mBaggageNum) / static_cast<double>(mSize.x * mSize.y) +
			//static_cast<double>(rates.m2x3Spaces) * static_cast<double>(mSize.x * mSize.y - mBaggageNum) / static_cast<double>(mSize.x * mSize.y) +
			//static_cast<double>(rates.m3x3Spaces) * static_cast<double>(mSize.x * mSize.y - mBaggageNum) / static_cast<double>(mSize.x * mSize.y) +
			static_cast<double>(rates.mDisCenterToGoal / (mSize.x * mSize.y)) +
			static_cast<double>(rates.mGoalArround) +
			static_cast<double>(rates.mDeadEnd) * static_cast<double>(mBaggageNum) / static_cast<double>(switchCount + 1) +
			static_cast<double>(rates.mCornerEnd) * static_cast<double>(mBaggageNum) / static_cast<double>(switchCount + 1) +
			static_cast<double>(rates.mSides) * static_cast<double>(mBaggageNum) / static_cast<double>(switchCount + 1) +
			static_cast<double>(rates.mCorners) * static_cast<double>(mBaggageNum) / static_cast<double>(switchCount + 1)
				);
		break;
	case 4:
		// 2x3�̃X�y�[�X�̐��� ((�Ֆʂ̖ʐ� - �ו��̐�) / �Ֆʂ̖ʐ�) ���������l
		// ����3�������ǂň͂܂�Ă���ו��̐�
		// ���K��}�X�̊p�̉ו��̐��� (�ו��̐� / �^������ו���؂�ւ�����) ���|��������
		// �O���̕ǂɐڂ���ו��̐��� (�ו��̐� / �^������ו���؂�ւ�����) ���|��������
		// �O���̊p�ɔz�u����Ă���ו��̐��� (�ו��̐� / �^������ו���؂�ւ�����) ���|��������
		// �����̘a
		rates.totalRate =
			std::floor(
			//static_cast<double>(rates.m2x2Spaces) * static_cast<double>(mSize.x * mSize.y - mBaggageNum) / static_cast<double>(mSize.x * mSize.y) +
			static_cast<double>(rates.m2x3Spaces) * static_cast<double>(mSize.x * mSize.y - mBaggageNum) / static_cast<double>(mSize.x * mSize.y) +
			//static_cast<double>(rates.m3x3Spaces) * static_cast<double>(mSize.x * mSize.y - mBaggageNum) / static_cast<double>(mSize.x * mSize.y) +
			static_cast<double>(rates.mDisCenterToGoal / (mSize.x * mSize.y)) +
			static_cast<double>(rates.mGoalArround) +
			static_cast<double>(rates.mDeadEnd) * static_cast<double>(mBaggageNum) / static_cast<double>(switchCount + 1) +
			static_cast<double>(rates.mCornerEnd) * static_cast<double>(mBaggageNum) / static_cast<double>(switchCount + 1) +
			static_cast<double>(rates.mSides) * static_cast<double>(mBaggageNum) / static_cast<double>(switchCount + 1) +
			static_cast<double>(rates.mCorners) * static_cast<double>(mBaggageNum) / static_cast<double>(switchCount + 1)
				);
		break;
	case 5:
		// 2x2�̃X�y�[�X�̐��� ((�Ֆʂ̖ʐ� - �ו��̐�) / �Ֆʂ̖ʐ�) ���������l
		// 2x3�̃X�y�[�X�̐��� ((�Ֆʂ̖ʐ� - �ו��̐�) / �Ֆʂ̖ʐ�) ���������l
		// ����3�������ǂň͂܂�Ă���ו��̐�
		// ���K��}�X�̊p�̉ו��̐��� (�ו��̐� / �^������ו���؂�ւ�����) ���|��������
		// �O���̕ǂɐڂ���ו��̐��� (�ו��̐� / �^������ו���؂�ւ�����) ���|��������
		// �O���̊p�ɔz�u����Ă���ו��̐��� (�ו��̐� / �^������ו���؂�ւ�����) ���|��������
		// �����̘a
		rates.totalRate =
			std::floor(
			static_cast<double>(rates.m2x2Spaces) * static_cast<double>(mSize.x * mSize.y - mBaggageNum) / static_cast<double>(mSize.x * mSize.y) +
			static_cast<double>(rates.m2x3Spaces) * static_cast<double>(mSize.x * mSize.y - mBaggageNum) / static_cast<double>(mSize.x * mSize.y) +
			//static_cast<double>(rates.m3x3Spaces) * static_cast<double>(mSize.x * mSize.y - mBaggageNum) / static_cast<double>(mSize.x * mSize.y) +
			static_cast<double>(rates.mDisCenterToGoal / (mSize.x * mSize.y)) +
			static_cast<double>(rates.mGoalArround) +
			static_cast<double>(rates.mDeadEnd) * static_cast<double>(mBaggageNum) / static_cast<double>(switchCount + 1) +
			static_cast<double>(rates.mCornerEnd) * static_cast<double>(mBaggageNum) / static_cast<double>(switchCount + 1) +
			static_cast<double>(rates.mSides) * static_cast<double>(mBaggageNum) / static_cast<double>(switchCount + 1) +
			static_cast<double>(rates.mCorners) * static_cast<double>(mBaggageNum) / static_cast<double>(switchCount + 1)
				);
		break;
	case 6:
		// 2x3�̃X�y�[�X�̐��� ((�Ֆʂ̖ʐ� - �ו��̐�) / �Ֆʂ̖ʐ�) ���������l
		// ����3�������ǂň͂܂�Ă���ו��̐�
		// ���K��}�X�̊p�̉ו��̐��� (�ו��̐� / �^������ו���؂�ւ�����) ���|��������
		// �O���̕ǂɐڂ���ו��̐��� (�ו��̐� / �^������ו���؂�ւ�����) ���|��������
		// �O���̊p�ɔz�u����Ă���ו��̐��� (�ו��̐� / �^������ו���؂�ւ�����) ���|��������
		// �����̘a����
		rates.totalRate =
			std::floor(
				//static_cast<double>(rates.m2x2Spaces) * static_cast<double>(mSize.x * mSize.y - mBaggageNum) / static_cast<double>(mSize.x * mSize.y) +
				static_cast<double>(rates.m2x3Spaces) * static_cast<double>(mSize.x * mSize.y - mBaggageNum) / static_cast<double>(mSize.x * mSize.y) +
				//static_cast<double>(rates.m3x3Spaces) * static_cast<double>(mSize.x * mSize.y - mBaggageNum) / static_cast<double>(mSize.x * mSize.y) +
				static_cast<double>(rates.mDisCenterToGoal / (mSize.x * mSize.y)) +
				static_cast<double>(rates.mGoalArround) +
				static_cast<double>(rates.mDeadEnd) * static_cast<double>(mBaggageNum) / static_cast<double>(switchCount + 1) +
				static_cast<double>(rates.mCornerEnd) * static_cast<double>(mBaggageNum) / static_cast<double>(switchCount + 1) +
				static_cast<double>(rates.mSides) * static_cast<double>(mBaggageNum) / static_cast<double>(switchCount + 1) +
				static_cast<double>(rates.mCorners) * static_cast<double>(mBaggageNum) / static_cast<double>(switchCount + 1) +
				static_cast<double>(rates.mChangedCandidates)
			);
		break;
	default:
		rates.totalRate =
			static_cast<double>(rates.m2x2Spaces) +
			//static_cast<double>(rates.m2x3Spaces) +
			//static_cast<double>(rates.m3x3Spaces) +
			static_cast<double>(rates.mDisCenterToGoal / (mSize.x * mSize.y)) +
			static_cast<double>(rates.mGoalArround) +
			static_cast<double>(rates.mDeadEnd) +
			static_cast<double>(rates.mCornerEnd) +
			static_cast<double>(rates.mSides) +
			static_cast<double>(rates.mCorners);
		break;
	}
}