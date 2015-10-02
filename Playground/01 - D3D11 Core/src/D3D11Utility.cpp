#include "stdafx.h"
#include "D3D11Utility.h"

bool D3D11Utility::LogError(HRESULT hr, wchar_t* file, long line, wchar_t* function)
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

	return true;
}