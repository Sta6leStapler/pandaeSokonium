#include "HUDHelper.h"
#include <queue>
#include <algorithm>

HUDHelper::HUDHelper(class Game* game)
	: mGame(game)
	, mBaggageNum(0)
{
	// �Ֆʂ̏�����
	mBoardStr = mGame->GetBoardData()[mGame->GetCurrentKey()];
	mInitialBoardStr = mBoardStr;
	// �ՖʃT�C�Y�̏�����
	mSize = mGame->GetBoardSize();
	// �v���C���[���W�̏�����
	mPlayerPos = mGame->GetPlayer()->GetBoardCoordinate();
	mInitialPlayerPos = mPlayerPos;
	// �ו��̍��W�Ɋւ��郁���o�ϐ��̏�����
	for (const auto& baggage : mGame->GetBaggages())
	{
		sf::Vector2i tmpPos{ baggage->GetBoardCoordinate() };
		int tmpIndex = mBaggagesPos.size();
		mBaggagesPos.emplace(tmpIndex, tmpPos);

		if (mBoardStr[tmpPos.y][tmpPos.x] == '$')
		{
			mNonGoaledBaggages.emplace(tmpIndex, tmpPos);
		}
		else if (mBoardStr[tmpPos.y][tmpPos.x] == '*')
		{
			mGoaledBaggages.emplace(tmpIndex, tmpPos);
		}
	}
	mInitialBaggagesPos = mBaggagesPos;
	mBaggageNum = mBaggagesPos.size();
	// �S�[�����W�̏�����
	for (int y = 0; y < mSize.y; ++y)
	{
		for (int x = 0; x < mSize.x; ++x)
		{
			if (mBoardStr[y][x] == '.' || mBoardStr[y][x] == '*' || mBoardStr[y][x] == '+')
			{
				mGoals.emplace_back(sf::Vector2i{ x, y });
			}
		}
	}
	// �v���C���[�Ɖו�����菜�����Ֆʏ���������
	mBoardMapStr = mBoardStr;
	for (const auto& baggage : mBaggagesPos)
	{
		if (mBoardMapStr[baggage.second.y][baggage.second.x] == '$')
		{
			mBoardMapStr[baggage.second.y][baggage.second.x] = ' ';
		}
		else if (mBoardMapStr[baggage.second.y][baggage.second.x] == '*')
		{
			mBoardMapStr[baggage.second.y][baggage.second.x] = '.';
		}
	}
	if (mBoardMapStr[mPlayerPos.y][mPlayerPos.x] == '@')
	{
		mBoardMapStr[mPlayerPos.y][mPlayerPos.x] = ' ';
	}
	else if (mBoardMapStr[mPlayerPos.y][mPlayerPos.x] == '+')
	{
		mBoardMapStr[mPlayerPos.y][mPlayerPos.x] = '.';
	}
	// �����܂ŔՖʂ̊�b�I���̏�����
	
	// �Ֆʍ\���̏�����
	mBoardStates.mGoaledBaggages = mGoaledBaggages;
	mBoardStates.mNonGoaledBaggages = mNonGoaledBaggages;
	mBoardStates.mMovableArea = GetMovableArea();
	mBoardStates.mDeadlockedBaggages = UpdateFreezeBaggagesMap();
	// �����ɏ����Ֆʂ�ǉ�
	std::set<std::pair<int, int>> baggagesSet{};
	for (const auto& baggage : mBaggagesPos)
	{
		baggagesSet.emplace(std::pair<int, int>{ baggage.second.x, baggage.second.y });
	}
	mHistory.emplace(std::pair<std::pair<int, int>, std::set<std::pair<int, int>>>{ std::pair<int, int>{ mPlayerPos.x, mPlayerPos.y }, baggagesSet }, mBoardStates);

}

HUDHelper::~HUDHelper()
{
	
}

