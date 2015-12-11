#pragma once

using namespace DirectX;

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
}

namespace Color
{
	XMGLOBALCONST XMVECTORF32 Red   = { 1.0f, 0.0f, 0.0f, 1.0f };
	XMGLOBALCONST XMVECTORF32 Green = { 0.0f, 1.0f, 0.0f, 1.0f };
	XMGLOBALCONST XMVECTORF32 Blue  = { 0.0f, 0.0f, 1.0f, 1.0f };
}