#pragma once

#include "SFML/Graphics.hpp"
#include <unordered_map>
#include <string>
#include <vector>
#include <chrono>

#include "IActor.h"
#include "IComponent.h"
#include "SpriteComponent.h"

#include "GameBoard.h"
#include "GameBoardComponent.h"
#include "BackGround.h"
#include "BackGroundComponent.h"
#include "Player.h"
#include "PlayerComponent.h"
#include "Baggage.h"
#include "BaggageComponent.h"
#include "IUIScreen.h"
#include "PauseMenu.h"
#include "HUD.h"
#include "THUD.h"
#include "HUDHelper.h"
#include "TGUI/TGUI.hpp"
#include "TGUI/Backend/SFML-Graphics.hpp"
#include "MySolution.h"

// �ꓮ��̃��O
struct Log
{
	std::pair<sf::Vector2i, sf::Vector2i> pCoordinate, bCoordinate;
	bool isBMoved;
	Player::Direction direction1, direction2;
	std::string time;
	std::vector<std::vector<Log>> thread;
};

using BoundingBox = std::pair<sf::Vector2f, sf::Vector2f>;

class Game
{
public:
	Game();
	bool Initialize();
	void RunLoop();
	void Shutdown();

	// �Q�[���̏��
	enum GameState
	{
		EGamePlay,
		EPaused,
		EQuit
	};

	// �e�N�X�`���̃��[�h
	sf::Texture* LoadTexture(const std::string& fileName);

	// �A�N�^�[�̒ǉ��ƍ폜
	void AddActor(class IActor* actor);
	void RemoveActor(class IActor* actor);

	// �X�v���C�g�̒ǉ��ƍ폜
	void AddSprite(class SpriteComponent* sprite);
	void RemoveSprite(class SpriteComponent* sprite);
	
	void AddActor(class GameBoard* gameboard);
	void RemoveActor(class GameBoard* gameboard);
	void AddActor(class BackGround* gameboard);
	void RemoveActor(class BackGround* gameboard);
	void AddActor(class Player* player);
	void RemoveActor(class Player* player);
	void AddActor(class Baggage* baggage);
	void RemoveActor(class Baggage* baggage);

	// �X�v���C�g�̒ǉ��ƍ폜
	void AddSprite(class GameBoardComponent* GBComp);
	void RemoveSprite(class GameBoardComponent* GBcomp);
	void AddSprite(class BackGroundComponent* GBComp);
	void RemoveSprite(class BackGroundComponent* GBcomp);
	void AddSprite(class PlayerComponent* PComp);
	void RemoveSprite(class PlayerComponent* PComp);
	void AddSprite(class BaggageComponent* BComp);
	void RemoveSprite(class BaggageComponent* BComp);

	// UI��ʂ̃X�^�b�N�Ɋւ��鏈��
	// �X�^�b�N�S�̂��Q�ƂŕԂ�
	const std::vector<class IUIScreen*>& GetUIStack() { return mUIStack; }
	// �w���UIScreen���X�^�b�N�Ƀv�b�V������
	void PushUI(class IUIScreen* screen) { mUIStack.emplace_back(screen); }

	// �Q�[�����L�̃����o�֐�������Βǉ�
	// �X�e�b�v�����Z
	void AddStep() { mStep++; }

	// undo/redo����
	void CallUndo();
	void CallRedo();
	void CallReset();
	void CallRedoAll();

	// �Ֆʂ��Z�[�u
	void CallSave();

	// �Ֆʂ̃����[�h
	void CallReload();

	// �S�Ẵ��Z�b�g
	void CallRestart();

	// ��̃��O���폜 (�v���C���[�̈ړ��������ɌĂяo��)
	void RemoveRedo();

	// ���O��ǉ�
	void AddLog(const sf::Vector2i& playerPos1, const sf::Vector2i& playerPos2, const Player::Direction& direction1, const Player::Direction& direction2);
	void AddLog(const sf::Vector2i& playerPos1, const sf::Vector2i& playerPos2, const sf::Vector2i& baggagePos1, const sf::Vector2i& baggagePos2, const Player::Direction& direction1, const Player::Direction& direction2);

	// ���O���e�L�X�g�ɕϊ�
	std::string ConvertLogToStr(const std::vector<Log>& logs, const unsigned long long& current);

	// ���O�̏o��
	void OutputLogs();

	// ���ݓ����𕶎���ŏo��
	std::string GetDateTime();

	// �I��������s���A�E�B���h�E�Œʒm
	void HasComplete();
	void DisplayResult();

	// �Ֆʂ̋K�͂̓���
	bool InputBoardData();

	// �w���v�E�B���h�E�̕\��
	void DisplayHelpWindow();

	// �Ֆʃ��X�g��\�����A�I�������ՖʂɕύX����
	void SelectBoards();
	void ChangeBoard();

