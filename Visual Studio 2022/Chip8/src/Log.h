#pragma once
#include <iostream>
#include <stdarg.h>

class Log
{
public:
	static void LogConsole(const char* format, ...);
};


#define LOG(...)	Log::LogConsole(__VA_ARGS__)

