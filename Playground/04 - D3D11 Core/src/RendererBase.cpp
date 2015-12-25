#include "stdafx.h"

#include <iomanip>
#include <memory>
#include <sstream>

#include "RendererBase.h"
#include "LoggedException.h"
#include "AssertionException.h"
#include "Utility.h"

#pragma comment(lib, "D3D11.lib")
#pragma comment(lib,  "DXGI.lib")

using namespace std;
using namespace Utility;
using namespace DirectX;

bool RendererBase::Initialize(HWND hwnd)
{
	      SetHwnd(hwnd);
	CHECK(InitializeDevice());
	CHECK(InitializeSwapChain());
	CHECK(InitializeDepthBuffer());
	      InitializeOutputMerger();
	      InitializeViewport();
	CHECK(OnInitialize());

	return true;
}

void RendererBase::SetHwnd(HWND hwnd)
{
	throw_assert(hwnd, L"Null HWND was provided.");

	RendererBase::hwnd = hwnd;
}

bool RendererBase::InitializeDevice()
{
	HRESULT hr;

	UINT createDeviceFlags = D3D11_CREATE_DEVICE_SINGLETHREADED;
	#ifdef _DEBUG
	createDeviceFlags |= D3D11_CREATE_DEVICE_DEBUG;
	//createDeviceFlags |= D3D11_CREATE_DEVICE_DEBUGGABLE; //11_1+
	#endif

	D3D_FEATURE_LEVEL featureLevel = {};

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
	SetDebugObjectName(pD3DDevice, "Device");
	SetDebugObjectName(pD3DImmediateContext, "Device Context");

	//Check feature level
	if ( (featureLevel & D3D_FEATURE_LEVEL_11_0) != D3D_FEATURE_LEVEL_11_0 )
		throw_logged(L"Created device does not support D3D 11");

	//It's ok if this fails, it's an optional check
	CheckForWarpDriver();

	hr = ObtainDXGIFactory(); CHECK_HR(hr);

	return true;
}

bool RendererBase::ObtainDXGIFactory()
{
	throw_assert(pD3DDevice, L"D3D device not initialized.");

	HRESULT hr;

	/* Obtain the DXGI factory used to create the current device.
	 * 
	 * NOTE: It looks like the IDXGIDevice is actually the same object as the ID3D11Device.
	 * Using SetPrivateData to set it's name clobbers the D3D device name and outputs a warning.
	 */
	CComPtr<IDXGIDevice1> pDXGIDevice;
	hr = pD3DDevice->QueryInterface(__uuidof(IDXGIDevice1), (void**) &pDXGIDevice); CHECK_HR(hr);

	CComPtr<IDXGIAdapter1> pDXGIAdapter;
	hr = pDXGIDevice->GetParent(__uuidof(IDXGIAdapter1), (void **) &pDXGIAdapter); CHECK_HR(hr);
	SetDebugObjectName(pDXGIAdapter, "DXGI Adapter");

	hr = pDXGIAdapter->GetParent(__uuidof(IDXGIFactory1), (void**) &pDXGIFactory); CHECK_HR(hr);
	SetDebugObjectName(pDXGIFactory, "DXGI Factory");

	return true;
}

bool RendererBase::CheckForWarpDriver()
{
	throw_assert(pD3DDevice, L"D3D device not initialized.");

	HRESULT hr;

	//Check for the WARP driver
	CComPtr<IDXGIDevice1> pDXGIDevice;
	hr = pD3DDevice->QueryInterface(__uuidof(IDXGIDevice1), (void**) &pDXGIDevice); CHECK_HR(hr);

	CComPtr<IDXGIAdapter> pDXGIAdapter;
	hr = pDXGIDevice->GetAdapter(&pDXGIAdapter); CHECK_HR(hr);

	DXGI_ADAPTER_DESC desc;
	hr = pDXGIAdapter->GetDesc(&desc); CHECK_HR(hr);

	if ( (desc.VendorId == 0x1414) && (desc.DeviceId == 0x8c) )
	{
		// WARNING: Microsoft Basic Render Driver is active.
		// Performance of this application may be unsatisfactory.
		// Please ensure that your video card is Direct3D10/11 capable
		// and has the appropriate driver installed.
		LOG_WARNING(L"WARP driver in use.");
	}

	return true;
}

