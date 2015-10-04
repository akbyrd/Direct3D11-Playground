#include "stdafx.h"
#include "Renderer.h"
#include "Logging.h"
#include "Utility.h"

#pragma comment(lib, "D3D11.lib")
#pragma comment(lib, "DXGI.lib")

Renderer::Renderer(HWND hwnd)
{
	Renderer::hwnd = hwnd;
	this->hwnd = hwnd;
}

long Renderer::Initialize()
{
	long result;

	result = InitializeDevice();
	if ( result < 0 ) { return result; }

	result = InitializeSwapChain();
	if ( result < 0 ) { return result; }

	//InitializeDXGIFactory();
	//InitializeRenderTargetView();
	//InitializeDepthBuffer();
	//InitializeResourceBindings();
	//InitializeViewport();

	return ExitCode::Success;
}

long Renderer::InitializeDevice()
{
	HRESULT hr;

	UINT createDeviceFlags = D3D11_CREATE_DEVICE_SINGLETHREADED;
	#ifdef _DEBUG
	createDeviceFlags |= D3D11_CREATE_DEVICE_DEBUG;
	//createDeviceFlags |= D3D11_CREATE_DEVICE_DEBUGGABLE; //11_1+
	#endif

	D3D_FEATURE_LEVEL    featureLevel;
	ID3D11Device*        pD3D11Device;
	ID3D11DeviceContext* pD3D11DeviceContext;

	hr = D3D11CreateDevice(
		nullptr,
		D3D_DRIVER_TYPE_HARDWARE,
		nullptr,
		createDeviceFlags,
		nullptr, 0,
		D3D11_SDK_VERSION,
		&pD3D11Device,
		&featureLevel,
		&pD3D11DeviceContext
	);
	if ( LOG_IF_FAILED(hr) ) { return hr; }

	//Check feature level
	if ( (featureLevel & D3D_FEATURE_LEVEL_11_0) != D3D_FEATURE_LEVEL_11_0 )
		return ExitCode::D3DFeatureLevelNotSupported;

	//Get device and context for 11.2
	hr = pD3D11Device->QueryInterface(__uuidof(ID3D11Device2), (void**) &pD3DDevice);
	if ( LOG_IF_FAILED(hr) ) { return hr; }
	pD3D11Device->Release();

	hr = pD3D11DeviceContext->QueryInterface(__uuidof(ID3D11DeviceContext2), (void**) &pD3DImmediateContext);
	if ( LOG_IF_FAILED(hr) ) { return hr; }
	pD3D11DeviceContext->Release();

	//Check for the WARP driver
	IDXGIDevice3* dxgiDevice = nullptr;
	hr = pD3DDevice->QueryInterface(__uuidof(IDXGIDevice3), reinterpret_cast<void**>(&dxgiDevice));
	if ( !LOG_IF_FAILED(hr) )
	{
		IDXGIAdapter* adapter = nullptr;
		hr = dxgiDevice->GetAdapter(&adapter);
		if ( !LOG_IF_FAILED(hr) )
		{
			DXGI_ADAPTER_DESC desc;
			hr = adapter->GetDesc(&desc);
			if ( !LOG_IF_FAILED(hr) )
			{
				if ( (desc.VendorId == 0x1414) && (desc.DeviceId == 0x8c) )
				{
					// WARNING: Microsoft Basic Render Driver is active.
					// Performance of this application may be unsatisfactory.
					// Please ensure that your video card is Direct3D10/11 capable
					// and has the appropriate driver installed.
				}
			}
			adapter->Release();
		}
		dxgiDevice->Release();
	}

	return ExitCode::Success;
}

