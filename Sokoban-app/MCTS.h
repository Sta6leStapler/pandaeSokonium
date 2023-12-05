#pragma once

#include "SFML/Graphics.hpp"
#include <string>
#include <vector>

// �f�[�^�쓮�^�]���Ɋւ���\����
struct boardState
{
	int tileMixingW;
	int tileMixingF;
	int blockCount;
	int baggageNum;
	int pathToGoalLength;
	int traffic01;
	int traffic02;
};

class MCTS
{
public:
	MCTS(const sf::Vector2i& size, const int& baggageNum);

	// �Q�b�^�[�Z�b�^�[
	std::vector<std::string> GetBoard() { return mBoardStr; }

private:
	enum Direction
	{
		ENorth,
		EEast,
		ESouth,
		EWest
	};

	void InitFaze();
	void SimFaze();

	// �����_���ȃp�����[�^���擾����֐�
	Direction RollDirection();
	int RollCoordinateX();
	int RollCoordinateY();

	// ������W�̎��͂̃}�X�𒲂ׂ�֐�
	std::vector<sf::Vector2i> GetAroundWalls(const sf::Vector2i& point);

	// ���z�v���C���[�̈ړ����s���֐�
	void moveVirtualPlayer(const Direction& dir, std::vector<std::string>& simField, sf::Vector2i& coordinate, std::vector<std::pair<sf::Vector2i, int>>& baggagesPushCount);

	// �Ֆʃf�[�^�������̃I�u�W�F�N�g�̍��W��Ԃ�
	std::vector<sf::Vector2i> GetCoordinates(const char& icon, const std::vector<std::string>& Field);

	// �ו��̍��W����C���f�b�N�X���擾
	int GetBaggageIndex(const sf::Vector2i& coordinate, const std::vector<std::pair<sf::Vector2i, int>>& baggagesPushCount);

	// �Ֆʃf�[�^����f�[�^�쓮�^�]�������߂�
	void ReloadBoardState(const std::vector<std::string>& Field);

	sf::Vector2i mSize;
	std::vector<std::string> mBoardStr;	// �Ֆʂ̏���\��
	int mBaggageNum;	// �ו��̐�

	int randomRange01;	// �ו���z�u���銄���̑傫��
	int randomRange02;	// �V�~�����[�V�����̌J��Ԃ���

	boardState mState;
};