#include "SolutionCodec.h"

#include <cctype>
#include <stdexcept>

namespace SolutionCodec
{
	namespace
	{
		constexpr std::size_t MAX_LURD_STEPS = 1000000;

		bool IsLurdChar(char c)
		{
			return c == 'r' || c == 'l' || c == 'u' || c == 'd'
				|| c == 'R' || c == 'L' || c == 'U' || c == 'D';
		}

		bool DirectionFromChar(char c, int& outDx, int& outDy, bool& outIsUpper)
		{
			outIsUpper = (c >= 'A' && c <= 'Z');
			const char lower = static_cast<char>(std::tolower(static_cast<unsigned char>(c)));
			switch (lower)
			{
			case 'r':
				outDx = 1;
				outDy = 0;
				return true;
			case 'l':
				outDx = -1;
				outDy = 0;
				return true;
			case 'u':
				outDx = 0;
				outDy = -1;
				return true;
			case 'd':
				outDx = 0;
				outDy = 1;
				return true;
			default:
				return false;
			}
		}

		bool ExpandLurdSegment(const std::string& input, std::size_t& pos, std::string& outMoves, std::string& outError, int depth)
		{
			if (depth > 16)
			{
				outError = "RLE nesting depth exceeded in LURD string";
				return false;
			}

			while (pos < input.size())
			{
				char c = input[pos];
				if (std::isspace(static_cast<unsigned char>(c)))
				{
					++pos;
					continue;
				}
				if (c == ')')
				{
					return true;
				}

				std::size_t repeatCount = 1;
				bool hasCount = false;
				if (std::isdigit(static_cast<unsigned char>(c)))
				{
					hasCount = true;
					repeatCount = 0;
					while (pos < input.size() && std::isdigit(static_cast<unsigned char>(input[pos])))
					{
						repeatCount = repeatCount * 10 + static_cast<std::size_t>(input[pos] - '0');
						if (repeatCount > MAX_LURD_STEPS)
						{
							outError = "Repeat count in LURD string is too large";
							return false;
						}
						++pos;
					}
					while (pos < input.size() && std::isspace(static_cast<unsigned char>(input[pos])))
					{
						++pos;
					}
					if (pos >= input.size())
					{
						outError = "Unexpected end of LURD string after repeat count";
						return false;
					}
					c = input[pos];
				}

				if (c == '(')
				{
					++pos;
					std::string inner;
					if (!ExpandLurdSegment(input, pos, inner, outError, depth + 1))
					{
						return false;
					}
					if (pos >= input.size() || input[pos] != ')')
					{
						outError = "Unclosed parenthesis in LURD string";
						return false;
					}
					++pos;
					if (!inner.empty() && repeatCount > (MAX_LURD_STEPS - outMoves.size()) / inner.size())
					{
						outError = "Expanded LURD string exceeds maximum step limit";
						return false;
					}
					for (std::size_t i = 0; i < repeatCount; ++i)
					{
						outMoves += inner;
					}
				}
				else if (IsLurdChar(c))
				{
					++pos;
					if (repeatCount > MAX_LURD_STEPS - outMoves.size())
					{
						outError = "Expanded LURD string exceeds maximum step limit";
						return false;
					}
					outMoves.append(repeatCount, c);
				}
				else
				{
					(void)hasCount;
					outError = std::string("Invalid LURD character: '") + c + "'";
					return false;
				}
			}

			return true;
		}
	}

	char EncodeMove(int dx, int dy, bool isPush)
	{
		char base = '\0';
		if (dx == 1 && dy == 0)
		{
			base = 'r';
		}
		else if (dx == -1 && dy == 0)
		{
			base = 'l';
		}
		else if (dx == 0 && dy == -1)
		{
			base = 'u';
		}
		else if (dx == 0 && dy == 1)
		{
			base = 'd';
		}
		else
		{
			return '\0';
		}
		return isPush ? static_cast<char>(std::toupper(static_cast<unsigned char>(base))) : base;
	}

	bool NormalizeLurdString(const std::string& rawInput, std::string& outMoves, std::string& outError)
	{
		outMoves.clear();
		outError.clear();

		std::size_t pos = 0;
		if (!ExpandLurdSegment(rawInput, pos, outMoves, outError, 0))
		{
			return false;
		}
		if (pos < rawInput.size())
		{
			outError = "Unexpected ')' in LURD string";
			return false;
		}
		return true;
	}

