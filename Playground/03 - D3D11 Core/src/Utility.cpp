#include "stdafx.h"

#include "Utility.h"

using namespace std;

namespace Utility
{
	//TODO: Pass in smart pointer?
	bool LoadFile(const wstring fileName, char* &data, SIZE_T &dataSize)
	{
		ifstream inFile(fileName, ios::binary | ios::ate);
		if ( !inFile.is_open() )
		{
			LOG_ERROR(L"Failed to open file: " + fileName);
			return false;
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

			LOG_ERROR(L"Failed to read file: " + fileName);
			return false;
		}

		return true;
	}
}