#include "stdafx.h"
#include "Renderer.h"
#include "D3D11Utility.h"

#pragma comment(lib, "D3D11.lib")

Renderer::Renderer() { }

void Renderer::Init()
{
	InitializeDevice();
	//InitializeSwapChain();
	//InitializeDXGIFactory();
	//InitializeRenderTargetView();
	//InitializeDepthBuffer();
	//InitializeResourceBindings();
	//InitializeViewport();
}

bool Renderer::InitializeDevice()
{
	UINT createDeviceFlags = D3D11_CREATE_DEVICE_SINGLETHREADED;
	#ifdef _DEBUG
	createDeviceFlags |= D3D11_CREATE_DEVICE_DEBUG | D3D11_CREATE_DEVICE_DEBUGGABLE;
	#endif

	D3D_FEATURE_LEVEL featureLevel;

	HRESULT hr = D3D11CreateDevice(
		nullptr,
		D3D_DRIVER_TYPE_HARDWARE,
		nullptr,
		createDeviceFlags,
		nullptr, 0,
		D3D11_SDK_VERSION,
		&d3dDevice,
		&featureLevel,
		&d3dDeviceContext
	);

	if ( LOG_ERROR(hr) )
		return hr;

	if ( (featureLevel & D3D_FEATURE_LEVEL_11_0) != D3D_FEATURE_LEVEL_11_0 )
		return false;

	return true;
}