void HUDHelper::Update()
{
	// �v���C���[����щו��̍��W�̍X�V�Ɋ�Â��e�����o�ϐ����X�V
	// �X�V���s���Ֆʊ֘A�̃����o�ϐ��͈ȉ�
	//	mBoardStr
	//	mPlayerPos
	//	mBaggagesPos
	//	mGoaledBaggages
	//	mNonGoaledBaggages
	//	mBoardStates
	//	mHistory
	// �v���C���[�Ɖו��̍X�V��̍��W���擾
	// �K���v���C���[�Ɖו��̃A�N�^�[�̍X�V�����̌�ɍs��
	sf::Vector2i nextPlayerPos{ mGame->GetPlayer()->GetBoardCoordinate() }, prevPlayerPos{ mPlayerPos };
	std::vector<sf::Vector2i> nextBaggages{ mGame->GetBaggagesPos() };
	std::unordered_map<int, sf::Vector2i> prevBaggagePos{ mBaggagesPos };
	std::set<std::pair<int, int>> nextBaggagesSet{};
	for (const auto& baggage : nextBaggages)
	{
		nextBaggagesSet.emplace(std::pair<int, int>{ baggage.x, baggage.y });
	}

	// �܂��ʒu�̕ς�����ו������邩�ǂ����𒲂ׂ�
	bool isMoved = false;
	std::unordered_map<int, sf::Vector2i> currentBaggages{ mBaggagesPos };
	std::vector<std::pair<std::pair<int, sf::Vector2i>, sf::Vector2i>> changedBaggages{};
	std::unordered_map<int, sf::Vector2i> tmpBaggages{ currentBaggages };
	for (const auto& currentBaggage : tmpBaggages)
	{
		// �X�V��̉ו��̃��X�g����A�Ή�����ו������݂��邩���ׂ�
		bool existsBaggage = false;
		for (const auto& nextBaggage : nextBaggages)
		{
			if (currentBaggage.second == nextBaggage)
			{
				existsBaggage = true;
				break;
			}
		}

		// �X�V��̉ו��̍��W�̃��X�g�ɁA���݂̑Ώۂɂ��Ă���ו��̍��W�����݂���Ȃ�΁A�X�V����ו��̃��X�g����폜
		if (existsBaggage)
		{
			currentBaggages.erase(currentBaggage.first);
			nextBaggages.erase(std::find(nextBaggages.begin(), nextBaggages.end(), currentBaggage.second));
		}
		// �����łȂ���΁A�ו��̈ʒu���ς�������ǂ����̔����^�ɂ���
		else
		{
			isMoved = true;
		}
	}

	// �ʒu�̍X�V�̂������ו��̐��ƁA�ו��̈ړ��O��̍��W���擾
	// note �ו��́u�{���́v�ړ��O�ƈړ���̈ʒu�̑Ή����������Ȃ��\�������邪�A���͂Ȃ��͂�
	int count = 0;
	for (const auto& baggage : currentBaggages)
	{
		changedBaggages.emplace_back(std::pair<std::pair<int, sf::Vector2i>, sf::Vector2i>{ baggage, nextBaggages[count] });
		++count;
	}

	// �p�^�[��
	// 1.1	�v���C���[���ړ����Ă��炸�A�ו����ړ����Ă��Ȃ�
	//		�v���C���[�Ɖו��ƔՖʍ\���̍X�V�����͍s��Ȃ�
	// 1.2	�v���C���[�͈ړ����Ă��Ȃ����A�ו��͈ړ����Ă��� (=���ۂ̓v���C���[���ړ�����)
	//		�ו��ƔՖʍ\���̍X�V�������s��
	// 2.1.	�v���C���[���ړ����A�ו��͈ړ����Ă��炸�A�v���C���[�̂��鏰�G���A���ς���Ă��Ȃ�
	//		�v���C���[�̍X�V�������s��
	// 2.2.	�v���C���[���ړ����A�ו��͈ړ����Ă��Ȃ����A�v���C���[�̂��鏰�G���A���ς����
	//		�v���C���[�ƔՖʍ\���̍X�V�������s��
	// 3.	�v���C���[���ړ����A�ו����ړ�����
	//		�S�Ă̍X�V�������s��
	// 
	// 1.2�͎����I��3�Ɠ����Ȃ̂ŁA�ו��̈ړ����猩��
	// �ו��̈ړ�������΁A1.2��3�̏������s���A�����łȂ���Ύ��̏�������Ɉڂ�
	// 1.1��2�̕�������� (�v���C���[���ړ����Ă��邩�ǂ������m�F)
	// 2�̒���2.1��2.2�̕��������
	
	/////////////////////////////////////////////////////////////////////////////

	std::pair<int, int> nextPlayerPosPair{ nextPlayerPos.x, nextPlayerPos.y };
	std::set<std::pair<int, int>> nextBaggagesPairSet{};
	for (const auto& baggage : nextBaggagesSet)
	{
		nextBaggagesPairSet.emplace(baggage);
	}
	// 1.2, 3		�ו����ړ����Ă��邩�ǂ����ŕ���
	// �ו����ړ����Ă���ꍇ
	if (count > 0)
	{
		// �ו���v���C���[�̔z�u���ς�����̂Ȃ�΁A�Ֆʊ֘A�̃����o�ϐ��̍X�V�Ƌl�ݔ�����s��
		// �����o�ϐ��̍X�V
		mPlayerPos = nextPlayerPos;
		// �ʒu�̕ς�����ו��̂ݍX�V
		for (const auto& item : changedBaggages)
		{
			mBaggagesPos[item.first.first] = item.second;

			// �S�[���ɔz�u����Ă��邩�ǂ������ς��ꍇ�A��������X�V
			// item��first.second���ړ��O�̈ʒu�Asecond���ړ���̈ʒu
			// �S�[�� -> ���ʂ̏��̏ꍇ
			// �ړ��O���S�[���ς݉ו��̃��X�g�ɑ��݂��A�ړ��オ�S�[�����X�g�ɑ��݂��Ȃ�
			if (mGoaledBaggages.count(item.first.first) && std::find(mGoals.begin(), mGoals.end(), item.second) == mGoals.end())
			{
				mGoaledBaggages.erase(item.first.first);
				mNonGoaledBaggages.emplace(item.first.first, item.second);
			}
			// ���ʂ̏� -> �S�[���̏ꍇ
			// �ړ��O�����S�[���ו��̃��X�g�ɑ��݂��A�ړ��オ�S�[�����X�g�ɑ��݂���
			else if (mNonGoaledBaggages.count(item.first.first) && std::find(mGoals.begin(), mGoals.end(), item.second) != mGoals.end())
			{
				mNonGoaledBaggages.erase(item.first.first);
				mGoaledBaggages.emplace(item.first.first, item.second);
			}
			// ���ʂ̏� -> ���ʂ̏��̏ꍇ
			// �ړ��O�����S�[���ו��̃��X�g�ɑ��݂��A�ړ��オ�S�[�����X�g�ɑ��݂��Ȃ�
			else if (mNonGoaledBaggages.count(item.first.first) && std::find(mGoals.begin(), mGoals.end(), item.second) == mGoals.end())
			{
				mNonGoaledBaggages[item.first.first] = item.second;
			}
			// �S�[�� -> �S�[���̏ꍇ
			// �ړ��O���S�[���ς݉ו��̃��X�g�ɑ��݂��A�ړ��オ�S�[�����X�g�ɑ��݂���
			else if (mGoaledBaggages.count(item.first.first) && std::find(mGoals.begin(), mGoals.end(), item.second) != mGoals.end())
			{
				mGoaledBaggages[item.first.first] = item.second;
			}

		}
		// �v���C���[�Ɖו��̈ʒu�̕ύX��Ֆʃf�[�^�ɔ��f������
		UpdateBoardStr();

		// �����Ɍ��݂̃v���C���[�Ɖו��̈ʒu�̑g�ݍ��킹�����邩���ׁA������Η����ɒǉ�
		// ���ɂ���Ȃ�΁A��������擾����
		if (!mHistory.count(std::pair<std::pair<int, int>, std::set<std::pair<int, int>>>{ nextPlayerPosPair, nextBaggagesPairSet }))
		{
			mBoardStates.mMovableArea = GetMovableArea();
			mBoardStates.mDeadlockedBaggages = UpdateFreezeBaggagesMap();
			mBoardStates.mGoaledBaggages = mGoaledBaggages;
			mBoardStates.mNonGoaledBaggages = mNonGoaledBaggages;
			mHistory.emplace(std::pair<std::pair<int, int>, std::set<std::pair<int, int>>>{ nextPlayerPosPair, nextBaggagesPairSet }, mBoardStates);
		}
		else;
		{
			mBoardStates = mHistory[std::pair<std::pair<int, int>, std::set<std::pair<int, int>>>{ nextPlayerPosPair, nextBaggagesPairSet }];
		}
	}
	// �ו����ړ����Ă��Ȃ��ꍇ
	// 2.1, 2.2	�v���C���[���ړ����Ă��邩�ŕ���
	else if (mPlayerPos != nextPlayerPos)
	{
		// �v���C���[�̂��鏰�G���A���ς�������ŕ���
		// 2.1	�ς���Ă��Ȃ��ꍇ
		if (isMovableArea(mPlayerPos, nextPlayerPos))
		{
			// �����o�ϐ��̍X�V
			mPlayerPos = nextPlayerPos;
			// �v���C���[�Ɖו��̈ʒu�̕ύX��Ֆʃf�[�^�ɔ��f������
			UpdateBoardStr();

			// �ו����ړ����Ă��炸�A�v���C���[�����G���A���ς���Ă��Ȃ����߁A�Ֆʍ\�� (mBoardStates) �����̂܂�
			// �������A�v���C���[�͈ړ����Ă���̂ŁA�����ɂ͒ǉ�����
			// �����Ɍ��݂̃v���C���[�Ɖו��̈ʒu�̑g�ݍ��킹�����邩���ׁA������Η����ɒǉ�
			// ���ɂ���Ȃ�΁A��������擾����
			if (!mHistory.count(std::pair<std::pair<int, int>, std::set<std::pair<int, int>>>{ nextPlayerPosPair, nextBaggagesPairSet }))
			{
				mHistory.emplace(std::pair<std::pair<int, int>, std::set<std::pair<int, int>>>{ nextPlayerPosPair, nextBaggagesPairSet }, mBoardStates);
			}
			else
			{
				mBoardStates = mHistory[std::pair<std::pair<int, int>, std::set<std::pair<int, int>>>{ nextPlayerPosPair, nextBaggagesPairSet }];
			}

		}
		// 2.2	�ς�����ꍇ
		else
		{
			// �����o�ϐ��̍X�V
			mPlayerPos = nextPlayerPos;
			// �v���C���[�Ɖו��̈ʒu�̕ύX��Ֆʃf�[�^�ɔ��f������
			UpdateBoardStr();

			// �v���C���[�̏��G���A���ς�����̂ŁA�Ֆʍ\�� (mBoardStates) ���X�V���ė����ɒǉ�
			// �����Ɍ��݂̃v���C���[�Ɖו��̈ʒu�̑g�ݍ��킹�����邩���ׁA������Η����ɒǉ�
			// ���ɂ���Ȃ�΁A��������擾����
			if (!mHistory.count(std::pair<std::pair<int, int>, std::set<std::pair<int, int>>>{ nextPlayerPosPair, nextBaggagesPairSet }))
			{
				mBoardStates.mMovableArea = GetMovableArea();
				mBoardStates.mDeadlockedBaggages = UpdateFreezeBaggagesMap();
				mBoardStates.mGoaledBaggages = mGoaledBaggages;
				mBoardStates.mNonGoaledBaggages = mNonGoaledBaggages;
				mHistory.emplace(std::pair<std::pair<int, int>, std::set<std::pair<int, int>>>{ nextPlayerPosPair, nextBaggagesPairSet }, mBoardStates);
			}
			else
			{
				mBoardStates = mHistory[std::pair<std::pair<int, int>, std::set<std::pair<int, int>>>{ nextPlayerPosPair, nextBaggagesPairSet }];
			}
		}
	}
}

