#pragma once

#include <string>

#define LOG(msg)           Logging::Log       (msg, __FILE__, __LINE__, __FUNCTION__)
#define LOG_WARNING(msg)   Logging::LogWarning(msg, __FILE__, __LINE__, __FUNCTION__)
#define LOG_ERROR(msg)     Logging::LogError  (msg, __FILE__, __LINE__, __FUNCTION__)
#define LOG_HRESULT(hr)    Logging::LogHRESULT( hr, __FILE__, __LINE__, __FUNCTION__)
#define LOG_LASTERROR()    Logging::LogHRESULT(HRESULT_FROM_WIN32(GetLastError()),  \
                                                    __FILE__, __LINE__, __FUNCTION__)
//TODO: Log expression

namespace Logging
{
	using namespace std;

	void Log          (wstring message);
	void Log          (wostringstream& msgStream);
	void Log          (wstring message,                     const char* file, long line, const char* function);
	void LogWarning   (wstring message,                     const char* file, long line, const char* function);
	void LogError     (wstring message,                     const char* file, long line, const char* function);
	void LogAssert    (wstring expression, wstring message, const char* file, long line, const char* function);

	#ifdef WIN32
	bool LogHRESULT   (HRESULT hr,                          const char* file, long line, const char* function);
	#endif
}