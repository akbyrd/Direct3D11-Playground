#pragma once

#include <wrl\client.h>
#include <memory>

#include <d3d11.h>

namespace Utility
{
	template<typename T, size_t S>
	inline size_t ArrayCount(const T (&arr)[S])
	{
		return S;
	}

	//TODO: Update usage sites and rename
	template<typename T, size_t S>
	inline size_t ArraySize(const T (&arr)[S])
	{
		return S * sizeof(T);
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

	//TODO: Probably don't use std:string
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