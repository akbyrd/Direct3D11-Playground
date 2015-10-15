#include "stdafx.h"

#include "Renderer.h"
#include "Utility.h"

#pragma comment(lib, "D3D11.lib")
#pragma comment(lib,  "DXGI.lib")

long Renderer::Initialize(HWND hwnd)
{
	long ret;

	this->hwnd = hwnd;
	if ( hwnd == nullptr )
	{
		LOG_ERROR("Failed. Null HWND was provided.");
		ret = ExitCode::BadHWNDProvided;
		goto Cleanup;
	}

	ret = InitializeDevice();       CHECK_RET(ret);
	ret = InitializeSwapChain();    CHECK_RET(ret);
	ret = InitializeDepthBuffer();  CHECK_RET(ret);
	ret = InitializeOutputMerger(); CHECK_RET(ret);
	ret = InitializeViewport();     CHECK_RET(ret);

	ret = ExitCode::Success;

Cleanup:

	return ret;
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
	); CHECK_HR(hr);

	//Check feature level
	if ( (featureLevel & D3D_FEATURE_LEVEL_11_0) != D3D_FEATURE_LEVEL_11_0 )
	{
		LOG_ERROR("Created device does not support D3D 11");
		hr = ExitCode::D3DFeatureLevelNotSupported;
		goto Cleanup;
	}

	//It's ok if this fails, it's an optional check
	CheckForWarpDriver();

	hr = ObtainDXGIFactory(); CHECK_RET(hr);

	hr = ExitCode::Success;

Cleanup:

	return hr;
}

long Renderer::ObtainDXGIFactory()
{
	HRESULT hr;

	if ( !pD3DDevice )
	{
		LOG_ERROR("Failed. D3D device not initialized.");
		hr = ExitCode::D3DDeviceNotInitialized;
		goto Cleanup;
	}

	//Obtain the DXGI factory used to create the current device
	IDXGIDevice1* pDXGIDevice = nullptr;
	hr = pD3DDevice->QueryInterface(__uuidof(IDXGIDevice1), (void**) &pDXGIDevice); CHECK_HR(hr);

	IDXGIAdapter1* pDXGIAdapter = nullptr;
	hr = pDXGIDevice->GetParent(__uuidof(IDXGIAdapter1), (void **) &pDXGIAdapter); CHECK_HR(hr);

	hr = pDXGIAdapter->GetParent(__uuidof(IDXGIFactory1), (void**) &pDXGIFactory); CHECK_HR(hr);

	hr = ExitCode::Success;

Cleanup:
	RELEASE_COM(pDXGIAdapter);
	RELEASE_COM(pDXGIDevice);

	return hr;
}

long Renderer::CheckForWarpDriver()
{
	HRESULT hr;

	//Check for the WARP driver
	IDXGIDevice1* pDXGIDevice = nullptr;
	hr = pD3DDevice->QueryInterface(__uuidof(IDXGIDevice1), reinterpret_cast<void**>(&pDXGIDevice)); CHECK_HR(hr);

	IDXGIAdapter* pDXGIAdapter = nullptr;
	hr = pDXGIDevice->GetAdapter(&pDXGIAdapter); CHECK_HR(hr);

	DXGI_ADAPTER_DESC desc;
	hr = pDXGIAdapter->GetDesc(&desc); CHECK_HR(hr);

	if ( (desc.VendorId == 0x1414) && (desc.DeviceId == 0x8c) )
	{
		// WARNING: Microsoft Basic Render Driver is active.
		// Performance of this application may be unsatisfactory.
		// Please ensure that your video card is Direct3D10/11 capable
		// and has the appropriate driver installed.
		LOG_WARNING("WARP driver in use.");
	}

	hr = ExitCode::Success;

Cleanup:
	RELEASE_COM(pDXGIAdapter);
	RELEASE_COM(pDXGIDevice);

	return hr;
}

//TODO: Rebuild swap chain on window resize
long Renderer::InitializeSwapChain()
{
	HRESULT hr;

	//Query and set MSAA quality levels
	hr = pD3DDevice->CheckMultisampleQualityLevels(DXGI_FORMAT_R8G8B8A8_UNORM, multiSampleCount, &numQualityLevels); CHECK_HR(hr);

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
	swapChainDesc.Windowed = !startFullscreen;
	swapChainDesc.SwapEffect = DXGI_SWAP_EFFECT_DISCARD;
	swapChainDesc.Flags = DXGI_SWAP_CHAIN_FLAG_ALLOW_MODE_SWITCH;

	//Create the swap chain
	hr = pDXGIFactory->CreateSwapChain(pD3DDevice, &swapChainDesc, &pSwapChain); CHECK_HR(hr);

	ID3D11Texture2D* pBackBuffer = nullptr;
	hr = pSwapChain->GetBuffer(0, __uuidof(ID3D11Texture2D), (void**) &pBackBuffer); CHECK_HR(hr);

	//Create a render target view to the back buffer
	hr = pD3DDevice->CreateRenderTargetView(pBackBuffer, nullptr, &pRenderTargetView); CHECK_HR(hr);

	hr = UpdateAllowFullscreen(); CHECK_HR(hr);

	hr = ExitCode::Success;

Cleanup:
	RELEASE_COM(pBackBuffer);

	return hr;
}

