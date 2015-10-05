#pragma once

#define LOG(msg)          Logging::Log        (msg, __FILE__, __LINE__, __FUNCTION__)
#define LOG_WARNING(msg)  Logging::LogWarning (msg, __FILE__, __LINE__, __FUNCTION__)
#define LOG_ERROR(msg)    Logging::LogError   (msg, __FILE__, __LINE__, __FUNCTION__)
#define LOG_IF_FAILED(hr) Logging::LogIfFailed(hr , __FILE__, __LINE__, __FUNCTION__)

#define CHECK(hr) if ( LOG_IF_FAILED(hr) ) { goto Cleanup; }

namespace Logging
{
	void Log(std::wstring);
	void Log(std::wstring, char*, long, char*);
	void LogWarning(std::wstring, char*, long, char*);
	void LogError(std::wstring, char*, long, char*);
	bool LogIfFailed(HRESULT, char*, long, char*);
}