bool HUDHelper::isDeadlockedBaggage(const sf::Vector2i& pos)
{
	for (const auto& baggage : mBoardStates.mDeadlockedBaggages)
	{
		if (baggage.second == pos)
		{
			return true;
		}
	}

	return false;
}

HUDHelper::Direction HUDHelper::RollDirection()
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

sf::Vector2i HUDHelper::RollCoordinate()
{
	return sf::Vector2i{ RollCoordinateX(), RollCoordinateY() };
}

sf::Vector2i HUDHelper::RollCoordinate(const int& xMin, const int& xMax, const int& yMin, const int& yMax)
{
	return sf::Vector2i{ RollCoordinateX(xMin, xMax), RollCoordinateY(yMin, yMax) };
}

int HUDHelper::RollCoordinateX()
{
	std::uniform_int_distribution<int> randX(1, mSize.x - 2);
	// �����𐶐����� 
	return randX(mt);
}

int HUDHelper::RollCoordinateX(const int& xMin, const int& xMax)
{
	std::uniform_int_distribution<int> randX(xMin, xMax);
	// �����𐶐����� 
	return randX(mt);
}

int HUDHelper::RollCoordinateY()
{
	std::uniform_int_distribution<int> randX(1, mSize.y - 2);
	// �����𐶐����� 
	return randX(mt);
}