long Renderer::UpdateAllowFullscreen()
{
	HRESULT hr;

	//MakeWindowAssociation only works if the swap chain has been created.
	if ( !pSwapChain )
	{
		LOG_WARNING("Failed. The swap chain has not been initialized.");
		hr = ExitCode::SwapChainNotInitialized;
		goto Cleanup;
	}

	UINT flags = 0;
	if ( !allowFullscreen )
		flags |= DXGI_MWA_NO_ALT_ENTER;

	hr = pDXGIFactory->MakeWindowAssociation(hwnd, flags); CHECK_HR(hr);

	hr = ExitCode::Success;

Cleanup:

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
	hr = pD3DDevice->CreateTexture2D(&depthDesc, nullptr, &pDepthBuffer); CHECK_HR(hr);

	hr = pD3DDevice->CreateDepthStencilView(pDepthBuffer, nullptr, &pDepthBufferView); CHECK_HR(hr);

	hr = ExitCode::Success;

Cleanup:
	RELEASE_COM(pDepthBuffer);

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


long Renderer::LogAdapters()
{
	HRESULT hr;

	IDXGIFactory1* pDXGIFactory = nullptr;
	hr = CreateDXGIFactory1(__uuidof(IDXGIFactory1), (void**) &pDXGIFactory); CHECK_HR(hr);

	UINT i = 0;
	IDXGIAdapter1* pDXGIAdapter = nullptr;
	while ( true )
	{
		hr = pDXGIFactory->EnumAdapters1(i, &pDXGIAdapter);

		//We've run out of adapters
		if ( hr == DXGI_ERROR_NOT_FOUND ) { break; }

		//Other error
		CHECK_HR(hr);

		//Get adapter description
		DXGI_ADAPTER_DESC1 adapterDesc;
		hr = pDXGIAdapter->GetDesc1(&adapterDesc); CHECK_HR(hr);

		//Log the adapter description
		std::wstringstream stream;
		stream <<               L"Adapter: " << i                                                        << std::endl;
		stream <<           L"AdapterLuid: " << adapterDesc.AdapterLuid.HighPart
		                                     << adapterDesc.AdapterLuid.LowPart                          << std::endl;
		stream <<           L"Description: " << adapterDesc.Description                                  << std::endl;
		stream <<              L"VendorId: " << adapterDesc.VendorId                                     << std::endl;
		stream <<              L"DeviceId: " << adapterDesc.DeviceId                                     << std::endl;
		stream <<              L"SubSysId: " << adapterDesc.SubSysId                                     << std::endl;
		stream <<              L"Revision: " << adapterDesc.Revision                                     << std::endl;
		stream <<  L"DedicatedVideoMemory: " << adapterDesc.DedicatedVideoMemory  / 1048576.0f << L" MB" << std::endl;
		stream << L"DedicatedSystemMemory: " << adapterDesc.DedicatedSystemMemory / 1048576.0f << L" MB" << std::endl;
		stream <<    L"SharedSystemMemory: " << adapterDesc.SharedSystemMemory    / 1048576.0f << L" MB" << std::endl;
		stream <<                 L"Flags: " << adapterDesc.Flags                                        << std::endl;
		Logging::Log(stream);

		LogOutputs(pDXGIAdapter);

		RELEASE_COM(pDXGIAdapter);
		++i;
	}

	hr = ExitCode::Success;

Cleanup:
	RELEASE_COM(pDXGIAdapter);
	RELEASE_COM(pDXGIFactory);

	return hr;
}

long Renderer::LogOutputs(IDXGIAdapter1* pDXGIAdapter)
{
	HRESULT hr;

	UINT i = 0;
	IDXGIOutput* pDXGIOutput = nullptr;
	while ( true )
	{
		hr = pDXGIAdapter->EnumOutputs(i, &pDXGIOutput);
		if ( hr == DXGI_ERROR_NOT_FOUND ) { break; }

		CHECK_HR(hr);

		DXGI_OUTPUT_DESC outputDesc;
		hr = pDXGIOutput->GetDesc(&outputDesc); CHECK_HR(hr);

		std::wstringstream stream;

		stream <<             "Output: "    << i                                            << std::endl;
		stream <<         "DeviceName: "    << outputDesc.DeviceName                        << std::endl;
		stream << "DesktopCoordinates: (L:" << outputDesc.DesktopCoordinates.left
		                         << L", T:" << outputDesc.DesktopCoordinates.top
		                         << L", R:" << outputDesc.DesktopCoordinates.right
		                         << L", B:" << outputDesc.DesktopCoordinates.bottom << L")" << std::endl;
		stream <<  "AttachedToDesktop: "    << outputDesc.AttachedToDesktop                 << std::endl;
		stream <<           "Rotation: "    << outputDesc.Rotation                          << std::endl;
		stream <<            "Monitor: "    << outputDesc.Monitor                           << std::endl;

		Logging::Log(stream);

		LogDisplayModes(pDXGIOutput);

		RELEASE_COM(pDXGIOutput);
		++i;
	}

	hr = ExitCode::Success;

Cleanup:
	RELEASE_COM(pDXGIOutput);

	return hr;
}

long Renderer::LogDisplayModes(IDXGIOutput* pDXGIOutput)
{
	HRESULT hr;

	DXGI_FORMAT format = DXGI_FORMAT_R8G8B8A8_UNORM;

	//Get the number of modes
	UINT numModes;
	hr = pDXGIOutput->GetDisplayModeList(format, 0, &numModes, nullptr); CHECK_HR(hr);

	//Allocate space
	DXGI_MODE_DESC* arrModeDesc = new DXGI_MODE_DESC[numModes];

	//Get the actual display modes
	hr = pDXGIOutput->GetDisplayModeList(format, 0, &numModes, arrModeDesc); CHECK_HR(hr);

	{
		std::wstringstream stream;
		for ( UINT i = 0; i < numModes; ++i )
		{
			UINT  width = arrModeDesc[i].Width;
			UINT  height = arrModeDesc[i].Height;
			float refreshRate = arrModeDesc[i].RefreshRate.Numerator / (float) arrModeDesc[i].RefreshRate.Denominator;

			stream << width << L"x" << height << L" @ " << refreshRate << L" Hz" << std::endl;
		}
		Logging::Log(stream);
	}

	hr = ExitCode::Success;

Cleanup:
	if ( arrModeDesc )
	{
		delete arrModeDesc;
		arrModeDesc = nullptr;
	}

	return hr;
}


long Renderer::Update(GameTimer* gameTimer)
{
	HRESULT hr;

	double t = gameTimer->Time();
	float  r = (float) sin(1. * t);
	float  g = (float) sin(2. * t);
	float  b = (float) sin(3. * t);

	XMVECTORF32 color = { r, g, b, 1.0f };

	pD3DImmediateContext->ClearRenderTargetView(pRenderTargetView, color);
	pD3DImmediateContext->ClearDepthStencilView(pDepthBufferView, D3D11_CLEAR_DEPTH | D3D11_CLEAR_STENCIL, 1, 0);

	hr = pSwapChain->Present(0, 0); CHECK_HR(hr);

	hr = ExitCode::Success;

Cleanup:

	return hr;
}

long Renderer::Teardown()
{
	hwnd = nullptr;

	RELEASE_COM(pD3DDevice);
	RELEASE_COM(pD3DImmediateContext);
	RELEASE_COM(pDXGIFactory);
	RELEASE_COM(pSwapChain);
	RELEASE_COM(pRenderTargetView);
	RELEASE_COM(pDepthBufferView);

	//Check for leaks
	LogLiveObjects();

	return ExitCode::Success;
}

void Renderer::LogLiveObjects()
{
	#ifdef _DEBUG

	typedef HRESULT(WINAPI *fPtr)(const IID&, void**);
	fPtr DXGIGetDebugInterface = (fPtr) GetProcAddress(GetModuleHandle(TEXT("dxgidebug.dll")), "DXGIGetDebugInterface");
	if ( DXGIGetDebugInterface == nullptr )
	{
		LOG_ERROR("Failed to obtain dxgidebug.dll module or DXGIGetDebugInterface function pointer");
		return;
	}

	HRESULT hr;

	IDXGIDebug* pDXGIDebug = nullptr;
	hr = DXGIGetDebugInterface(__uuidof(IDXGIDebug), (void**) &pDXGIDebug); CHECK_HR(hr);

	pDXGIDebug->ReportLiveObjects(DXGI_DEBUG_ALL, DXGI_DEBUG_RLO_ALL);
	OutputDebugString(TEXT("\n"));

Cleanup:
	RELEASE_COM(pDXGIDebug);

	#endif
}