#include "stdafx.h"
#include "Logging.h"

using namespace std;

//TODO: Make implementation DRYer
namespace Logging
{
	void Log(wstring message)
	{
		wostringstream stream;

		stream << message << endl;

		//Send it all to the VS Output window
		OutputDebugString(stream.str().c_str());
	}

	void Log(wostringstream& stream)
	{
		stream << endl;

		//Send it all to the VS Output window
		OutputDebugString(stream.str().c_str());
	}

	void Log(wstring message, char* file, long line, char* function)
	{
		wostringstream stream;

		stream << function << " - " << message << endl;

		//Append the error location (it's clickable!)
		stream << "\t" << file << "(" << line << ")" << endl;

		//Send it all to the VS Output window
		OutputDebugString(stream.str().c_str());
	}

	void LogWarning(wstring message, char* file, long line, char* function)
	{
		wostringstream stream;

		stream << "WARNING: " << function << " - " << message << endl;

		//Append the error location (it's clickable!)
		stream << "\t" << file << "(" << line << ")" << endl;

		//Send it all to the VS Output window
		OutputDebugString(stream.str().c_str());
	}

	void LogError(wstring message, char* file, long line, char* function)
	{
		wostringstream stream;

		stream << "ERROR: " << function << " - " << message << endl;

		//Append the error location (it's clickable!)
		stream << "\t" << file << "(" << line << ")" << endl;

		//Send it all to the VS Output window
		OutputDebugString(stream.str().c_str());

		__debugbreak();
	}

	void LogAssert(wstring expression, wstring message, const char* file, long line, const char* function)
	{
		wostringstream stream;

		stream << "ASSERT FAILED: " << function << " - " << expression << endl
		       << message << endl;

		//Append the error location (it's clickable!)
		stream << "\t" << file << "(" << line << ")" << endl;

		//Send it all to the VS Output window
		OutputDebugString(stream.str().c_str());

		__debugbreak();
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

		wostringstream stream;

		//Log the friendly error message
		if ( ret != 0 )
		{
			stream << "ERROR: " << function << " - " << errorMessage;
		}
		//FormatMessage failed, log that too
		else
		{
			stream << "ERROR: Failed to format error message from HRESULT: " << hr << ". FormatMessage error: " << GetLastError() << endl;
		}
		HeapFree(GetProcessHeap(), NULL, errorMessage);

		//Append the error location (it's clickable!)
		stream << "\t" << file << "(" << line << ")" << endl;

		//Send it all to the VS Output window
		OutputDebugString(stream.str().c_str());

		__debugbreak();
		return true;
	}
}