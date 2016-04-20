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
	void Log          (std::wstring);
	void Log          (std::wostringstream&);
	void Log          (std::wstring,               const char*, long, const char*);
	void LogWarning   (std::wstring,               const char*, long, const char*);
	void LogError     (std::wstring,               const char*, long, const char*);
	bool LogHRESULT   (HRESULT,                    const char*, long, const char*);
	void LogAssert    (std::wstring, std::wstring, const char*, long, const char*);
}