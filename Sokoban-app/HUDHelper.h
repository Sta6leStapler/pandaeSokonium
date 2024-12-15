#pragma once

#include <vector>
#include <unordered_map>
#include <set>
#include <string>
#include <memory>
#include <random>
#include "SFML/Graphics.hpp"

using Board = std::vector<std::string>;
struct BoardStates
{
	std::vector<sf::Vector2i> mMovableArea;	// �v���C���[�̈ړ��\�ȍ��W�̃��X�g
	std::unordered_map<int, sf::Vector2i> mDeadlockedBaggages;	// �l�ݏ�Ԃ��m�肵�Ă���ו�
	std::unordered_map<int, sf::Vector2i> mGoaledBaggages;	// �S�[����Ԃ̉ו�
	std::unordered_map<int, sf::Vector2i> mNonGoaledBaggages;	// �S�[����ԂłȂ��ו�
};

class HUDHelper
{
public:
	HUDHelper(class Game* game);
	~HUDHelper();

	// �Ֆʂ̍X�V����
	void Update();

	// �l�݂̉ו��̔z�u���Ȃ����Ƃ��m�F����֐�
	// 1�ł��l�ݏ�Ԃ̉ו��������false��Ԃ�
	// 
	bool isArrangementValid();

	// �l�݂̉ו��̍��W�̃��X�g��Ԃ��֐�
	std::unordered_map<int, sf::Vector2i> GetDeadlockedBaggages() const { return mBoardStates.mDeadlockedBaggages; }

	// �v���C���[���ړ��\�ȍ��W�̃��X�g��Ԃ��֐�
	std::vector<sf::Vector2i> GetMovableArea() const { return mBoardStates.mMovableArea; }

	// �S�[����Ԃ̉ו���Ԃ��֐�
	std::unordered_map<int, sf::Vector2i> GetGoaledBaggages() const { return mBoardStates.mGoaledBaggages; }

	// �S�[����ԂłȂ��ו���Ԃ��֐�
	std::unordered_map<int, sf::Vector2i> GetNonGoaledBaggages() const { return mBoardStates.mNonGoaledBaggages; }

	// �w�肳�ꂽ�ו����l�݂��ǂ�����Ԃ��֐�
	bool isDeadlockedBaggage(const sf::Vector2i& pos);

	// ������ו��̌������߂�֐�
	std::vector<sf::Vector2i> GetCandidates();

private:
	enum Direction
	{
		ENorth,
		EEast,
		ESouth,
		EWest
	};

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
	std::vector<sf::Vector2i> GetRandomDirections();

	// �p�u���b�N�ȃ����o�֐����Ŏg���w���p�[�֐�
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
	// �w�肳�ꂽ�ו����l�ݏ�Ԃ��ǂ���
	bool isFreezeBaggage(const sf::Vector2i& baggagePos);
	// 2�̍��W���������G���A�ɂ��邩�𔻒肷��֐�
	bool isMovableArea(const sf::Vector2i& pos1, const sf::Vector2i& pos2);
	bool isMovableArea(const sf::Vector2i& pos1, const sf::Vector2i& pos2, const Board& board);
	// �v���C���[�̈ړ��\�Ȕ͈͂��擾����֐�
	std::vector<sf::Vector2i> GetMovableArea();
	// �f�b�h���b�N��Ԃ̉ו����X�V����֐�
	std::unordered_map<int, sf::Vector2i> UpdateFreezeBaggagesMap();
	// �X�V�����v���C���[�̍��W�Ɖו��̍��W����A�Ֆʂ��č\��
	void UpdateBoardStr();

	// �ȉ������o�ϐ�
	// Game�N���X
	class Game* mGame;
	
	// �T���֘A�̕ϐ�
	const std::vector<sf::Vector2i> directions =
	{
		sf::Vector2i(1, 0),   // �E
		sf::Vector2i(0, 1),  // ��
		sf::Vector2i(-1, 0),   // ��
		sf::Vector2i(0, -1)   // ��
	};

	// �����֘A�̕ϐ�
	std::random_device rd;
	std::mt19937 mt;

	// �Ֆʍ\���̕ϐ�
	BoardStates mBoardStates;
	std::map<std::pair<std::pair<int, int>, std::set<std::pair<int, int>>>, BoardStates> mHistory;

	// �Ֆʊ֘A�̕ϐ�
	sf::Vector2i mSize;
	Board mBoardStr;	// �Ֆʂ̏���\��
	Board mInitialBoardStr;	//�����Ֆ�
	Board mBoardMapStr;	// �Ֆʂ̒n�` (�v���C���[�Ɖו���r�������Ֆ�)
	int mBaggageNum;	// �ו��̐�
	sf::Vector2i mPlayerPos;	// �v���C���[�̍��W
	sf::Vector2i mInitialPlayerPos;	//�v���C���[�̏����ʒu
	std::vector<sf::Vector2i> mGoals;	// �S�[�����W
	// TODO �C���f�b�N�X��t���闝�R
	std::unordered_map<int, sf::Vector2i> mBaggagesPos;	// �ו��̒u���Ă�����W�̃��X�g
	std::unordered_map<int, sf::Vector2i> mGoaledBaggages;
	std::unordered_map<int, sf::Vector2i> mNonGoaledBaggages;
	std::unordered_map<int, sf::Vector2i> mInitialBaggagesPos;	// �ו��̏����ʒu
};