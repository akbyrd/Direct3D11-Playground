#include "stdafx.h"

#include "LoggedException.h"

using namespace std;

LoggedException::LoggedException() { }

LoggedException::LoggedException(
	wstring     message,
	const char* file,
	DWORD       line,
	const char* function)
{
	Logging::LogError(message, file, line, function);
}

const char* LoggedException::what() const
{
	return message.c_str();
}