	SimulationResult SimulateLurd(const std::vector<std::string>& startBoard, const std::string& rawInput, bool strictCase)
	{
		SimulationResult result{};
		result.mIsValid = false;
		result.mReachesComplete = false;
		result.mHasCaseMismatchWarning = false;
		result.mErrorIndex = 0;
		result.mMoveCount = 0;
		result.mPushCount = 0;

		std::string moves;
		std::string parseError;
		if (!NormalizeLurdString(rawInput, moves, parseError))
		{
			result.mErrorMessage = parseError;
			return result;
		}

		if (startBoard.empty())
		{
			result.mErrorMessage = "Board is empty";
			return result;
		}

		// 盤面状態（地形と荷物・プレイヤー位置）を抽出
		const int height = static_cast<int>(startBoard.size());
		std::vector<std::string> terrain(height);
		std::vector<std::vector<bool>> hasBox(height);
		int playerX = -1;
		int playerY = -1;
		int boxCount = 0;

		for (int y = 0; y < height; ++y)
		{
			terrain[y].resize(startBoard[y].size(), ' ');
			hasBox[y].assign(startBoard[y].size(), false);
			for (int x = 0; x < static_cast<int>(startBoard[y].size()); ++x)
			{
				const char c = startBoard[y][x];
				switch (c)
				{
				case '#':
					terrain[y][x] = '#';
					break;
				case '.':
					terrain[y][x] = '.';
					break;
				case '$':
					terrain[y][x] = ' ';
					hasBox[y][x] = true;
					++boxCount;
					break;
				case '*':
					terrain[y][x] = '.';
					hasBox[y][x] = true;
					++boxCount;
					break;
				case '@':
					terrain[y][x] = ' ';
					playerX = x;
					playerY = y;
					break;
				case '+':
					terrain[y][x] = '.';
					playerX = x;
					playerY = y;
					break;
				default:
					terrain[y][x] = ' ';
					break;
				}
			}
		}

		if (playerX < 0 || playerY < 0)
		{
			result.mErrorMessage = "Player not found on start board";
			return result;
		}

		auto isInside = [&](int x, int y) {
			return y >= 0 && y < height && x >= 0 && x < static_cast<int>(terrain[y].size());
		};

		result.mSteps.reserve(moves.size());
		result.mNormalizedLurd.reserve(moves.size());

		for (std::size_t i = 0; i < moves.size(); ++i)
		{
			int dx = 0;
			int dy = 0;
			bool requestedPush = false;
			if (!DirectionFromChar(moves[i], dx, dy, requestedPush))
			{
				result.mErrorIndex = i;
				result.mErrorMessage = std::string("Invalid move character at step ") + std::to_string(i + 1);
				return result;
			}

			const int nextX = playerX + dx;
			const int nextY = playerY + dy;
			if (!isInside(nextX, nextY) || terrain[nextY][nextX] == '#')
			{
				result.mErrorIndex = i;
				result.mErrorMessage = "Illegal move into wall at step " + std::to_string(i + 1);
				return result;
			}

			const bool boxAtNext = hasBox[nextY][nextX];
			if (boxAtNext != requestedPush)
			{
				if (strictCase)
				{
					result.mErrorIndex = i;
					result.mErrorMessage = boxAtNext
						? ("Expected uppercase push move at step " + std::to_string(i + 1))
						: ("Expected lowercase walk move at step " + std::to_string(i + 1));
					return result;
				}
				result.mHasCaseMismatchWarning = true;
			}

			SolutionStep step{};
			step.mFromX = playerX;
			step.mFromY = playerY;
			step.mToX = nextX;
			step.mToY = nextY;
			step.mIsPush = boxAtNext;
			step.mBaggageFromX = 0;
			step.mBaggageFromY = 0;
			step.mBaggageToX = 0;
			step.mBaggageToY = 0;

			if (boxAtNext)
			{
				const int boxDestX = nextX + dx;
				const int boxDestY = nextY + dy;
				if (!isInside(boxDestX, boxDestY) || terrain[boxDestY][boxDestX] == '#')
				{
					result.mErrorIndex = i;
					result.mErrorMessage = "Illegal push into wall at step " + std::to_string(i + 1);
					return result;
				}
				if (hasBox[boxDestY][boxDestX])
				{
					result.mErrorIndex = i;
					result.mErrorMessage = "Illegal push into another box at step " + std::to_string(i + 1);
					return result;
				}

				hasBox[nextY][nextX] = false;
				hasBox[boxDestY][boxDestX] = true;
				step.mBaggageFromX = nextX;
				step.mBaggageFromY = nextY;
				step.mBaggageToX = boxDestX;
				step.mBaggageToY = boxDestY;
				++result.mPushCount;
			}

			playerX = nextX;
			playerY = nextY;
			step.mNormalizedChar = EncodeMove(dx, dy, boxAtNext);
			result.mNormalizedLurd.push_back(step.mNormalizedChar);
			result.mSteps.push_back(step);
			++result.mMoveCount;
		}

		// 全荷物がゴール上にあるか確認
		bool allOnGoal = (boxCount > 0);
		for (int y = 0; y < height && allOnGoal; ++y)
		{
			for (int x = 0; x < static_cast<int>(terrain[y].size()); ++x)
			{
				if (hasBox[y][x] && terrain[y][x] != '.')
				{
					allOnGoal = false;
					break;
				}
			}
		}

		result.mIsValid = true;
		result.mReachesComplete = allOnGoal;
		return result;
	}
}
