#pragma once

#include <string>

#define LOG(msg)          Logging::Log        (msg, __FILE__, __LINE__, __FUNCTION__)
#define LOG_WARNING(msg)  Logging::LogWarning (msg, __FILE__, __LINE__, __FUNCTION__)
#define LOG_ERROR(msg)    Logging::LogError   (msg, __FILE__, __LINE__, __FUNCTION__)
//TODO: Log expression
#define LOG_IF_FAILED(hr) Logging::LogIfFailed( hr, __FILE__, __LINE__, __FUNCTION__)

namespace Logging
{
	void Log        (std::wstring);
	void Log        (std::wostringstream&);
	void Log        (std::wstring,               const char*, long, const char*);
	void LogWarning (std::wstring,               const char*, long, const char*);
	void LogError   (std::wstring,               const char*, long, const char*);
	bool LogIfFailed(HRESULT,                    const char*, long, const char*);
	void LogAssert  (std::wstring, std::wstring, const char*, long, const char*);
}