long Renderer::InitializeSwapChain()
{
	HRESULT hr;

	//Query and set MSAA quality levels
	hr = pD3DDevice->CheckMultisampleQualityLevels1(DXGI_FORMAT_R8G8B8A8_UNORM, multiSampleCount, 0, &numQualityLevels);
	if ( LOG_IF_FAILED(hr) ) { return hr; }

	DXGI_SAMPLE_DESC sampleDesc;
	sampleDesc.Count = multiSampleCount;
	sampleDesc.Quality = numQualityLevels - 1;

	//Set swap chain properties
	//DXGI_SWAP_CHAIN_DESC1 swapChainDesc;
	//swapChainDesc.Width = width;
	//swapChainDesc.Height = height;
	//swapChainDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
	//swapChainDesc.Stereo = false;
	//swapChainDesc.SampleDesc = sampleDesc;
	//swapChainDesc.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
	//swapChainDesc.BufferCount = 1;
	//swapChainDesc.Scaling = DXGI_SCALING_ASPECT_RATIO_STRETCH; //Probably Windows Phone only
	//swapChainDesc.SwapEffect = DXGI_SWAP_EFFECT_DISCARD;
	//swapChainDesc.AlphaMode = DXGI_ALPHA_MODE_STRAIGHT;
	//swapChainDesc.Flags = 0;

	DXGI_SWAP_CHAIN_DESC swapChainDesc;
	swapChainDesc.BufferDesc.Width = width;
	swapChainDesc.BufferDesc.Height = height;
	swapChainDesc.BufferDesc.RefreshRate.Numerator = 60;
	swapChainDesc.BufferDesc.RefreshRate.Denominator = 1;
	swapChainDesc.BufferDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
	swapChainDesc.BufferDesc.ScanlineOrdering = DXGI_MODE_SCANLINE_ORDER_UNSPECIFIED;
	swapChainDesc.BufferDesc.Scaling = DXGI_MODE_SCALING_UNSPECIFIED;
	swapChainDesc.SampleDesc.Count = 1;
	swapChainDesc.SampleDesc.Quality = 0;
	swapChainDesc.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
	swapChainDesc.BufferCount = 1;
	swapChainDesc.OutputWindow = hwnd;
	swapChainDesc.Windowed = true;
	swapChainDesc.SwapEffect = DXGI_SWAP_EFFECT_DISCARD;
	swapChainDesc.Flags = 0;

	//TODO: Redo this by hand
	IDXGIDevice3* dxgiDevice = 0;
	hr = pD3DDevice->QueryInterface(__uuidof(IDXGIDevice3), (void**) &dxgiDevice);
	if ( LOG_IF_FAILED(hr) ) { return hr; }

	IDXGIAdapter2* dxgiAdapter = 0;
	hr = dxgiDevice->GetParent(__uuidof(IDXGIAdapter2), (void**) &dxgiAdapter);
	if ( LOG_IF_FAILED(hr) ) { return hr; }

	IDXGIFactory3* dxgiFactory = 0;
	hr = dxgiAdapter->GetParent(__uuidof(IDXGIFactory3), (void**) &dxgiFactory);
	if ( LOG_IF_FAILED(hr) ) { return hr; }

	IDXGISwapChain* mSwapChain;
	hr = dxgiFactory->CreateSwapChain(pD3DDevice, &swapChainDesc, &mSwapChain);
	if ( LOG_IF_FAILED(hr) ) { return hr; }

	//TODO: This seems to work, but it's not supposed to?
	IDXGIFactory3* pDXGIFactory;
	CreateDXGIFactory(__uuidof(IDXGIFactory3), (void**) &pDXGIFactory);

	IDXGISwapChain* mSwapChain2;
	hr = pDXGIFactory->CreateSwapChain(
		(IUnknown*) pD3DDevice,
		&swapChainDesc,
		&mSwapChain2
	);
	if ( LOG_IF_FAILED(hr) ) { return hr; }

	return ExitCode::Success;
}

long Renderer::Update()
{
	return ExitCode::Success;
}

long Renderer::Teardown()
{
	RELEASE_COM(&pD3DDevice);
	RELEASE_COM(&pD3DImmediateContext);
	RELEASE_COM(&pSwapChain);

	hwnd = nullptr;

	return ExitCode::Success;
}