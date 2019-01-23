#include "stdafx.h"
#include "ILogger.h"
#include <chrono>
#include <ctime>
#include <iomanip>
#include <sstream>
#include <thread>
#include <locale> 
#include <codecvt>
void ILogger::LogWarning(std::wstring & message)
{
	//setup converter
	std::wstring_convert<std::codecvt_utf8<wchar_t>, wchar_t> converter;

	std::string converted_str = converter.to_bytes(message);
	LogWarning(converted_str);
}
std::string ILogger::Now()
{
		auto now = std::chrono::high_resolution_clock::now();
		auto now2 = std::time(nullptr);

		auto millisec = std::chrono::duration_cast<std::chrono::milliseconds>(now.time_since_epoch());
		
		auto fractional_seconds = millisec.count() % 1000;

		auto localTime = *std::localtime(&now2);

		std::ostringstream oss;
		oss << std::put_time(&localTime, "%Y.%m.%d %H:%M:%S.") << fractional_seconds;
		return oss.str();

}

std::string ILogger::ThreadId()
{
	std::ostringstream oss;
	oss << "[" <<  std::this_thread::get_id() << "]";
	return oss.str();
}
