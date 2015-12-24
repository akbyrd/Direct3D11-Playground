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
		OutputDebugStringW(stream.str().c_str());
	}

	void Log(wostringstream& stream)
	{
		stream << endl;

		//Send it all to the VS Output window
		OutputDebugStringW(stream.str().c_str());
	}

	void Log(wstring message, const char* file, long line, const char* function)
	{
		wostringstream stream;

		stream << function << L" - " << message << endl;

		//Append the error location (it's clickable!)
		stream << L"\t" << file << L"(" << line << L")" << endl;

		//Send it all to the VS Output window
		OutputDebugStringW(stream.str().c_str());
	}

	void LogWarning(wstring message, const char* file, long line, const char* function)
	{
		wostringstream stream;

		stream << L"WARNING: " << function << L" - " << message << endl;

		//Append the error location (it's clickable!)
		stream << L"\t" << file << L"(" << line << L")" << endl;

		//Send it all to the VS Output window
		OutputDebugStringW(stream.str().c_str());
	}

	void LogError(wstring message, const char* file, long line, const char* function)
	{
		wostringstream stream;

		stream << L"ERROR: " << function << L" - " << message << endl;

		//Append the error location (it's clickable!)
		stream << "\t" << file << L"(" << line << L")" << endl;

		//Send it all to the VS Output window
		OutputDebugStringW(stream.str().c_str());

		__debugbreak();
	}

	void LogAssert(wstring expression, wstring message, const char* file, long line, const char* function)
	{
		wostringstream stream;

		stream << L"ASSERT FAILED: " << function << L" - " << expression << endl
		       << message << endl;

		//Append the error location (it's clickable!)
		stream << L"\t" << file << L"(" << line << L")" << endl;

		//Send it all to the VS Output window
		OutputDebugStringW(stream.str().c_str());

		__debugbreak();
	}

	bool LogIfFailed(HRESULT hr, const char* file, long line, const char* function)
	{
		//Success!
		if ( !FAILED(hr) ) { return false; }

		//Get a friendly string from the D3D error
		wchar_t* errorMessage;
		DWORD ret = FormatMessageW(
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
			stream << L"ERROR: " << function << L" - " << errorMessage;
		}
		//FormatMessage failed, log that too
		else
		{
			stream << L"ERROR: Failed to format error message from HRESULT: " << hr << L". FormatMessage error: " << GetLastError() << endl;
		}
		HeapFree(GetProcessHeap(), NULL, errorMessage);

		//Append the error location (it's clickable!)
		stream << L"\t" << file << L"(" << line << L")" << endl;

		//Send it all to the VS Output window
		OutputDebugStringW(stream.str().c_str());

		__debugbreak();
		return true;
	}
}