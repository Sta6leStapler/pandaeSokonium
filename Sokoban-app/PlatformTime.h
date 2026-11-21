#pragma once

#include <ctime>
#include <stdexcept>

// プラットフォーム差異を吸収してローカル時刻を取得するヘルパー
namespace PlatformTime
{
	inline std::tm GetLocalTime(std::time_t timeValue)
	{
		std::tm timeInfo{};
#if defined(_WIN32)
		if (localtime_s(&timeInfo, &timeValue) != 0)
		{
			throw std::runtime_error("localtime_s failed");
		}
#else
		if (localtime_r(&timeValue, &timeInfo) == nullptr)
		{
			throw std::runtime_error("localtime_r failed");
		}
#endif
		return timeInfo;
	}
}
