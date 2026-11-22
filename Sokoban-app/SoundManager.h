#pragma once

#include "AppPaths.h"

#include "SFML/Audio.hpp"
#include "SFML/System/Clock.hpp"

#include <cstddef>
#include <unordered_map>
#include <vector>

// 効果音（SE）の読み込みと再生を管理するクラス
class SoundManager
{
public:
	enum class SoundEffect
	{
		Move,        // プレイヤー移動
		Push,        // 荷物押し
		GoalIn,      // 荷物がゴールに乗った
		Blocked,     // 壁・二重荷物等で移動不可
		Undo,        // 元に戻す
		Redo,        // やり直し
		Clear,       // ステージクリア
		Deadlock,    // 詰み検出
		ButtonClick  // UIボタン操作
	};

	SoundManager();
	~SoundManager();

	// Assets/Sounds から全効果音をメモリ経由で読み込む
	bool Initialize(const AppPaths& paths);

	// 効果音を再生する（音声デバイス無効環境では安全に無音スキップ）
	void Play(SoundEffect effect);

	// 音量（0.0f〜100.0f）とミュートの取得・設定
	void SetVolume(float volume);
	float GetVolume() const { return mVolume; }
	void SetMuted(bool muted);
	bool IsMuted() const { return mIsMuted; }

private:
	static constexpr std::size_t SOUND_POOL_SIZE = 16;
	static constexpr float MIN_REPEAT_INTERVAL_SEC = 0.035f;

	std::unordered_map<SoundEffect, sf::SoundBuffer> mBuffers;
	std::unordered_map<SoundEffect, float> mLastPlayedSec;
	std::vector<sf::Sound> mSoundPool;
	std::size_t mNextChannel;
	sf::Clock mClock;
	float mVolume;
	bool mIsMuted;
	bool mIsInitialized;
};
