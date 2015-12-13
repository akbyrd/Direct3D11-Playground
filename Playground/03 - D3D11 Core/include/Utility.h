#pragma once

namespace Utility
{
	//Safely release a COM object.
	template<typename T>
	inline void SafeRelease(T& ptr)
	{
		if ( ptr != nullptr )
		{
			ptr->Release();
			ptr = nullptr;
		}
	}

	//Safely delete an object
	template<typename T>
	inline void SafeDelete(T& ptr)
	{
		if ( ptr != nullptr )
		{
			delete ptr;
			ptr = nullptr;
		}
	}

	long LoadFile(const std::wstring fileName, char* &data, SIZE_T &dataSize);
}