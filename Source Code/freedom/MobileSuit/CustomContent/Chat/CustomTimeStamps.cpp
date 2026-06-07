#include "stdafx.h"
#include "CustomTimeStamps.h"
#include <chrono>

std::wstring CustomTimeStamps::FormatChatTimeHHMM(std::chrono::system_clock::time_point const& timePoint)
{
	auto const tt = std::chrono::system_clock::to_time_t(timePoint);

	std::tm tmLocal{};
	localtime_s(&tmLocal, &tt);

	wchar_t buffer[16] = {};
	wcsftime(buffer, _countof(buffer), L"%H:%M", &tmLocal);
	return buffer;
}