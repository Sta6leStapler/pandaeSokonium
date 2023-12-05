#pragma once

#include "SFML/Graphics.hpp"
#include <string>
#include <vector>

class PGSL
{
public:
	PGSL(const sf::Vector2i& size, const int& baggageNum);
private:

	sf::Vector2i size;	// size��3*3�̈��~���l�߂鐔
	std::vector<std::string> mBoardStr;	// �Ֆʂ̏���\��
	int mBaggageNum;	// �ו��̐�
	std::vector<std::vector<std::string>> templates;	// �Ֆʂ̍\���ɗp����e���v���[�g�̃��X�g
};