bool RendererBase::InitializeSwapChain()
{
	throw_assert(pD3DDevice, L"D3D device not initialized.");
	throw_assert(pDXGIFactory, L"DXGI factory not initialized.");

	HRESULT hr;

	//Query and set MSAA quality levels
	hr = pD3DDevice->CheckMultisampleQualityLevels(DXGI_FORMAT_R8G8B8A8_UNORM, multiSampleCount, &numQualityLevels); CHECK_HR(hr);

	//Get the actual window size, just in case
	RECT rect = {};
	if ( GetClientRect(hwnd, &rect) )
	{
		width  = rect.right - rect.left;
		height = rect.bottom - rect.top;
	}
	else
	{
		hr = GetLastError(); CHECK_HR(hr);
	}

	//Set swap chain properties
	DXGI_SWAP_CHAIN_DESC swapChainDesc;
	swapChainDesc.BufferDesc.Width                   = width;
	swapChainDesc.BufferDesc.Height                  = height;
	swapChainDesc.BufferDesc.RefreshRate.Numerator   = 60;
	swapChainDesc.BufferDesc.RefreshRate.Denominator = 1;
	swapChainDesc.BufferDesc.Format                  = DXGI_FORMAT_R8G8B8A8_UNORM;
	swapChainDesc.BufferDesc.ScanlineOrdering        = DXGI_MODE_SCANLINE_ORDER_UNSPECIFIED;
	swapChainDesc.BufferDesc.Scaling                 = DXGI_MODE_SCALING_UNSPECIFIED;
	swapChainDesc.SampleDesc.Count                   = multiSampleCount;
	swapChainDesc.SampleDesc.Quality                 = numQualityLevels - 1;
	swapChainDesc.BufferUsage                        = DXGI_USAGE_RENDER_TARGET_OUTPUT;
	swapChainDesc.BufferCount                        = 1;
	swapChainDesc.OutputWindow                       = hwnd;
	swapChainDesc.Windowed                           = !startFullscreen;
	swapChainDesc.SwapEffect                         = DXGI_SWAP_EFFECT_DISCARD;
	swapChainDesc.Flags                              = DXGI_SWAP_CHAIN_FLAG_ALLOW_MODE_SWITCH;

	//Create the swap chain
	hr = pDXGIFactory->CreateSwapChain(pD3DDevice, &swapChainDesc, &pSwapChain); CHECK_HR(hr);
	SetDebugObjectName(pSwapChain, "Swap Chain");

	hr = CreateBackBufferView(); CHECK_HR(hr);
	hr = UpdateAllowFullscreen(); CHECK_HR(hr);

	return true;
}

bool RendererBase::CreateBackBufferView()
{
	throw_assert(pD3DDevice, L"D3D device not initialized.");
	throw_assert(pSwapChain, L"The swap chain has not been initialized.");

	HRESULT hr;

	CComPtr<ID3D11Texture2D> pBackBuffer;
	hr = pSwapChain->GetBuffer(0, __uuidof(ID3D11Texture2D), (void**) &pBackBuffer); CHECK_HR(hr);
	SetDebugObjectName(pBackBuffer, "Back Buffer");

	//Create a render target view to the back buffer
	hr = pD3DDevice->CreateRenderTargetView(pBackBuffer, nullptr, &pRenderTargetView); CHECK_HR(hr);
	SetDebugObjectName(pRenderTargetView, "Render Target View");

	return true;
}