int HUDHelper::RollCoordinateY(const int& yMin, const int& yMax)
{
	std::uniform_int_distribution<int> randX(yMin, yMax);
	// �����𐶐����� 
	return randX(mt);
}

template <typename T>
T HUDHelper::GetRandomElement(const std::vector<T>& vec)
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
std::pair<T, U> HUDHelper::GetRandomElement(const std::unordered_map<T, U>& map)
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

std::vector<sf::Vector2i> HUDHelper::GetRandomDirections()
{
	std::vector<sf::Vector2i> output = directions;
	std::shuffle(output.begin(), output.end(), mt);
	return output;
}

bool HUDHelper::isArrangementValid()
{
	// �e�X�̉ו����l�݂��ǂ����m�F����
	for (const auto baggagePos : mBaggagesPos)
	{
		if (isFreezeBaggage(baggagePos.second))
		{
			return false;
		}
	}

	return true;
}

std::unordered_map<int, sf::Vector2i> HUDHelper::UpdateFreezeBaggagesMap()
{
	std::unordered_map<int, sf::Vector2i> result{};

	// �l�݂̉ו������X�g�ɒǉ����Ă���
	for (const auto baggagePos : mBaggagesPos)
	{
		if (isFreezeBaggage(baggagePos.second))
		{
			result.emplace(baggagePos.first, baggagePos.second);
		}
	}

	return result;
}

