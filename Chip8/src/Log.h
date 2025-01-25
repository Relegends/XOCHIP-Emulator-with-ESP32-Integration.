#pragma once
#include <iostream>
#include <stdarg.h>

class Log
{
public:
	static void LogConsole(const char* format, ...);
	static void LogConsolePrint(const char* format, bool print, ...);
};


#define LOG(...)	Log::LogConsole(__VA_ARGS__)
#define LOG_PRINT(...)	Log::LogConsolePrint(__VA_ARGS__)

