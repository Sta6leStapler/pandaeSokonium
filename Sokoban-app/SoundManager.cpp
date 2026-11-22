#include "SoundManager.h"

#include <algorithm>
#include <iostream>
#include <string>
#include <utility>

SoundManager::SoundManager()
	: mNextChannel(0)
	, mVolume(70.0f)
	, mIsMuted(false)
	, mIsInitialized(false)
{
}

SoundManager::~SoundManager()
{
	for (auto& sound : mSoundPool)
	{
		sound.stop();
	}
	mSoundPool.clear();
	mBuffers.clear();
}

bool SoundManager::Initialize(const AppPaths& paths)
{
	mBuffers.clear();
	mLastPlayedSec.clear();
	mSoundPool.clear();
	mIsInitialized = false;

	const std::vector<std::pair<SoundEffect, std::string>> soundFiles = {
		{ SoundEffect::Move,        "Assets/Sounds/move.wav" },
		{ SoundEffect::Push,        "Assets/Sounds/push.wav" },
		{ SoundEffect::GoalIn,      "Assets/Sounds/goal_in.wav" },
		{ SoundEffect::Blocked,     "Assets/Sounds/blocked.wav" },
		{ SoundEffect::Undo,        "Assets/Sounds/undo.wav" },
		{ SoundEffect::Redo,        "Assets/Sounds/redo.wav" },
		{ SoundEffect::Clear,       "Assets/Sounds/clear.wav" },
		{ SoundEffect::Deadlock,    "Assets/Sounds/deadlock.wav" },
		{ SoundEffect::ButtonClick, "Assets/Sounds/button.wav" }
	};

	bool anyLoaded = false;
	for (const auto& entry : soundFiles)
	{
		try
		{
			const auto filePath = paths.Asset(entry.second);
			const auto data = AppPaths::ReadFile(filePath);
			sf::SoundBuffer buffer;
			if (!buffer.loadFromMemory(data.data(), data.size()))
			{
				std::cerr << "Warning: Failed to decode sound file: " << filePath.u8string() << std::endl;
				continue;
			}
			mBuffers.emplace(entry.first, std::move(buffer));
			anyLoaded = true;
		}
		catch (const std::exception& error)
		{
			std::cerr << "Warning: Sound asset unavailable (" << entry.second << "): " << error.what() << std::endl;
		}
	}

	mSoundPool.resize(SOUND_POOL_SIZE);
	mNextChannel = 0;
	mClock.restart();
	mIsInitialized = anyLoaded;
	return mIsInitialized;
}

void SoundManager::Play(SoundEffect effect)
{
	if (!mIsInitialized || mIsMuted || mVolume <= 0.0f)
	{
		return;
	}

	const auto it = mBuffers.find(effect);
	if (it == mBuffers.end())
	{
		return;
	}

	// 連続移動や一括操作での耳障りな多重発音を防ぐクールダウン
	const float nowSec = mClock.getElapsedTime().asSeconds();
	const auto lastIt = mLastPlayedSec.find(effect);
	if (effect != SoundEffect::Clear && lastIt != mLastPlayedSec.end()
		&& (nowSec - lastIt->second) < MIN_REPEAT_INTERVAL_SEC)
	{
		return;
	}
	mLastPlayedSec[effect] = nowSec;

	// 空いているチャンネルを探し、なければリングバッファで再利用する
	std::size_t selected = mNextChannel;
	for (std::size_t i = 0; i < mSoundPool.size(); ++i)
	{
		const std::size_t idx = (mNextChannel + i) % mSoundPool.size();
		if (mSoundPool[idx].getStatus() != sf::Sound::Playing)
		{
			selected = idx;
			break;
		}
	}
	mNextChannel = (selected + 1) % mSoundPool.size();

	sf::Sound& sound = mSoundPool[selected];
	sound.stop();
	sound.setBuffer(it->second);
	sound.setVolume(mVolume);
	sound.play();
}

void SoundManager::SetVolume(float volume)
{
	mVolume = std::clamp(volume, 0.0f, 100.0f);
	if (!mIsMuted)
	{
		for (auto& sound : mSoundPool)
		{
			sound.setVolume(mVolume);
		}
	}
}

void SoundManager::SetMuted(bool muted)
{
	mIsMuted = muted;
	const float effectiveVolume = mIsMuted ? 0.0f : mVolume;
	for (auto& sound : mSoundPool)
	{
		sound.setVolume(effectiveVolume);
	}
}
