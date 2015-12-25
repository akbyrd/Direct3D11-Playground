#pragma once

#include <memory>

namespace Utility
{
	template<UINT TNameLength>
	inline void SetDebugObjectName(_In_ ID3D11Device* resource,
		_In_z_ const char (&name)[TNameLength])
	{
		#if defined(_DEBUG) || defined(PROFILE)
		resource->SetPrivateData(WKPDID_D3DDebugObjectName, TNameLength - 1, name);
		#endif
	}

	template<UINT TNameLength>
	inline void SetDebugObjectName(_In_ ID3D11DeviceChild* resource,
		_In_z_ const char (&name)[TNameLength])
	{
		#if defined(_DEBUG) || defined(PROFILE)
		resource->SetPrivateData(WKPDID_D3DDebugObjectName, TNameLength - 1, name);
		#endif
	}

	template<UINT TNameLength>
	inline void SetDebugObjectName(_In_ IDXGIObject* resource,
		_In_z_ const char (&name)[TNameLength])
	{
		#if defined(_DEBUG) || defined(PROFILE)
		resource->SetPrivateData(WKPDID_D3DDebugObjectName, TNameLength - 1, name);
		#endif
	}

	bool LoadFile(const std::wstring fileName, std::unique_ptr<char[]> &data, SIZE_T &dataSize);
}