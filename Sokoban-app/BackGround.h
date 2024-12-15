#pragma once

#include "SFML/Graphics.hpp"
#include "IActor.h"

#include <unordered_map>
#include <cmath>

class BackGround : IActor
{
public:
	BackGround(class Game* game);
	virtual ~BackGround();

	// �A�N�^�[���A�N�e�B�u��Ԃ̂Ƃ��X�V�������s���֐�
	void Update(float deltaTime) override;

	// �A�N�^�[�����S�ẴR���|�[�l���g���X�V
	void UpdateComponents(float deltaTime) override;

	// Game�N���X������͏������Ăяo��
	void ProcessInput(const sf::Event* event) override;

	// �A�N�^�[�������ׂẴR���|�[�l���g�̓��͏������s��
	void ProcessInputComponents(const sf::Event* event) override;

	// �R���|�[�l���g�̒ǉ��ƍ폜
	void AddComponent(class IComponent* component) override;
	void RemoveComponent(class IComponent* component) override;

	// �A�N�^�[���ʂ̃Q�b�^�[�ƃZ�b�^�[
	class Game* GetGame() override { return mGame; }
	IActor::ActorState GetState() override { return mState; }
	void SetState(const ActorState state) override { this->mState = state; }
	sf::Vector2f GetPosition() override { return mPosition; }
	sf::Vector2f GetScale() override { return mScale; }
	float GetRotation() override { return mRotation; }

	// �ȉ��{�A�N�^�[���L�̃����o�֐�
	// �Q�b�^�[�ƃZ�b�^�[
	sf::Vector2f GetForward() const { return sf::Vector2f(std::cos(mRotation), -std::sin(mRotation)); }

private:
	// �A�N�^�[�̏��
	IActor::ActorState mState;

	// �ϊ�
	sf::Vector2f mPosition;
	sf::Vector2f mScale;
	float mRotation;

	std::vector<IComponent*> mComponents;
	class SpriteComponent* mSpriteComponent;

	// �R���|�[�l���g�Ɏg���e�N�X�`��
	std::unordered_map<std::string, sf::Texture*> mTextures;

	class Game* mGame;

};