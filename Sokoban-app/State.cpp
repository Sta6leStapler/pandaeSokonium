#include "State.h"

State::State()
	:mState(EState::EActive)
{
}

State::State(EState state)
	:mState(state)
{
}