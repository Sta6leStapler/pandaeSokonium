#pragma once
class State
{
public:
	// �A�N�^�[�̏�Ԃ�\������񋓌^
	enum EState
	{
		EActive,
		EPaused,
		EDead
	};

	State();
	State(EState state);

	// �Q�b�^�[�ƃZ�b�^�[
	EState GetEState() const { return mState; }
	void SetEState(const EState state) { mState = state; }

private:
	EState mState;
};