bool RendererBase::UpdateAllowFullscreen()
{
	//MakeWindowAssociation only works if the swap chain has been created.
	throw_assert(pSwapChain, L"The swap chain has not been initialized.");
	throw_assert(pDXGIFactory, L"DXGI factory not initialized.");

	HRESULT hr;

	UINT flags = 0;
	if ( !allowFullscreen )
		flags |= DXGI_MWA_NO_ALT_ENTER;

	hr = pDXGIFactory->MakeWindowAssociation(hwnd, flags); CHECK_HR(hr);

	return true;
}

bool RendererBase::InitializeDepthBuffer()
{
	throw_assert(pD3DDevice, L"D3D device not initialized.");

	HRESULT hr;

	D3D11_TEXTURE2D_DESC depthDesc = {};
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

	CComPtr<ID3D11Texture2D> pDepthBuffer;
	hr = pD3DDevice->CreateTexture2D(&depthDesc, nullptr, &pDepthBuffer); CHECK_HR(hr);
	SetDebugObjectName(pDepthBuffer, "Depth Buffer");

	hr = pD3DDevice->CreateDepthStencilView(pDepthBuffer, nullptr, &pDepthBufferView); CHECK_HR(hr);
	SetDebugObjectName(pDepthBufferView, "Depth Buffer View");

	return true;
}

void RendererBase::InitializeOutputMerger()
{
	throw_assert(pD3DImmediateContext, L"D3D context not initialized.");

	pD3DImmediateContext->OMSetRenderTargets(1, &pRenderTargetView.p, pDepthBufferView);
}

void RendererBase::InitializeViewport()
{
	throw_assert(pD3DImmediateContext, L"D3D context not initialized.");

	D3D11_VIEWPORT viewport = {};
	viewport.TopLeftX = 0;
	viewport.TopLeftY = 0;
	viewport.Width = (FLOAT) width;
	viewport.Height = (FLOAT) height;
	viewport.MinDepth = 0;
	viewport.MaxDepth = 1;

	pD3DImmediateContext->RSSetViewports(1, &viewport);
}


bool RendererBase::LogAdapters()
{
	HRESULT hr;

	CComPtr<IDXGIFactory1> pDXGIFactory;
	hr = CreateDXGIFactory1(__uuidof(IDXGIFactory1), (void**) &pDXGIFactory); CHECK_HR(hr);

	UINT i = 0;
	while ( true )
	{
		CComPtr<IDXGIAdapter1> pDXGIAdapter;
		hr = pDXGIFactory->EnumAdapters1(i, &pDXGIAdapter);

		//We've run out of adapters
		if ( hr == DXGI_ERROR_NOT_FOUND ) { break; }

		//Other errors
		CHECK_HR(hr);

		//Get adapter description
		DXGI_ADAPTER_DESC1 adapterDesc;
		hr = pDXGIAdapter->GetDesc1(&adapterDesc); CHECK_HR(hr);

		//Log the adapter description
		wostringstream stream;
		stream <<               L"Adapter: " << i                                                        << endl;
		stream <<           L"AdapterLuid: " << adapterDesc.AdapterLuid.HighPart
		                                     << adapterDesc.AdapterLuid.LowPart                          << endl;
		stream <<           L"Description: " << adapterDesc.Description                                  << endl;
		stream <<              L"VendorId: " << adapterDesc.VendorId                                     << endl;
		stream <<              L"DeviceId: " << adapterDesc.DeviceId                                     << endl;
		stream <<              L"SubSysId: " << adapterDesc.SubSysId                                     << endl;
		stream <<              L"Revision: " << adapterDesc.Revision                                     << endl;
		stream <<  L"DedicatedVideoMemory: " << adapterDesc.DedicatedVideoMemory  / 1048576.0f << L" MB" << endl;
		stream << L"DedicatedSystemMemory: " << adapterDesc.DedicatedSystemMemory / 1048576.0f << L" MB" << endl;
		stream <<    L"SharedSystemMemory: " << adapterDesc.SharedSystemMemory    / 1048576.0f << L" MB" << endl;
		stream <<                 L"Flags: " << adapterDesc.Flags                                        << endl;
		Logging::Log(stream);

		LogOutputs(pDXGIAdapter);

		++i;
	}

	return true;
}