std::vector<sf::Vector2i> HUDHelper::GetCandidates()
{
	// �������Ƃ̂ł���ו��̌����擾
	std::vector<sf::Vector2i> result{};
	for (const auto& baggage : mBaggagesPos)
	{
		std::set<std::pair<int, int>> targetPosSet{};

		// �ו��̏㉺�ƍ��E�Ɉړ��̗]�T�����邩�𒲂ׂ�
		if ((mBoardStr[baggage.second.y][baggage.second.x + 1] == ' ' ||
			mBoardStr[baggage.second.y][baggage.second.x + 1] == '.' ||
			mBoardStr[baggage.second.y][baggage.second.x + 1] == '@' ||
			mBoardStr[baggage.second.y][baggage.second.x + 1] == '+') &&
			(mBoardStr[baggage.second.y][baggage.second.x - 1] == ' ' ||
			mBoardStr[baggage.second.y][baggage.second.x - 1] == '.' ||
			mBoardStr[baggage.second.y][baggage.second.x - 1] == '@' ||
			mBoardStr[baggage.second.y][baggage.second.x - 1] == '+'))
		{
			targetPosSet.emplace(std::pair<int, int>{ baggage.second.y, baggage.second.x + 1 });
			targetPosSet.emplace(std::pair<int, int>{ baggage.second.y, baggage.second.x - 1 });
		}
		if ((mBoardStr[baggage.second.y + 1][baggage.second.x] == ' ' ||
			mBoardStr[baggage.second.y + 1][baggage.second.x] == '.' ||
			mBoardStr[baggage.second.y + 1][baggage.second.x] == '@' ||
			mBoardStr[baggage.second.y + 1][baggage.second.x] == '+') &&
			(mBoardStr[baggage.second.y - 1][baggage.second.x] == ' ' ||
			mBoardStr[baggage.second.y - 1][baggage.second.x] == '.' ||
			mBoardStr[baggage.second.y - 1][baggage.second.x] == '@' ||
			mBoardStr[baggage.second.y - 1][baggage.second.x] == '+'))
		{
			targetPosSet.emplace(std::pair<int, int>{ baggage.second.y + 1, baggage.second.x });
			targetPosSet.emplace(std::pair<int, int>{ baggage.second.y - 1, baggage.second.x });
		}

		if (targetPosSet.empty())
		{
			continue;
		}

		std::vector<std::vector<bool>> explored(mSize.y, std::vector<bool>(mSize.x, false));
		explored[mPlayerPos.y][mPlayerPos.x] = true;

		std::queue<std::pair<int, int>> q{};
		q.push(std::pair<int, int>{ mPlayerPos.x, mPlayerPos.y });

		while (!q.empty())
		{
			std::pair<int, int> current{ q.front() };
			q.pop();

			if (targetPosSet.count(current))
			{
				result.emplace_back(baggage.second);
				break;
			}

			for (const auto& dir : directions)
			{
				std::pair<int, int> next{ current.first + dir.x, current.second + dir.y };

				if (!explored[next.second][next.first])
				{
					if (mBoardStr[next.second][next.first] == ' ' || mBoardStr[next.second][next.first] == '.' || mBoardStr[next.second][next.first] == '@' || mBoardStr[next.second][next.first] == '+')
					{
						explored[next.second][next.first] = true;
						q.push(next);
					}
				}
			}
		}
	}
	
	return result;
}

