#include "stdafx.h"
#include "Utility.h"

using namespace std;

namespace Utility
{
	long LoadFile(const wstring fileName, char* &data, SIZE_T &dataSize)
	{
		long ret;

		ifstream inFile(fileName, ios::binary | ios::ate);
		if ( !inFile.is_open() )
		{
			wstring errorMessage = L"Failed to open file: " + fileName;
			Logging::LogError(errorMessage, __FILE__, __LINE__, __FUNCTION__);

			ret = ExitCode::FileOpenFailed;
			goto Cleanup;
		}

		dataSize = (SIZE_T) inFile.tellg();
		data = new char[dataSize];

		inFile.seekg(0);
		inFile.read(data, dataSize);

		if ( inFile.bad() )
		{
			dataSize = 0;
			delete data;
			data = nullptr;

			wstring errorMessage = L"Failed to read file: " + fileName;
			Logging::LogError(errorMessage, __FILE__, __LINE__, __FUNCTION__);

			ret = ExitCode::FileReadFailed;
			goto Cleanup;
		}

		ret = ExitCode::Success;

	Cleanup:

		return ret;
	}
}