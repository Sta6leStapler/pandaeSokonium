#pragma once
class State
{
public:
	// アクターの状態を表現する列挙型
	enum EState
	{
		EActive,
		EPaused,
		EDead
	};

	State();
	State(EState state);

	// ゲッターとセッター
	EState GetEState() const { return mState; }
	void SetEState(const EState state) { mState = state; }

private:
	EState mState;
};