bool RendererBase::LogOutputs(CComPtr<IDXGIAdapter1> pDXGIAdapter)
{
	throw_assert(pDXGIAdapter, L"pDXGIAdapter is null.");

	HRESULT hr;

	UINT i = 0;
	while ( true )
	{
		CComPtr<IDXGIOutput> pDXGIOutput;
		hr = pDXGIAdapter->EnumOutputs(i, &pDXGIOutput);

		//We've run out of outputs
		if ( hr == DXGI_ERROR_NOT_FOUND ) { break; }

		CHECK_HR(hr);

		DXGI_OUTPUT_DESC outputDesc = {};
		hr = pDXGIOutput->GetDesc(&outputDesc); CHECK_HR(hr);

		wostringstream stream;

		stream <<             "Output: "    << i                                            << endl;
		stream <<         "DeviceName: "    << outputDesc.DeviceName                        << endl;
		stream << "DesktopCoordinates: (L:" << outputDesc.DesktopCoordinates.left
		                         << L", T:" << outputDesc.DesktopCoordinates.top
		                         << L", R:" << outputDesc.DesktopCoordinates.right
		                         << L", B:" << outputDesc.DesktopCoordinates.bottom << L")" << endl;
		stream <<  "AttachedToDesktop: "    << outputDesc.AttachedToDesktop                 << endl;
		stream <<           "Rotation: "    << outputDesc.Rotation                          << endl;
		stream <<            "Monitor: "    << outputDesc.Monitor                           << endl;

		Logging::Log(stream);

		LogDisplayModes(pDXGIOutput);

		++i;
	}

	return true;
}

bool RendererBase::LogDisplayModes(CComPtr<IDXGIOutput> pDXGIOutput)
{
	HRESULT hr;

	DXGI_FORMAT format = DXGI_FORMAT_R8G8B8A8_UNORM;

	//Get the number of modes
	UINT numModes;
	hr = pDXGIOutput->GetDisplayModeList(format, 0, &numModes, nullptr); CHECK_HR(hr);

	//Allocate space
	unique_ptr<DXGI_MODE_DESC[]> arrModeDesc(new DXGI_MODE_DESC[numModes]);

	//Get the actual display modes
	hr = pDXGIOutput->GetDisplayModeList(format, 0, &numModes, arrModeDesc.get()); CHECK_HR(hr);

	wostringstream stream;
	for ( UINT i = 0; i < numModes; ++i )
	{
		UINT  width = arrModeDesc[i].Width;
		UINT  height = arrModeDesc[i].Height;
		float refreshRate = arrModeDesc[i].RefreshRate.Numerator / (float) arrModeDesc[i].RefreshRate.Denominator;

		stream << width << L"x" << height << L" @ " << refreshRate << L" Hz" << endl;
	}
	Logging::Log(stream);

	return true;
}


bool RendererBase::Resize()
{
	throw_assert(pSwapChain, L"The swap chain has not been initialized.");

	HRESULT hr;

	//Get the new window size
	RECT rect = {};
	if ( GetClientRect(hwnd, &rect) )
	{
		width  = rect.right - rect.left;
		height = rect.bottom - rect.top;
	}
	else
	{
		hr = GetLastError(); CHECK_HR(hr);
	}

	//Preserve the swap chain configuration
	DXGI_SWAP_CHAIN_DESC swapChainDesc;
	hr = pSwapChain->GetDesc(&swapChainDesc); CHECK_HR(hr);

	//Skip resizing if it's not necessary
	if ( swapChainDesc.BufferDesc.Width  == width
	  && swapChainDesc.BufferDesc.Height == height )
	{
		return true;
	}

	//Release the old resource views (required to resize)
	pRenderTargetView.Release();
	pDepthBufferView.Release();

	hr = pSwapChain->ResizeBuffers(1, width, height, swapChainDesc.BufferDesc.Format, swapChainDesc.Flags); CHECK_HR(hr);

	hr = CreateBackBufferView(); CHECK_HR(hr);
	hr = InitializeDepthBuffer(); CHECK_HR(hr);

	InitializeViewport();

	hr = OnResize(); CHECK_HR(hr);

	return true;
}

