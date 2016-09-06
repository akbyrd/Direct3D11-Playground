#pragma once

//TODO: This is inherently WIN32-only
//TODO: Get rid of this
#include <Windows.h>

#include <string>

#define LOG(msg)           Logging::Log       (msg, __FILE__, __LINE__, __FUNCTION__)
#define LOG_WARNING(msg) { Logging::LogWarning(msg, __FILE__, __LINE__, __FUNCTION__); __debugbreak(); }
#define LOG_ERROR(msg)   { Logging::LogError  (msg, __FILE__, __LINE__, __FUNCTION__); __debugbreak(); }
#define LOG_HRESULT(hr)    Logging::LogHRESULT( hr, __FILE__, __LINE__, __FUNCTION__)
#define LOG_LASTERROR()    LOG_HRESULT(HRESULT_FROM_WIN32(GetLastError()))
//TODO: Log expression

namespace Logging
{
	using namespace std;

	void Log        (wstring message);
	void Log        (wostringstream& msgStream);
	void Log        (wstring message,                     const char* file, long line, const char* function);
	void LogWarning (wstring message,                     const char* file, long line, const char* function);
	void LogError   (wstring message,                     const char* file, long line, const char* function);
	void LogAssert  (wstring expression, wstring message, const char* file, long line, const char* function);
	bool LogHRESULT (HRESULT hr,                          const char* file, long line, const char* function);
}