bool HUDHelper::isFreezeBaggage(const sf::Vector2i& baggagePos)
{
	// �����܂ł��̉ו����g���l�݂��ǂ����𔻕ʂ���֐��ł���A�ו��S�̂̔z�u����l�ݔ�����s���킯�ł͂Ȃ�
	// �ו��̎��͂̒n�`����l�ݔ�����s�������ƁA�ו����S�[���Ɉړ��ł��邩�ǂ����ŋl�ݔ�����s��������2�X�e�b�v����Ȃ�

	// �S�[���ʒu�ɂ���ו��͋l�݂ł͂Ȃ�
	if (std::find(mGoals.begin(), mGoals.end(), baggagePos) != mGoals.end())
	{
		// ������false��Ԃ��̂́A�ו����S�[���ɓ��B�ł���ȏ�A�l�݂ł���ƒf��ł��Ȃ�����
		// �l�݂łȂ����Ƃ�ۏ؂�����̂ł͂Ȃ�
		// NOTE ����āA�l�ݔ���̏����̕ύX�ɔ����A�����̏�����ς���\���͂���
		return false;
	}

	// �Ֆʂ̊p�̉ו��͓������Ȃ�
	if (baggagePos == sf::Vector2i{ 1, 1 } && baggagePos == sf::Vector2i{ mSize.x, 1 } || baggagePos == sf::Vector2i{ 1, mSize.y } || baggagePos == sf::Vector2i{ mSize.x, mSize.y })
	{
		return true;
	}

	// �ו��̎��͂̒n�`�����āA�������邩�ǂ����𒲂ׂ�
	// TODO �ו��𓮂����Ȃ��`��̒��ɁA1�ł��S�[����ԂłȂ��ו�������Ă�����OUT
	for (int i = 0; i < directions.size(); i++)
	{
		// $$ ## #$ #$ $#
		// $$ $$ $# $$ ## �̌`��
		if ((mBoardStr[baggagePos.y + directions[i].y][baggagePos.x + directions[i].x] == '#' || mBoardStr[baggagePos.y + directions[i].y][baggagePos.x + directions[i].x] == '$') &&
			(mBoardStr[baggagePos.y + directions[i].y + directions[(i + 1) % 4].y][baggagePos.x + directions[i].x + directions[(i + 1) % 4].x] == '#' || mBoardStr[baggagePos.y + directions[i].y + directions[(i + 1) % 4].y][baggagePos.x + directions[i].x + directions[(i + 1) % 4].x] == '$') &&
			(mBoardStr[baggagePos.y + directions[(i + 1) % 4].y][baggagePos.x + directions[(i + 1) % 4].x] == '#' || mBoardStr[baggagePos.y + directions[(i + 1) % 4].y][baggagePos.x + directions[(i + 1) % 4].x] == '$'))
		{
			return true;
		}

		// $#
		// #  �̌`��
		if (mBoardStr[baggagePos.y + directions[i].y][baggagePos.x + directions[i].x] == '#' &&
			mBoardStr[baggagePos.y + directions[(i + 1) % 4].y][baggagePos.x + directions[(i + 1) % 4].x] == '#')
		{
			return true;
		}

		//  $#
		// #$  �̌`��
		if (mBoardStr[baggagePos.y + directions[i].y][baggagePos.x + directions[i].x] == '#' &&
			mBoardStr[baggagePos.y + directions[(i + 1) % 4].y][baggagePos.x + directions[(i + 1) % 4].x] == '$' &&
			mBoardStr[baggagePos.y + directions[(i + 1) % 4].y + directions[(i + 2) % 4].y][baggagePos.x + directions[(i + 1) % 4].x + directions[(i + 2) % 4].x] == '#')
		{
			return true;
		}

		// #$
		//  $# �̌`��
		if (mBoardStr[baggagePos.y + directions[i].y + directions[(i + 1) % 4].y][baggagePos.x + directions[i].x + directions[(i + 1) % 4].x] == '#' &&
			mBoardStr[baggagePos.y + directions[(i + 1) % 4].y][baggagePos.x + directions[(i + 1) % 4].x] == '$' &&
			mBoardStr[baggagePos.y + directions[(i + 2) % 4].y][baggagePos.x + directions[(i + 2) % 4].x] == '#')
		{
			return true;
		}

		// #
		// $$
		//  $# �̌`��
		if (mBoardStr[baggagePos.y + directions[i].y + directions[(i + 1) % 4].y][baggagePos.x + directions[i].x + directions[(i + 1) % 4].x] == '#' &&
			mBoardStr[baggagePos.y + directions[(i + 1) % 4].y][baggagePos.x + directions[(i + 1) % 4].x] == '$' &&
			mBoardStr[baggagePos.y + directions[(i + 2) % 4].y][baggagePos.x + directions[(i + 2) % 4].x] == '$' &&
			mBoardStr[baggagePos.y + directions[(i + 2) % 4].y + directions[(i + 3) % 4].y][baggagePos.x + directions[(i + 2) % 4].x + directions[(i + 3) % 4].x] == '#')
		{
			return true;
		}
	}

	// �ו��̈ړ��\�Ȕ͈͓��ɃS�[�������邩���ׂ�
	// �S�[���ɓ��B�ł��Ȃ��̂Ȃ�΋l�݂ŁA�����łȂ���΋l�݂��ǂ����͂킩��Ȃ�
	// �T���p�̔Ֆ� (�ו��ƃv���C���[����菜�����Ֆʂ�p����)
	bool nonExistsRoute = true;
	Board currentBoard = mBoardMapStr;
	std::set<std::pair<int, int>> targetPosSet{};

	// �ו��̏㉺�ƍ��E�Ɉړ��̗]�T�����邩�𒲂ׂ�
	// ���̏ꏊ�Ƀv���C���[���ړ��ł���Ƃ͌���Ȃ�
	if ((currentBoard[baggagePos.y][baggagePos.x + 1] == ' ' ||
		currentBoard[baggagePos.y][baggagePos.x + 1] == '.' ||
		currentBoard[baggagePos.y][baggagePos.x + 1] == '@' ||
		currentBoard[baggagePos.y][baggagePos.x + 1] == '+') &&
		(currentBoard[baggagePos.y][baggagePos.x - 1] == ' ' ||
		currentBoard[baggagePos.y][baggagePos.x - 1] == '.' ||
		currentBoard[baggagePos.y][baggagePos.x - 1] == '@' ||
		currentBoard[baggagePos.y][baggagePos.x - 1] == '+'))
	{
		targetPosSet.emplace(std::pair<int, int>{ baggagePos.x + 1, baggagePos.y });
		targetPosSet.emplace(std::pair<int, int>{ baggagePos.x - 1, baggagePos.y });
	}
	if ((currentBoard[baggagePos.y + 1][baggagePos.x] == ' ' ||
		currentBoard[baggagePos.y + 1][baggagePos.x] == '.' ||
		currentBoard[baggagePos.y + 1][baggagePos.x] == '@' ||
		currentBoard[baggagePos.y + 1][baggagePos.x] == '+') &&
		(currentBoard[baggagePos.y - 1][baggagePos.x] == ' ' ||
		currentBoard[baggagePos.y - 1][baggagePos.x] == '.' ||
		currentBoard[baggagePos.y - 1][baggagePos.x] == '@' ||
		currentBoard[baggagePos.y - 1][baggagePos.x] == '+'))
	{
		targetPosSet.emplace(std::pair<int, int>{ baggagePos.x, baggagePos.y + 1 });
		targetPosSet.emplace(std::pair<int, int>{ baggagePos.x, baggagePos.y - 1 });
	}

	// �ו��̏㉺�������͍��E�Ɉړ��\�ȗ]�T������Α��s
	if (!targetPosSet.empty())
	{
		// �T���p�̔ՖʂɃv���C���[�Ɖו���ݒu����
		currentBoard[mPlayerPos.y][mPlayerPos.x] = '@';
		currentBoard[baggagePos.y][baggagePos.x] = '$';

		// �ו��̉^�����V�~�����[�g���邽�߂̃v���C���[�̏����ʒu���擾����
		// ��̓I�ɂ́A�ו��̉^���J�n�ʒu�Ƀv���C���[���ړ��ł��邩�𒲂ׂ�
		bool existsRoute = false;
		for (const auto& targetPos : targetPosSet)
		{
			if (isMovableArea(mPlayerPos, sf::Vector2i{ targetPos.first, targetPos.second }, currentBoard))
			{
				existsRoute = true;
				break;
			}
		}

		// �ו��̏㉺���E�̗]�T�Ƀv���C���[���ړ��\�Ȃ�Α��s
		if (existsRoute)
		{
			// ���������������T������
			// ex. explored[4][7][1] = true �Ȃ�A(7,4)�̈ʒu�ɂ���ו����������ɓ��������������ɂ������Ƃ�\��
			std::vector<std::vector<std::vector<bool>>> explored(mSize.y, std::vector<std::vector<bool>>(mSize.x, std::vector<bool>(directions.size(), false)));
			
			// ���D��T���ɗp����L���[�̗v�f
			struct CurrentState
			{
				Board mBoard;
				sf::Vector2i mPlayer;
				sf::Vector2i mBaggage;

				// ��r���Z�q�̒�`
				bool operator<(const CurrentState& other) const
				{
					if (mPlayer.x < other.mPlayer.x) return true;
					if (mPlayer.y < other.mPlayer.y) return true;
					if (other.mPlayer.x < mPlayer.x) return false;
					if (other.mPlayer.y < mPlayer.y) return false;
					if (mBaggage.x < other.mBaggage.x) return true;
					if (mBaggage.y < other.mBaggage.y) return true;
					if (other.mBaggage.x < mBaggage.x) return false;
					return mBaggage.y < other.mBaggage.y;
				}
			};

			std::queue<CurrentState> q{};
			q.push(CurrentState{ currentBoard, mPlayerPos, baggagePos });

			while (!q.empty())
			{
				CurrentState current = q.front();
				q.pop();

				if (std::find(mGoals.begin(), mGoals.end(), current.mBaggage) != mGoals.end())
				{
					// ������false��Ԃ��̂́A�ו����S�[���ɓ��B�ł���ȏ�A�l�݂ł���ƒf��ł��Ȃ�����
					// �l�݂łȂ����Ƃ�ۏ؂�����̂ł͂Ȃ�
					// NOTE ����āA�l�ݔ���̏����̕ύX�ɔ����A�����̏�����ς���\���͂���
					nonExistsRoute = false;
				}

				for (const auto& dir : directions)
				{
					sf::Vector2i nextBaggagePos{ current.mBaggage + dir };
					if (!explored[current.mBaggage.y][current.mBaggage.x][GetDirectionIndex(dir)])
					{
						// �ו��̎��̈ړ��悪�ǃ}�X�łȂ��A���v���C���[���^���J�n�ʒu�Ɉړ��ł���̂Ȃ�΁A�L���[�ɒǉ�
						if (current.mBoard[nextBaggagePos.y][nextBaggagePos.x] != '#')
						{
							if (isMovableArea(current.mPlayer, current.mBaggage - dir, current.mBoard))
							{
								Board nextBoard{ current.mBoard };
								nextBoard[current.mPlayer.y][current.mPlayer.x] = ' ';
								nextBoard[current.mBaggage.y][current.mBaggage.x] = '@';
								nextBoard[nextBaggagePos.y][nextBaggagePos.x] = '$';
								CurrentState next{ nextBoard, current.mBaggage, nextBaggagePos };

								explored[current.mBaggage.y][current.mBaggage.x][GetDirectionIndex(dir)] = true;
								q.push(next);
							}
						}
					}
				}
			}
		}
	}

	if (nonExistsRoute)
	{
		return true;
	}

	return false;
}

