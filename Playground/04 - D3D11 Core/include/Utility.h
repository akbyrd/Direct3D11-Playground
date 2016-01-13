#pragma once

#include <wrl\client.h>
#include <memory>

namespace Utility
{
	template<typename T, size_t S>
	inline size_t ArraySize(const T (&arr)[S])
	{
		return S;
	}

	template<typename T>
	using ComPtr = Microsoft::WRL::ComPtr<T>;

	template<UINT TNameLength>
	inline void SetDebugObjectName(const ComPtr<ID3D11Device> &resource,
		const char (&name)[TNameLength])
	{
		#if defined(_DEBUG)
		resource->SetPrivateData(WKPDID_D3DDebugObjectName, TNameLength - 1, name);
		#endif
	}

	template<UINT TNameLength>
	inline void SetDebugObjectName(const ComPtr<ID3D11DeviceChild> &resource,
		const char (&name)[TNameLength])
	{
		#if defined(_DEBUG)
		resource->SetPrivateData(WKPDID_D3DDebugObjectName, TNameLength - 1, name);
		#endif
	}

	template<UINT TNameLength>
	inline void SetDebugObjectName(const ComPtr<IDXGIObject> &resource,
		const char (&name)[TNameLength])
	{
		#if defined(_DEBUG)
		resource->SetPrivateData(WKPDID_D3DDebugObjectName, TNameLength - 1, name);
		#endif
	}

	inline void SetDebugObjectName(const ComPtr<ID3D11DeviceChild> &resource,
		const std::string &name)
	{
		#if defined(_DEBUG)
		resource->SetPrivateData(WKPDID_D3DDebugObjectName, name.length(), name.c_str());
		#endif
	}

	bool LoadFile(const std::wstring &fileName, std::unique_ptr<char[]> &data, size_t &dataSize);

	ULONG GetRefCount(IUnknown *obj);
}