bool RendererBase::Update(const GameTimer &gameTimer)
{
	HRESULT hr;

	const float t = (float) gameTimer.Time();
	const float r = sinf(1.0f * t);
	const float g = sinf(2.0f * t);
	const float b = sinf(3.0f * t);

	const XMVECTORF32 color = { r, g, b, 1.0f };

	pD3DImmediateContext->ClearRenderTargetView(pRenderTargetView, color);
	pD3DImmediateContext->ClearDepthStencilView(pDepthBufferView, D3D11_CLEAR_DEPTH | D3D11_CLEAR_STENCIL, 1, 0);

	hr = pSwapChain->Present(0, 0); CHECK_HR(hr);

	UpdateFrameStatistics(gameTimer);

	return true;
}

void RendererBase::UpdateFrameStatistics(const GameTimer &gameTimer)
{
	const static int bufferSize = 30;

	static double buffer[bufferSize];
	static int head = -1;
	static int length = 0;
	static double deltaToMS;

	//HACK: Hate this
	if ( length == 0 )
		buffer[bufferSize - 1] = gameTimer.RealTime();

	//Update the head position and length
	head = (head + 1) % bufferSize;
	if ( length < bufferSize - 1 )
	{
		++length;
		deltaToMS = 1000. / length;
	}

	//Update the head value
	buffer[head] = gameTimer.RealTime();

	int tail = (head - length) % bufferSize;
	if ( tail < 0 )
		tail += bufferSize;

	double delta = buffer[head] - buffer[tail];
	averageFrameTime = delta * deltaToMS;

	//Update FPS in window title once a second
	static double lastFPSUpdateTime = DBL_EPSILON;
	if ( gameTimer.RealTime() - lastFPSUpdateTime >= .5f )
	{
		lastFPSUpdateTime = gameTimer.RealTime();

		wostringstream outs;
		outs << L"FPS: " << setprecision(0) << fixed << (1000 / averageFrameTime);
		outs << L"   Frame Time: " << setprecision(2) << averageFrameTime << L" ms";
		outs << L"   (" << width << L" x " << height << L")";

		SetWindowTextW(hwnd, outs.str().c_str());
	}

	return;
}

void RendererBase::Teardown()
{
	OnTeardown();

	hwnd = nullptr;

	//TODO: This doesn't need to be done
	pD3DDevice.Release();
	pD3DImmediateContext.Release();
	pDXGIFactory.Release();
	pSwapChain.Release();
	pRenderTargetView.Release();
	pDepthBufferView.Release();

	//Check for leaks
	LogLiveObjects();
}

bool RendererBase::OnInitialize() { return true; }
bool RendererBase::OnResize()     { return true; }
void RendererBase::OnTeardown()   { }

bool RendererBase::LogLiveObjects()
{
	#ifdef _DEBUG

	typedef HRESULT(WINAPI *fPtr)(const IID&, void**);
	fPtr DXGIGetDebugInterface = (fPtr) GetProcAddress(GetModuleHandleW(L"dxgidebug.dll"), "DXGIGetDebugInterface");
	if ( DXGIGetDebugInterface == nullptr )
	{
		LOG_ERROR(L"Failed to obtain dxgidebug.dll module or DXGIGetDebugInterface function pointer");
		return false;
	}

	HRESULT hr;

	CComPtr<IDXGIDebug> pDXGIDebug;
	hr = DXGIGetDebugInterface(__uuidof(IDXGIDebug), (void**) &pDXGIDebug); CHECK_HR(hr);

	pDXGIDebug->ReportLiveObjects(DXGI_DEBUG_ALL, DXGI_DEBUG_RLO_ALL);
	OutputDebugStringW(L"\n");

	#endif

	return true;
}