bool HUDHelper::isMovableArea(const sf::Vector2i& pos1, const sf::Vector2i& pos2)
{
	std::vector<std::vector<bool>> explored(mSize.y, std::vector<bool>(mSize.x, false));
	explored[pos1.y][pos1.x] = true;

	std::queue<std::pair<int, int>> q{};
	q.push(std::pair<int, int>{ pos1.x, pos1.y });

	while (!q.empty())
	{
		std::pair<int, int> current{ q.front() };
		q.pop();

		if (current == std::pair<int, int>{ pos2.x, pos2.y })
		{
			return true;
		}

		for (const auto& dir : directions)
		{
			std::pair<int, int> next{ current.first + dir.x, current.second + dir.y };

			if (!explored[next.second][next.first])
			{
				if (mBoardStr[next.second][next.first] == ' ' || mBoardStr[next.second][next.first] == '.' || mBoardStr[next.second][next.first] == '@' || mBoardStr[next.second][next.first] == '+')
				{
					explored[next.second][next.first] = true;
					q.push(next);
				}
			}
		}
	}

	return false;
}

bool HUDHelper::isMovableArea(const sf::Vector2i& pos1, const sf::Vector2i& pos2, const Board& board)
{
	std::vector<std::vector<bool>> explored(mSize.y, std::vector<bool>(mSize.x, false));
	explored[pos1.y][pos1.x] = true;

	std::queue<std::pair<int, int>> q{};
	q.push(std::pair<int, int>{ pos1.x, pos1.y });

	while (!q.empty())
	{
		std::pair<int, int> current{ q.front() };
		q.pop();

		if (current == std::pair<int, int>{ pos2.x, pos2.y })
		{
			return true;
		}

		for (const auto& dir : directions)
		{
			std::pair<int, int> next{ current.first + dir.x, current.second + dir.y };

			if (!explored[next.second][next.first])
			{
				if (board[next.second][next.first] == ' ' || board[next.second][next.first] == '.' || board[next.second][next.first] == '@' || board[next.second][next.first] == '+')
				{
					explored[next.second][next.first] = true;
					q.push(next);
				}
			}
		}
	}

	return false;
}

