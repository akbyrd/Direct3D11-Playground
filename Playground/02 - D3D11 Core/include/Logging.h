#pragma once

#define LOG(msg)          Logging::Log        (msg, __FILE__, __LINE__, __FUNCTION__)
#define LOG_IF_FAILED(hr) Logging::LogIfFailed(hr , __FILE__, __LINE__, __FUNCTION__)

namespace Logging
{
	void Log(std::wstring);
	void Log(std::wstring, char*, long, char*);
	bool LogIfFailed(HRESULT, char*, long, char*);
}