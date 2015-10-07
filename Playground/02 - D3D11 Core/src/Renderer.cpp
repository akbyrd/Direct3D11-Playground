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

	result = InitializeDepthBuffer();
	if ( result < 0 ) { return result; }

	InitializeOutputMerger();
	if ( result < 0 ) { return result; }

	InitializeViewport();
	if ( result < 0 ) { return result; }

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

	D3D_FEATURE_LEVEL featureLevel;

	hr = D3D11CreateDevice(
		nullptr,
		D3D_DRIVER_TYPE_HARDWARE,
		nullptr,
		createDeviceFlags,
		nullptr, 0,
		D3D11_SDK_VERSION,
		&pD3DDevice,
		&featureLevel,
		&pD3DImmediateContext
	); CHECK(hr);

	//Check feature level
	if ( (featureLevel & D3D_FEATURE_LEVEL_11_0) != D3D_FEATURE_LEVEL_11_0 )
	{
		LOG_ERROR(L"Created device does not support D3D 11");
		hr = ExitCode::D3DFeatureLevelNotSupported;
		goto Cleanup;
	}

	hr = CheckForWarpDriver();

	hr = ExitCode::Success;

Cleanup:

	return hr;
}

long Renderer::CheckForWarpDriver()
{
	HRESULT hr;

	//Check for the WARP driver
	IDXGIDevice1* pDXGIDevice = nullptr;
	hr = pD3DDevice->QueryInterface(__uuidof(IDXGIDevice1), reinterpret_cast<void**>(&pDXGIDevice)); CHECK(hr);

	IDXGIAdapter* pDXGIAdapter = nullptr;
	hr = pDXGIDevice->GetAdapter(&pDXGIAdapter); CHECK(hr);

	DXGI_ADAPTER_DESC desc;
	hr = pDXGIAdapter->GetDesc(&desc); CHECK(hr);

	if ( (desc.VendorId == 0x1414) && (desc.DeviceId == 0x8c) )
	{
		// WARNING: Microsoft Basic Render Driver is active.
		// Performance of this application may be unsatisfactory.
		// Please ensure that your video card is Direct3D10/11 capable
		// and has the appropriate driver installed.
		LOG_WARNING(L"WARP driver in use.");
	}

	hr = ExitCode::Success;

Cleanup:
	RELEASE_COM(&pDXGIAdapter);
	RELEASE_COM(&pDXGIDevice);

	return hr;
}

//TODO: Rebuild swap chain on window resize
long Renderer::InitializeSwapChain()
{
	HRESULT hr;

	//Query and set MSAA quality levels
	hr = pD3DDevice->CheckMultisampleQualityLevels(DXGI_FORMAT_R8G8B8A8_UNORM, multiSampleCount, &numQualityLevels); CHECK(hr);

	//Set swap chain properties
	DXGI_SWAP_CHAIN_DESC swapChainDesc;
	swapChainDesc.BufferDesc.Width = width;
	swapChainDesc.BufferDesc.Height = height;
	swapChainDesc.BufferDesc.RefreshRate.Numerator = 60;
	swapChainDesc.BufferDesc.RefreshRate.Denominator = 1;
	swapChainDesc.BufferDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
	swapChainDesc.BufferDesc.ScanlineOrdering = DXGI_MODE_SCANLINE_ORDER_UNSPECIFIED;
	swapChainDesc.BufferDesc.Scaling = DXGI_MODE_SCALING_UNSPECIFIED;
	swapChainDesc.SampleDesc.Count = multiSampleCount;
	swapChainDesc.SampleDesc.Quality = numQualityLevels - 1;
	swapChainDesc.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
	swapChainDesc.BufferCount = 1;
	swapChainDesc.OutputWindow = hwnd;
	swapChainDesc.Windowed = true;
	swapChainDesc.SwapEffect = DXGI_SWAP_EFFECT_DISCARD;
	swapChainDesc.Flags = 0;

	//Obtain the DXGI factory used to create the current device
	IDXGIDevice1* pDXGIDevice = nullptr;
	hr = pD3DDevice->QueryInterface(__uuidof(IDXGIDevice1), (void**) &pDXGIDevice); CHECK(hr);

	IDXGIAdapter1* pDXGIAdapter = nullptr;
	hr = pDXGIDevice->GetParent(__uuidof(IDXGIAdapter1), (void **) &pDXGIAdapter); CHECK(hr);

	IDXGIFactory1* pDXGIFactory = nullptr;
	hr = pDXGIAdapter->GetParent(__uuidof(IDXGIFactory1), (void**) &pDXGIFactory); CHECK(hr);

	//Create the swap chain
	hr = pDXGIFactory->CreateSwapChain(pD3DDevice, &swapChainDesc, &pSwapChain); CHECK(hr);

	ID3D11Texture2D* pBackBuffer = nullptr;
	hr = pSwapChain->GetBuffer(0, __uuidof(ID3D11Texture2D), (void**) &pBackBuffer); CHECK(hr);

	//Create a render target view to the back buffer
	hr = pD3DDevice->CreateRenderTargetView(pBackBuffer, nullptr, &pRenderTargetView); CHECK(hr);

	hr = ExitCode::Success;

Cleanup:
	RELEASE_COM(&pDXGIFactory);
	RELEASE_COM(&pDXGIAdapter);
	RELEASE_COM(&pDXGIDevice);
	RELEASE_COM(&pBackBuffer);

	return hr;
}

