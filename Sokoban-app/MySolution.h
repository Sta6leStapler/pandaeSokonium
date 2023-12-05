#pragma once

#include "SFML/Graphics.hpp"
#include <string>
#include <vector>
#include <random>
#include <unordered_map>
#include <map>

#include <iostream>

using Board = std::vector<std::string>;
using Route = std::vector<sf::Vector2i>;
using CandidateItems = std::unordered_map<int, std::vector<sf::Vector2i>>;
using CandidateRoutes = std::unordered_map<int, std::vector<Route>>;

class MySolution
{
public:
	MySolution(const sf::Vector2i& size, const int& baggageNum, const int& buildNum, const int& runsNum);

	// �Q�b�^�[�Z�b�^�[
	Board GetBoard() { return mBoardStr; }
	int GetBaggageNum() { return mBaggageNum; }
	std::vector<std::pair<sf::Vector2i, sf::Vector2i>> GetInitToGoalList() { return mInitToGoalList; }
	int GetSolveUpper() { return mSolveUpper; }

private:
	enum Direction
	{
		ENorth,
		EEast,
		ESouth,
		EWest
	};

	// �ו��ƃG�[�W�F���g��z�u����t�F�[�Y
	void InitSettings();
	
	// ���z�v���C�̃V�~�����[�V�������s���t�F�[�Y
	void RunSimulation();

	// �V�~�����[�V�����̏I��������s���֐�
	bool isLoopEnd(const Board& board, const std::vector<bool>& isMoved, const std::vector<std::vector<unsigned int>>& pPassCount, const std::vector<std::vector<unsigned int>>& bPassCount);

	// �����_���ȃp�����[�^���擾����֐�
	Direction RollDirection();
	sf::Vector2i RollCoordinate();
	sf::Vector2i RollCoordinate(const int& xMin, const int& xMax, const int& yMin, const int& yMax);
	int RollCoordinateX();
	int RollCoordinateX(const int& xMin, const int& xMax);
	int RollCoordinateY();
	int RollCoordinateY(const int& yMin, const int& yMax);
	template <typename T>
	T GetRandomElement(const std::vector<T>& vec);
	template<typename T, typename U>
	std::pair<T, U> GetRandomElement(const std::unordered_map<T, U>& map);

	// ������W�̎��͂̃}�X�𒲂ׂ�֐�
	std::vector<sf::Vector2i> GetAroundWalls(const sf::Vector2i& point);

	// �l�݂̉ו��̔z�u���Ȃ����Ƃ��m�F����֐�
	bool isArrangementValid(const Board& board, const std::vector<sf::Vector2i>& bPositions);
	
	// �ו����z�u�ł��邩���ׂ�֐�
	bool existsFreeSpace(const Board& board, const std::vector<sf::Vector2i>& bPositions);

	// ������ו��̌��Ɖ����n�߂���ʒu�̑S�Ă̌������߂�֐�
	void SetCandidates(const sf::Vector2i& pPos, const std::vector<sf::Vector2i>& bPositions, const Board& board, CandidateItems& outList);

	// �v���C���[�Ƃ���ו��̉\�Ȉړ��o�H�̑S�Ă̌������߂�֐�
	void SetRoutes(const sf::Vector2i& pPos, const std::vector<sf::Vector2i>& bPositions, const Board& board, std::unordered_map<int, std::vector<Route>>& outRoutes, const CandidateItems& initPositions, const std::vector<std::vector<unsigned int>>& pPassed, const std::vector<std::vector<unsigned int>>& bPassed);

	// �v���C���[�̌��ݒn����ו��̉��܂ł̈ړ��o�H�����߂�֐�
	Route FindRouteToBaggage(const sf::Vector2i& pCoordinate, const sf::Vector2i& goal, const Board& board, const std::vector<std::vector<unsigned int>>& pPassed, const std::vector<std::vector<unsigned int>>& bPassed);

	// �v���C���[�̈ړ��o�H���󂯎��A�V�~�����[�g���s���֐�
	void MoveOnPath(
		const Route& route,
		Board& board,
		sf::Vector2i& pPos,
		sf::Vector2i& bPos,
		std::vector<std::vector<unsigned int>>& pPassCount,
		std::vector<std::vector<unsigned int>>& bPassCount);

	// �o�H��]�����A�K���Ȃ��̂�I������֐�
	std::pair<int, Route> GetBestRoute(
		const std::unordered_map<int, std::vector<Route>>& routes,
		const Board& board,
		const sf::Vector2i& pPos,
		const std::vector<sf::Vector2i> bPositions,
		const std::vector<std::vector<unsigned int>>& pPassCount,
		const std::vector<std::vector<unsigned int>>& bPassCount);

	// �T���֘A�̕ϐ�
	const std::vector<sf::Vector2i> directions = {
		sf::Vector2i(1, 0),   // �E
		sf::Vector2i(0, 1),  // ��
		sf::Vector2i(-1, 0),   // ��
		sf::Vector2i(0, -1)   // ��
	};

	// �������C���f�b�N�X�ɕϊ�����֐�
	int GetDirectionIndex(const sf::Vector2i& dir)
	{
		if (dir == sf::Vector2i{ 1, 0 })
		{
			return 0;
		}
		else if (dir == sf::Vector2i{ 0, 1 })
		{
			return 1;
		}
		else if (dir == sf::Vector2i{ -1, 0 })
		{
			return 2;
		}
		else if (dir == sf::Vector2i{ 0, -1 })
		{
			return 3;
		}
		
		return -1;
	}

	// ���W�̃f�[�^�\���̕ϊ�
	std::pair<int, int> Vec2iToPair(const sf::Vector2i& pos)
	{
		return std::make_pair(pos.x, pos.y);
	}
	sf::Vector2i PairToVec2i(const std::pair<int, int>& pos)
	{
		return sf::Vector2i{ pos.first, pos.second };
	}

	// �����֘A�̕ϐ�
	std::random_device rd;
	std::mt19937 mt;

	// �Ֆʊ֘A�̕ϐ�
	sf::Vector2i mSize;
	Board mBoardStr;	// �Ֆʂ̏���\��
	int mBaggageNum;	// �ו��̐�
	std::vector<sf::Vector2i> mBaggagesCoordinate;	// �ו��̒u���Ă�����W�̃��X�g

	// �Ֆʂ̐����Ɋւ���ϐ�
	const int mBuildsNum;
	const int mRunsNum;

	// ���̏�E
	int mSolveUpper;

	// �ו��ƃS�[���̑Ή�
	std::vector<std::pair<sf::Vector2i, sf::Vector2i>> mInitToGoalList;
};