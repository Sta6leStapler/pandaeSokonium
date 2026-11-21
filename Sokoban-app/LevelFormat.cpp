#include "LevelFormat.h"
#include "AppPaths.h"

#include <algorithm>
#include <cctype>
#include <sstream>
#include <stdexcept>

namespace LevelFormat
{
	namespace
	{
		constexpr std::size_t MAX_EXPANDED_BOARD_CHARS = 262144;

		std::string ToLowerAscii(std::string value)
		{
			std::transform(value.begin(), value.end(), value.begin(), [](unsigned char c) {
				return static_cast<char>(std::tolower(c));
			});
			return value;
		}

		std::string Trim(const std::string& value)
		{
			const std::size_t first = value.find_first_not_of(" \t\r\n");
			if (first == std::string::npos)
			{
				return "";
			}
			const std::size_t last = value.find_last_not_of(" \t\r\n");
			return value.substr(first, last - first + 1);
		}

		std::string TrimRight(const std::string& value)
		{
			const std::size_t last = value.find_last_not_of(" \t\r\n");
			if (last == std::string::npos)
			{
				return "";
			}
			return value.substr(0, last + 1);
		}

		bool IsBoardChar(char c)
		{
			return c == '#' || c == ' ' || c == '-' || c == '_'
				|| c == '$' || c == '.' || c == '*' || c == '@' || c == '+';
		}

		bool IsPlainBoardLine(const std::string& line)
		{
			if (line.empty())
			{
				return false;
			}
			bool hasWallOrPiece = false;
			for (char c : line)
			{
				if (!IsBoardChar(c))
				{
					return false;
				}
				if (c == '#' || c == '$' || c == '.' || c == '*' || c == '@' || c == '+')
				{
					hasWallOrPiece = true;
				}
			}
			return hasWallOrPiece;
		}

		bool IsRleBoardLine(const std::string& line)
		{
			const std::string trimmed = Trim(line);
			if (trimmed.empty())
			{
				return false;
			}
			bool hasWall = false;
			bool hasRleToken = false;
			for (char c : trimmed)
			{
				if (IsBoardChar(c))
				{
					if (c == '#')
					{
						hasWall = true;
					}
					continue;
				}
				if (std::isdigit(static_cast<unsigned char>(c)) || c == '|' || c == '(' || c == ')')
				{
					hasRleToken = true;
					continue;
				}
				return false;
			}
			return hasWall && hasRleToken;
		}

		bool IsLurdLine(const std::string& line)
		{
			const std::string trimmed = Trim(line);
			if (trimmed.empty())
			{
				return false;
			}
			bool hasDirection = false;
			for (char c : trimmed)
			{
				if (c == ' ' || c == '\t' || c == '(' || c == ')' || std::isdigit(static_cast<unsigned char>(c)))
				{
					continue;
				}
				const char lower = static_cast<char>(std::tolower(static_cast<unsigned char>(c)));
				if (lower == 'r' || lower == 'l' || lower == 'u' || lower == 'd')
				{
					hasDirection = true;
					continue;
				}
				return false;
			}
			return hasDirection;
		}

		bool StartsWithKeyIgnoreCase(const std::string& line, const std::string& key, std::string& outValue)
		{
			const std::size_t colonPos = line.find(':');
			if (colonPos == std::string::npos)
			{
				return false;
			}
			const std::string head = ToLowerAscii(Trim(line.substr(0, colonPos)));
			if (head != key)
			{
				return false;
			}
			outValue = Trim(line.substr(colonPos + 1));
			return true;
		}

		bool ExpandRleSegment(const std::string& input, std::size_t& pos, std::string& outExpanded, std::string& outError, int depth)
		{
			if (depth > 16)
			{
				outError = "RLE nesting depth exceeded";
				return false;
			}

			while (pos < input.size())
			{
				char c = input[pos];
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
						if (repeatCount > MAX_EXPANDED_BOARD_CHARS)
						{
							outError = "RLE repeat count is too large";
							return false;
						}
						++pos;
					}
					if (pos >= input.size())
					{
						outError = "Unexpected end of RLE sequence after repeat count";
						return false;
					}
					c = input[pos];
				}