long Renderer::InitializeDepthBuffer()
{
	HRESULT hr;

	D3D11_TEXTURE2D_DESC depthDesc;
	depthDesc.Width = width;
	depthDesc.Height = height;
	depthDesc.MipLevels = 1;
	depthDesc.ArraySize = 1;
	depthDesc.Format = DXGI_FORMAT_D24_UNORM_S8_UINT;
	depthDesc.SampleDesc.Count = multiSampleCount;
	depthDesc.SampleDesc.Quality = numQualityLevels - 1;
	depthDesc.Usage = D3D11_USAGE_DEFAULT;
	depthDesc.BindFlags = D3D11_BIND_DEPTH_STENCIL;
	depthDesc.CPUAccessFlags = 0;
	depthDesc.MiscFlags = 0;

	ID3D11Texture2D* pDepthBuffer = nullptr;
	hr = pD3DDevice->CreateTexture2D(&depthDesc, nullptr, &pDepthBuffer); CHECK(hr);

	hr = pD3DDevice->CreateDepthStencilView(pDepthBuffer, nullptr, &pDepthBufferView); CHECK(hr);

	hr = ExitCode::Success;

Cleanup:
	RELEASE_COM(&pDepthBuffer);

	return hr;
}

long Renderer::InitializeOutputMerger()
{
	pD3DImmediateContext->OMSetRenderTargets(1, &pRenderTargetView, pDepthBufferView);

	return ExitCode::Success;
}

long Renderer::InitializeViewport()
{
	D3D11_VIEWPORT viewport;
	viewport.TopLeftX = 0;
	viewport.TopLeftY = 0;
	viewport.Width = (FLOAT) width;
	viewport.Height = (FLOAT) height;
	viewport.MinDepth = 0;
	viewport.MaxDepth = 1;

	pD3DImmediateContext->RSSetViewports(0, &viewport);

	return ExitCode::Success;
}

long Renderer::Update(GameTimer* gameTimer)
{
	HRESULT hr;

	float r = sin(1. * gameTimer->Time());
	float g = sin(2. * gameTimer->Time());
	float b = sin(3. * gameTimer->Time());

	XMVECTORF32 color = { r, g, b, 1.0f };

	pD3DImmediateContext->ClearRenderTargetView(pRenderTargetView, color);
	pD3DImmediateContext->ClearDepthStencilView(pDepthBufferView, D3D11_CLEAR_DEPTH | D3D11_CLEAR_STENCIL, 1, 0);

	hr = pSwapChain->Present(0, 0); CHECK(hr);

	hr = ExitCode::Success;

Cleanup:

	return hr;
}

long Renderer::Teardown()
{
	RELEASE_COM(&pD3DDevice);
	RELEASE_COM(&pD3DImmediateContext);
	RELEASE_COM(&pSwapChain);
	RELEASE_COM(&pRenderTargetView);

	hwnd = nullptr;

	return ExitCode::Success;
}