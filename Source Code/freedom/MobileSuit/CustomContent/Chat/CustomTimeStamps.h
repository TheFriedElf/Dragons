#pragma once

#include <string>
#include <chrono>



class CustomTimeStamps
{
public:
	static std::wstring FormatChatTimeHHMM(std::chrono::system_clock::time_point const& timePoint);
};