				if (c == '(')
				{
					++pos;
					std::string inner;
					if (!ExpandRleSegment(input, pos, inner, outError, depth + 1))
					{
						return false;
					}
					if (pos >= input.size() || input[pos] != ')')
					{
						outError = "Unclosed parenthesis in RLE sequence";
						return false;
					}
					++pos;
					if (inner.size() > 0 && repeatCount > (MAX_EXPANDED_BOARD_CHARS - outExpanded.size()) / inner.size())
					{
						outError = "Expanded RLE board exceeds maximum size";
						return false;
					}
					for (std::size_t i = 0; i < repeatCount; ++i)
					{
						outExpanded += inner;
					}
				}
				else if (IsBoardChar(c) || c == '|')
				{
					++pos;
					if (repeatCount > MAX_EXPANDED_BOARD_CHARS - outExpanded.size())
					{
						outError = "Expanded RLE board exceeds maximum size";
						return false;
					}
					outExpanded.append(repeatCount, c);
				}
				else if (!hasCount && (c == '\r' || c == '\n' || c == '\t'))
				{
					++pos;
				}
				else
				{
					outError = std::string("Invalid character in RLE board: '") + c + "'";
					return false;
				}
			}

			return true;
		}
	}

	bool IsSupportedExtension(const std::filesystem::path& path)
	{
		const std::string ext = ToLowerAscii(path.extension().u8string());
		return ext == ".txt" || ext == ".xsb" || ext == ".sok";
	}

	std::string DetectFormatFromPath(const std::filesystem::path& path)
	{
		const std::string ext = ToLowerAscii(path.extension().u8string());
		if (ext == ".sok")
		{
			return "sok";
		}
		if (ext == ".xsb")
		{
			return "xsb";
		}
		return "txt";
	}

	bool ExpandBoardRle(const std::string& input, std::vector<std::string>& outRows, std::string& outError)
	{
		outRows.clear();
		outError.clear();

		std::size_t pos = 0;
		std::string expanded;
		if (!ExpandRleSegment(input, pos, expanded, outError, 0))
		{
			return false;
		}
		if (pos < input.size())
		{
			outError = "Unexpected ')' in RLE board";
			return false;
		}

		std::string currentRow;
		for (char c : expanded)
		{
			if (c == '|')
			{
				outRows.push_back(currentRow);
				currentRow.clear();
			}
			else
			{
				currentRow.push_back(c);
			}
		}
		if (!currentRow.empty() || outRows.empty())
		{
			outRows.push_back(currentRow);
		}
		return true;
	}

	std::vector<std::string> NormalizeBoard(const std::vector<std::string>& rawRows)
	{
		std::vector<std::string> normalized;
		normalized.reserve(rawRows.size());

		std::size_t maxWidth = 0;
		for (const auto& rawRow : rawRows)
		{
			std::string row;
			row.reserve(rawRow.size());
			for (char c : rawRow)
			{
				if (c == '-' || c == '_')
				{
					row.push_back(' ');
				}
				else if (c != '\r' && c != '\n')
				{
					row.push_back(c);
				}
			}
			// 行末の余分な空白を一旦整える
			while (!row.empty() && row.back() == ' ')
			{
				row.pop_back();
			}
			maxWidth = std::max(maxWidth, row.size());
			normalized.push_back(std::move(row));
		}

		// 全行を最大幅へ右スペースパディング
		for (auto& row : normalized)
		{
			if (row.size() < maxWidth)
			{
				row.append(maxWidth - row.size(), ' ');
			}
		}
		return normalized;
	}

	bool ValidateBoard(const std::vector<std::string>& board, std::string& outError)
	{
		outError.clear();
		if (board.empty() || board.front().empty())
		{
			outError = "Board is empty";
			return false;
		}

		const std::size_t width = board.front().size();
		int playerCount = 0;
		int baggageCount = 0;
		int goalCount = 0;
		int wallCount = 0;

		for (const auto& row : board)
		{
			if (row.size() != width)
			{
				outError = "Board rows do not have uniform width";
				return false;
			}
			for (char c : row)
			{
				switch (c)
				{
				case ' ':
					break;
				case '#':
					++wallCount;
					break;
				case '$':
					++baggageCount;
					break;
				case '.':
					++goalCount;
					break;
				case '*':
					++baggageCount;
					++goalCount;
					break;
				case '@':
					++playerCount;
					break;
				case '+':
					++playerCount;
					++goalCount;
					break;
				default:
					outError = std::string("Invalid board character: '") + c + "'";
					return false;
				}
			}
		}

		if (wallCount == 0)
		{
			outError = "Board must contain walls";
			return false;
		}
		if (playerCount != 1)
		{
			outError = "Board must contain exactly one player";
			return false;
		}
		if (baggageCount == 0)
		{
			outError = "Board must contain at least one box";
			return false;
		}
		if (baggageCount != goalCount)
		{
			outError = "Box count and goal count do not match";
			return false;
		}
		return true;
	}

	LevelCollection ParseCollection(const std::string& text, const std::filesystem::path& sourcePath)
	{
		LevelCollection collection;
		collection.mFilePath = sourcePath;
		if (!sourcePath.empty())
		{
			collection.mCollectionTitle = sourcePath.stem().u8string();
		}

		// UTF-8 BOMを除去
		std::string cleanText = text;
		if (cleanText.size() >= 3
			&& static_cast<unsigned char>(cleanText[0]) == 0xEF
			&& static_cast<unsigned char>(cleanText[1]) == 0xBB
			&& static_cast<unsigned char>(cleanText[2]) == 0xBF)
		{
			cleanText.erase(0, 3);
		}

		// 行単位へ分割（CRLF/CRを吸収）
		std::vector<std::string> lines;
		{
			std::string line;
			for (std::size_t i = 0; i < cleanText.size(); ++i)
			{
				const char c = cleanText[i];
				if (c == '\r')
				{
					lines.push_back(line);
					line.clear();
					if (i + 1 < cleanText.size() && cleanText[i + 1] == '\n')
					{
						++i;
					}
				}
				else if (c == '\n')
				{
					lines.push_back(line);
					line.clear();
				}
				else
				{
					line.push_back(c);
				}
			}
			if (!line.empty() || !cleanText.empty())
			{
				lines.push_back(line);
			}
		}

		std::vector<std::string> pendingBoardRows;
		std::string pendingTitle;
		std::string pendingAuthor;
		std::string pendingComment;
		std::string lastPlainTextLine;
		bool inBoard = false;
		bool expectingSolution = false;
		std::string currentSolutionBuffer;

		auto flushSolution = [&]() {
			if (!currentSolutionBuffer.empty() && !collection.mLevels.empty())
			{
				collection.mLevels.back().mSolutions.push_back(currentSolutionBuffer);
				currentSolutionBuffer.clear();
			}
			expectingSolution = false;
		};

		auto flushBoard = [&]() {
			if (pendingBoardRows.empty())
			{
				return;
			}
			const auto normalized = NormalizeBoard(pendingBoardRows);
			std::string validationError;
			if (ValidateBoard(normalized, validationError))
			{
				LevelData level;
				level.mTitle = !pendingTitle.empty() ? pendingTitle : lastPlainTextLine;
				level.mAuthor = pendingAuthor;
				level.mComment = pendingComment;
				level.mBoard = normalized;
				collection.mLevels.push_back(std::move(level));
			}
			pendingBoardRows.clear();
			pendingTitle.clear();
			pendingAuthor.clear();
			pendingComment.clear();
			lastPlainTextLine.clear();
			inBoard = false;
		};

		for (const auto& rawLine : lines)
		{
			const std::string rightTrimmed = TrimRight(rawLine);
			const std::string trimmed = Trim(rawLine);

			// 空行は盤面・解ブロックの区切り
			if (trimmed.empty())
			{
				if (inBoard)
				{
					flushBoard();
				}
				if (expectingSolution || !currentSolutionBuffer.empty())
				{
					flushSolution();
				}
				continue;
			}

			// 盤面行（通常XSB行 または RLE行）の判定
			if (IsPlainBoardLine(rightTrimmed))
			{
				if (!inBoard)
				{
					flushSolution();
					inBoard = true;
				}
				pendingBoardRows.push_back(rightTrimmed);
				continue;
			}
			if (IsRleBoardLine(trimmed))
			{
				std::vector<std::string> expandedRows;
				std::string rleError;
				if (ExpandBoardRle(trimmed, expandedRows, rleError))
				{
					if (!inBoard)
					{
						flushSolution();
						inBoard = true;
					}
					for (const auto& row : expandedRows)
					{
						pendingBoardRows.push_back(row);
					}
					continue;
				}
			}

			// 盤面中に非盤面行が来たら盤面を確定
			if (inBoard)
			{
				flushBoard();
			}

			// "::" で始まる .sok コメント行は無視
			if (trimmed.size() >= 2 && trimmed[0] == ':' && trimmed[1] == ':')
			{
				continue;
			}

			// ";" で始まるコメント行（"; Title:" 等のメタデータも抽出）
			if (trimmed[0] == ';')
			{
				const std::string body = Trim(trimmed.substr(1));
				std::string value;
				if (StartsWithKeyIgnoreCase(body, "title", value))
				{
					pendingTitle = value;
				}
				else if (StartsWithKeyIgnoreCase(body, "author", value))
				{
					pendingAuthor = value;
				}
				else if (StartsWithKeyIgnoreCase(body, "comment", value))
				{
					if (!pendingComment.empty()) pendingComment += "\n";
					pendingComment += value;
				}
				else if (!body.empty() && pendingTitle.empty())
				{
					// "; Level 1" のように ; 直後にタイトルを書く慣習にも対応
					lastPlainTextLine = body;
				}
				continue;
			}

			// Solution / Saved Game ブロックの処理
			{
				const std::string lowerTrimmed = ToLowerAscii(trimmed);
				std::string value;
				if (lowerTrimmed == "solution" || lowerTrimmed == "saved game"
					|| StartsWithKeyIgnoreCase(trimmed, "solution", value))
				{
					flushSolution();
					expectingSolution = true;
					if (!value.empty() && IsLurdLine(value))
					{
						currentSolutionBuffer += value;
					}
					continue;
				}
			}

			if ((expectingSolution || (!collection.mLevels.empty() && pendingTitle.empty())) && IsLurdLine(trimmed))
			{
				expectingSolution = true;
				currentSolutionBuffer += trimmed;
				continue;
			}

			// 解ブロック終了
			if (expectingSolution || !currentSolutionBuffer.empty())
			{
				flushSolution();
			}

			// キー付きメタデータ（Title:, Author:, Comment:, Collection:）
			std::string value;
			if (StartsWithKeyIgnoreCase(trimmed, "collection", value))
			{
				if (!value.empty())
				{
					collection.mCollectionTitle = value;
				}
				continue;
			}
			if (StartsWithKeyIgnoreCase(trimmed, "title", value))
			{
				pendingTitle = value;
				continue;
			}
			if (StartsWithKeyIgnoreCase(trimmed, "author", value))
			{
				pendingAuthor = value;
				continue;
			}
			if (StartsWithKeyIgnoreCase(trimmed, "comment", value))
			{
				if (!pendingComment.empty()) pendingComment += "\n";
				pendingComment += value;
				continue;
			}

			// その他キー:値メタデータ（Date:, Email:, Solver: 等）はタイトル候補にしない
			if (trimmed.find(':') != std::string::npos)
			{
				const std::string head = trimmed.substr(0, trimmed.find(':'));
				bool alphaOnly = !head.empty();
				for (char c : head)
				{
					if (!std::isalpha(static_cast<unsigned char>(c)) && c != ' ' && c != '-' && c != '_')
					{
						alphaOnly = false;
						break;
					}
				}
				if (alphaOnly)
				{
					continue;
				}
			}

			// 盤面直前の通常テキスト行をタイトル候補として保持
			lastPlainTextLine = trimmed;
		}

		// 末尾に残った盤面・解を確定
		flushBoard();
		flushSolution();

		if (collection.mLevels.empty())
		{
			throw std::runtime_error("No valid Sokoban level found"
				+ (sourcePath.empty() ? std::string("") : ": " + sourcePath.u8string()));
		}

		// タイトルが空のレベルには番号付きフォールバックを設定
		for (std::size_t i = 0; i < collection.mLevels.size(); ++i)
		{
			if (collection.mLevels[i].mTitle.empty())
			{
				if (collection.mLevels.size() == 1 && !collection.mCollectionTitle.empty())
				{
					collection.mLevels[i].mTitle = collection.mCollectionTitle;
				}
				else
				{
					collection.mLevels[i].mTitle = "Level " + std::to_string(i + 1);
				}
			}
		}

		return collection;
	}

	LevelCollection LoadCollectionFromFile(const std::filesystem::path& path)
	{
		const auto bytes = AppPaths::ReadFile(path);
		const std::string text(bytes.begin(), bytes.end());
		return ParseCollection(text, path);
	}

	std::string FormatLevel(const LevelData& level, const std::string& format)
	{
		const std::string normalizedFormat = ToLowerAscii(format.empty() ? "txt" : format);
		std::ostringstream out;

		if (normalizedFormat == "sok")
		{
			if (!level.mTitle.empty())
			{
				out << "Title: " << level.mTitle << "\n";
			}
			if (!level.mAuthor.empty())
			{
				out << "Author: " << level.mAuthor << "\n";
			}
			if (!level.mComment.empty())
			{
				out << "Comment: " << level.mComment << "\n";
			}
			for (const auto& row : level.mBoard)
			{
				out << TrimRight(row) << "\n";
			}
			for (const auto& sol : level.mSolutions)
			{
				if (!sol.empty())
				{
					out << "\nSolution:\n" << sol << "\n";
				}
			}
		}
		else if (normalizedFormat == "xsb")
		{
			if (!level.mTitle.empty())
			{
				out << "; Title: " << level.mTitle << "\n";
			}
			if (!level.mAuthor.empty())
			{
				out << "; Author: " << level.mAuthor << "\n";
			}
			for (const auto& row : level.mBoard)
			{
				out << TrimRight(row) << "\n";
			}
		}
		else
		{
			// シンプルな .txt 形式（既存互換）
			for (const auto& row : level.mBoard)
			{
				out << row << "\n";
			}
		}

		return out.str();
	}

	void SaveLevelToFile(const std::filesystem::path& path, const LevelData& level, const std::string& format)
	{
		const std::string targetFormat = format.empty() ? DetectFormatFromPath(path) : format;
		const std::string contents = FormatLevel(level, targetFormat);
		AppPaths::WriteFile(path, contents);
	}
}