	// �Q�b�^�[�ƃZ�b�^�[
	// �Q�[������֘A
	void SetState(const GameState& gameState) { mGameState = gameState; }
	sf::Vector2f GetWindowSize() const { return mWindowSize; }
	std::vector<std::string> GetFilenames() const { return mFilenames; }
	std::string GetCurrentKey() const { return mCurrentKey; }
	std::string GetFilename(unsigned int num) const { return mFilenames.at(num); }
	std::unordered_map<std::string, std::vector<std::string>> GetBoardData() const { return mBoardData; }
	sf::Vector2i GetBoardSize() const { return mBoardSize; }
	BoundingBox GetBoardViewArea() const { return mBoardViewArea; }
	unsigned int GetStep() const { return mStep; }
	double GetSecTime() const { return std::chrono::duration_cast<std::chrono::seconds>(std::chrono::system_clock::now() - mStart).count(); };
	double GetMSecTime() const { return std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::system_clock::now() - mStart).count(); };

	// �Ֆʊ֘A
	std::vector<class Baggage*>& GetBaggages() { return mBaggages; }
	std::vector<sf::Vector2i> GetBaggagesPos() const;
	class Player* GetPlayer() const { return mPlayer; }
	int GetBaggageNumLimit(const sf::Vector2i& size, const double& wallRate = 0.0) const;

	class HUDHelper* GetHUDHelper() const { return mHUDHelper; }
	std::vector<std::string> GetBoardState() const { return mBoardState; }

private:
	void ProcessInput();
	void UpdateGame();
	void GenerateOutput();
	void LoadData();
	void UnloadData();

	// �w���p�[�֐��͈ȉ��ɒ�`
	void SyncSliderWithEditBox(tgui::Slider::Ptr slider, tgui::EditBox::Ptr editBox, const bool& isInteger);

	// ���[�h���ꂽ�e�N�X�`���̃}�b�v
	std::unordered_map<std::string, sf::Texture*> mTextures;

	// �A�N�e�B�u�ȃA�N�^�[�Ƒҋ@���̃A�N�^�[
	std::vector<IActor*> mActiveActors;
	std::vector<IActor*> mPendingActors;

	// �Q�[���̂��ׂẴA�N�^�[
	class GameBoard* mGameBoard;
	class BackGround* mBackGround;
	class Player* mPlayer;
	std::vector<class Baggage*> mBaggages;
	// �ҋ@���̃A�N�^�[
	class GameBoard* mPendingGameBoard;
	class BackGround* mPendingBackGround;
	class Player* mPendingPlayer;
	std::vector<class Baggage*> mPendingBaggages;

	// �`����s���R���|�[�l���g
	GameBoardComponent* mGameBoardComponent;
	BackGroundComponent* mBackGroundComponent;
	PlayerComponent* mPlayerComponent;
	std::vector<BaggageComponent*> mBaggageComponents;

	sf::RenderWindow* mWindow;
	sf::Clock mClock;
	sf::Time mTicksCount;
	sf::Font mFont;
	sf::Text mInfoTxt;

	// �Q�[����Ԃ��i�[����ϐ�
	GameState mGameState;
	bool mIsComplete;

	// �A�N�^�[���X�V�����ǂ����̒ǐ� (true : ���� / false : �Ȃ�)
	bool mUpdatingActors;

	// UI�̃X�v���C�g�������s�����߂̃X�^�b�N
	std::vector<class IUIScreen*> mUIStack;

	// HUD�̕⏕���s��HUDHelper�N���X
	HUDHelper* mHUDHelper;

	// �x�[�X�ƂȂ�gui
	std::unique_ptr<tgui::Gui> mGui;
	std::unique_ptr<tgui::Theme> mTheme;

	// �Ֆʂ̊�b�I���
	sf::Vector2i mBoardSize;
	int mBaggageNum;
	int mRepetition01;			// �ו��ƃv���C���[�̔z�u�̃��Z�b�g��
	int mRepetition02;			// �ו��̉^����
	double mRepetition03;		// ���炩���ߐݒu���Ă����ǃ}�X�̊���
	double mRepetition04;		// �K��ς݂ɂ��炩���ߊ���U�銄��
	int mRepetition05;			// �]���֐��̃C���f�b�N�X

	// �Q�[�����L�̃����o�ϐ�������Βǉ�
	sf::Vector2f mWindowSize;
	std::string mCurrentKey;
	std::vector<std::string> mFilenames;
	std::unordered_map<std::string, std::vector<std::string>> mBoardData;		// �v���C���[�Ɖו����܂߂��Ֆʏ��
	std::unordered_map<std::string, std::vector<std::string>> mInitBoardData;	// �����Ֆ�
	std::vector<std::string> mBoardState;	// �v���C���[�Ɖו����������Ֆʏ��
	std::vector<sf::Vector2i> mGoalPos;
	const sf::Vector2i mSizeMax{ 48, 48 };
	const sf::Vector2i mSizeMin{ 5, 5 };
	int mBaggageLimit;
	const double mWallRateLimit = 0.5;
	const double mVisitedRateLimit = 1.0;
	const std::pair<int, int> mEvaluateFancIndexRange = std::pair<int, int>{ 0, 6 };
	std::chrono::system_clock::time_point mStart;

	// �v���C���[�Ɖו��̏����ʒu
	sf::Vector2i mInitialPlayerPos;
	std::unordered_map<Baggage*, sf::Vector2i> mInitialBaggagePos;

	// �Ֆʂ̕`��͈�
	BoundingBox mBoardViewArea;

	// ���͂̃N�[���_�E��
	float mInputCooldown;

	// ���݂̃^�[����
	unsigned int mStep;

	// ���O�����W�̃��X�g�ŕ\��
	std::vector<Log> mLogs;
};