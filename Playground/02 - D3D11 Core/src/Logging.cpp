#include "stdafx.h"
#include "Logging.h"

//TODO: Make implementation DRYer
namespace Logging
{
	void Log(std::wstring message)
	{
		std::wstringstream stream;

		stream << message << std::endl;

		//Send it all to the VS Output window
		OutputDebugString(stream.str().c_str());
	}

	void Log(std::wstring message, char* file, long line, char* function)
	{
		std::wstringstream stream;

		stream << function << " - " << message << std::endl;

		//Append the error location (it's clickable!)
		stream << "\t" << file << "(" << line << ")" << std::endl;

		//Send it all to the VS Output window
		OutputDebugString(stream.str().c_str());
	}

	void LogWarning(std::wstring message, char* file, long line, char* function)
	{
		std::wstringstream stream;

		stream << "WARNING: " << function << " - " << message << std::endl;

		//Append the error location (it's clickable!)
		stream << "\t" << file << "(" << line << ")" << std::endl;

		//Send it all to the VS Output window
		OutputDebugString(stream.str().c_str());
	}

	void LogError(std::wstring message, char* file, long line, char* function)
	{
		std::wstringstream stream;

		stream << "ERROR: " << function << " - " << message << std::endl;

		//Append the error location (it's clickable!)
		stream << "\t" << file << "(" << line << ")" << std::endl;

		//Send it all to the VS Output window
		OutputDebugString(stream.str().c_str());
	}

	bool LogIfFailed(HRESULT hr, char* file, long line, char* function)
	{
		//Success!
		if ( !FAILED(hr) ) { return false; }

		//Get a friendly string from the D3D error
		wchar_t* errorMessage;
		DWORD ret = FormatMessage(
			FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_ALLOCATE_BUFFER,
			nullptr,
			hr,
			LANG_USER_DEFAULT,
			(LPTSTR) &errorMessage,
			0,
			nullptr
		);

		std::wstringstream stream;

		//Log the friendly error message
		if ( ret != 0 )
		{
			stream << "ERROR: " << function << " - " << errorMessage;
		}
		//FormatMessage failed, log that too
		else
		{
			stream << "ERROR: Failed to format error message from HRESULT: " << hr << ". FormatMessage error: " << GetLastError() << std::endl;
		}
		HeapFree(GetProcessHeap(), NULL, errorMessage);

		//Append the error location (it's clickable!)
		stream << "\t" << file << "(" << line << ")" << std::endl;

		//Send it all to the VS Output window
		OutputDebugString(stream.str().c_str());

		//DebugBreak();
		__debugbreak();
		return true;
	}
}