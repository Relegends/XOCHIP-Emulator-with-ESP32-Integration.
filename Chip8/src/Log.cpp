#include "Log.h"


void Log::LogConsole(const char* format, ...)
{
	if (true) return;
	va_list args;
	va_start(args, format);
	vprintf(format, args);
	va_end(args);
}

void Log::LogConsolePrint(const char* format, bool print, ...)
{
	if (!print) return;
	va_list args;
	va_start(args, print);
	vprintf(format, args);
	va_end(args);
}