std::vector<sf::Vector2i> HUDHelper::GetMovableArea()
{
	std::vector<sf::Vector2i> result{ mPlayerPos };
	std::vector<std::vector<bool>> explored(mSize.y, std::vector<bool>(mSize.x, false));
	explored[mPlayerPos.y][mPlayerPos.x] = true;

	std::queue<std::pair<int, int>> q{};
	q.push(std::pair<int, int>{ mPlayerPos.x, mPlayerPos.y });

	while (!q.empty())
	{
		std::pair<int, int> current{ q.front() };
		q.pop();

		for (const auto& dir : directions)
		{
			std::pair<int, int> next{ current.first + dir.x, current.second + dir.y };

			if (!explored[next.second][next.first])
			{
				if (mBoardStr[next.second][next.first] == ' ' || mBoardStr[next.second][next.first] == '.' || mBoardStr[next.second][next.first] == '@' || mBoardStr[next.second][next.first] == '+')
				{
					explored[next.second][next.first] = true;
					q.push(next);
					result.emplace_back(sf::Vector2i{ next.first, next.second });
				}
			}
		}
	}

	return result;
}

void HUDHelper::UpdateBoardStr()
{
	// ��U�Ֆʏ�̃v���C���[�Ɖו����폜
	mBoardStr = mBoardMapStr;

	// ���ɍX�V����Ă���v���C���[�Ɖו��̈ʒu�ɔz�u
	if (std::find(mGoals.begin(), mGoals.end(), mPlayerPos) == mGoals.end())
	{
		mBoardStr[mPlayerPos.y][mPlayerPos.x] = '@';
	}
	else
	{
		mBoardStr[mPlayerPos.y][mPlayerPos.x] = '+';
	}
	for (const auto& mBaggagePos : mGoaledBaggages)
	{
		mBoardStr[mBaggagePos.second.y][mBaggagePos.second.x] = '*';
	}
	for (const auto& mBaggagePos : mNonGoaledBaggages)
	{
		mBoardStr[mBaggagePos.second.y][mBaggagePos.second.x] = '$';
	}
	
}