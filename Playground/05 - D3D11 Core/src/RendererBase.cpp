#include "stdafx.h"

//TODO: This needs to be tested on WinXP, 7, 8, and 8.1
#ifdef _DEBUG
	#define DEBUG_11_2

	#if defined(DEBUG_11)
		#include <d3d11sdklayers.h>
	#elif defined(DEBUG_11_1)
		#include <d3d11sdklayers.h>
		#include <dxgidebug.h>
	#elif defined(DEBUG_11_1_Plus)
		#include <d3d11sdklayers.h>
		#include <dxgidebug.h>
	#elif defined(DEBUG_11_2)
		#include <d3d11sdklayers.h>
		#include <dxgidebug.h>
		#include <dxgi1_3.h>
	#endif
#endif

#pragma comment(lib, "D3D11.lib")
#pragma comment(lib,  "DXGI.lib")

#include <iomanip>
#include <memory>
#include <sstream>

#include "RendererBase.h"
#include "LoggedException.h"
#include "AssertionException.h"
#include "Utility.h"

using namespace std;
using namespace Utility;
using namespace DirectX;

//TODO: Errors that prevent this renderer from doing its job should throw instead of returning false?

RendererBase::RendererBase() { }

bool RendererBase::Initialize(HWND hwnd)
{
	bool ret;

	isInitialized = true;

	SetHwnd(hwnd);

	ret = InitializeDevice();
	if(!ret) {return false;}

	ret = InitializeSwapChain();
	if(!ret) {return false;}

	ret = InitializeDepthBuffer();
	if(!ret) {return false;}

	InitializeOutputMerger();
	InitializeViewport();

	return true;
}

void RendererBase::SetHwnd(HWND hwnd)
{
	if ( !hwnd )
		throw_logged(L"Null HWND was provided.");

	RendererBase::hwnd = hwnd;
}

bool RendererBase::InitializeDevice()
{
	HRESULT hr;
	bool ret;

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
	);
	if(LOG_FAILED(hr)) {return false;}
	SetDebugObjectName(pD3DDevice, "Device");
	SetDebugObjectName(pD3DImmediateContext, "Device Context");

	//Check feature level
	if ( (featureLevel & D3D_FEATURE_LEVEL_11_0) != D3D_FEATURE_LEVEL_11_0 )
		throw_logged(L"Created device does not support D3D 11");

	//It's ok if these fail, they're optional debugging stuff
	InitializeDebugOptions();
	CheckForWarpDriver();

	ret = ObtainDXGIFactory();
	if(!ret) {return false;}

	return true;
}

bool RendererBase::InitializeDebugOptions()
{
	HRESULT hr;

	//TODO: Debug with the highest available interface
	//WinXP
	#if defined(DEBUG_11)
	throw_assert(pD3DDevice, L"D3D device not initialized.");

	ComPtr<ID3D11Debug> pD3DDebug;
	hr = pD3DDevice->QueryInterface(IID_PPV_ARGS(&pD3DDebug));
	if(LOG_FAILED(hr)) {return false;}

	ComPtr<ID3D11InfoQueue> pD3DInfoQueue;
	hr = pD3DDebug->QueryInterface(IID_PPV_ARGS(&pD3DInfoQueue));
	if(LOG_FAILED(hr)) {return false;}

	hr = pD3DInfoQueue->SetBreakOnSeverity(D3D11_MESSAGE_SEVERITY_CORRUPTION, true);
	if(LOG_FAILED(hr)) {return false;}
	hr = pD3DInfoQueue->SetBreakOnSeverity(D3D11_MESSAGE_SEVERITY_ERROR     , true);
	if(LOG_FAILED(hr)) {return false;}

	//Win7
	#elif defined(DEBUG_11_1) || defined(DEBUG_11_Plus)

	//Win8
	#if defined(DEBUG_11_1_Plus)
	typedef decltype(&DXGIGetDebugInterface) fPtr;
	#else
	typedef HRESULT (WINAPI *fPtr)(REFIID, void**);
	#endif

	HMODULE dxgiDebugModule = LoadLibraryExW(L"dxgidebug.dll", nullptr, LOAD_LIBRARY_SEARCH_SYSTEM32);
	if ( !dxgiDebugModule )
	{
		LOG_ERROR(L"Failed to load dxgidebug.dll");
		return false;
	}

	fPtr DXGIGetDebugInterface = (fPtr) GetProcAddress(dxgiDebugModule, "DXGIGetDebugInterface");
	if ( !DXGIGetDebugInterface )
	{
		LOG_ERROR(L"Failed to obtain DXGIGetDebugInterface function pointer");
		return false;
	}

	ComPtr<IDXGIInfoQueue> dxgiInfoQueue;
	hr = DXGIGetDebugInterface(IID_PPV_ARGS(&dxgiInfoQueue));
	if(LOG_FAILED(hr)) {return false;}

	hr = dxgiInfoQueue->SetBreakOnSeverity(DXGI_DEBUG_ALL, DXGI_INFO_QUEUE_MESSAGE_SEVERITY_ERROR     , true);
	if(LOG_FAILED(hr)) {return false;}
	hr = dxgiInfoQueue->SetBreakOnSeverity(DXGI_DEBUG_ALL, DXGI_INFO_QUEUE_MESSAGE_SEVERITY_CORRUPTION, true);
	if(LOG_FAILED(hr)) {return false;}

	//TODO: Smart pointer
	FreeLibrary(dxgiDebugModule);

	//Win8.1
	#elif defined(DEBUG_11_2)
	ComPtr<IDXGIDebug1> pDXGIDebug;
	hr = DXGIGetDebugInterface1(0, IID_PPV_ARGS(&pDXGIDebug));
	if(LOG_FAILED(hr)) {return false;}

	pDXGIDebug->EnableLeakTrackingForThread();

	ComPtr<IDXGIInfoQueue> pDXGIInfoQueue;
	hr = DXGIGetDebugInterface1(0, IID_PPV_ARGS(&pDXGIInfoQueue));
	if(LOG_FAILED(hr)) {return false;}

	hr = pDXGIInfoQueue->SetBreakOnSeverity(DXGI_DEBUG_ALL, DXGI_INFO_QUEUE_MESSAGE_SEVERITY_ERROR,      true); LOG_FAILED(hr);
	hr = pDXGIInfoQueue->SetBreakOnSeverity(DXGI_DEBUG_ALL, DXGI_INFO_QUEUE_MESSAGE_SEVERITY_CORRUPTION, true); LOG_FAILED(hr);

	#else
	UNREFERENCED_PARAMETER(hr);
	#endif

	return true;
}

bool RendererBase::ObtainDXGIFactory()
{
	throw_assert(pD3DDevice, L"D3D device not initialized.");

	HRESULT hr;

	/* Obtain the DXGI factory used to create the current device.
	 * 
	 * NOTE: It looks like the IDXGIDevice is actually the same object as the ID3D11Device.
	 * Using SetPrivateData to set its name clobbers the D3D device name and outputs a warning.
	 */
	ComPtr<IDXGIDevice1> pDXGIDevice;
	hr = pD3DDevice.As(&pDXGIDevice);
	if(LOG_FAILED(hr)) {return false;}

	ComPtr<IDXGIAdapter1> pDXGIAdapter;
	hr = pDXGIDevice->GetParent(IID_PPV_ARGS(&pDXGIAdapter));
	if(LOG_FAILED(hr)) {return false;}
	SetDebugObjectName(pDXGIAdapter, "DXGI Adapter");

	hr = pDXGIAdapter->GetParent(IID_PPV_ARGS(&pDXGIFactory));
	if(LOG_FAILED(hr)) {return false;}
	SetDebugObjectName(pDXGIFactory, "DXGI Factory");

	return true;
}

bool RendererBase::CheckForWarpDriver()
{
	throw_assert(pD3DDevice, L"D3D device not initialized.");

	HRESULT hr;

	//Check for the WARP driver
	ComPtr<IDXGIDevice1> pDXGIDevice;
	hr = pD3DDevice.As(&pDXGIDevice);
	if(LOG_FAILED(hr)) {return false;}

	ComPtr<IDXGIAdapter> pDXGIAdapter;
	hr = pDXGIDevice->GetAdapter(&pDXGIAdapter);
	if(LOG_FAILED(hr)) {return false;}

	DXGI_ADAPTER_DESC desc = {};
	hr = pDXGIAdapter->GetDesc(&desc);
	if(LOG_FAILED(hr)) {return false;}

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
	bool ret;

	//Query and set MSAA quality levels
	hr = pD3DDevice->CheckMultisampleQualityLevels(DXGI_FORMAT_R8G8B8A8_UNORM, multiSampleCount, &numQualityLevels);
	if(LOG_FAILED(hr)) {return false;}

	//Get the actual window size, in case we want it later.
	ret = GetWindowClientSize(width, height);
	if(!ret) {return false;}

	/* TODO: When using fullscreen, the display mode should be chosen by enumerating supported
	 * modes. If a mode is chosen that isn't supported, a performance penalty will be incurred due
	 * to Present performing a blit instead of a swap (does this apply to incorrect refresh rates
	 * or only incorrect reolutions?).
	 */

	/* Set swap chain properties
	 * 
	 * NOTE:
	 * If the DXGI_SWAP_CHAIN_FLAG_ALLOW_MODE_SWITCH flag is used, the display mode that most
	 * closely matches the back buffer will be used when entering fullscreen. If this happens to be
	 * the same size as the back buffer, no WM_SIZE event is sent to the application (I'm only
	 * assuming it *does* get sent if the size changes, I haven't tested it). If the flag is not
	 * used, the display mode will be changed to match that of the desktop (usually the monitors
	 * native display mode). This generally results in a WM_SIZE event (again, I'm only assuming
	 * one will not be sent if the window happens to already be the same size as the desktop). For
	 * now, I think it makes the most sense to use the native display mode when entering
	 * fullscreen, so I'm removing the flag.
	 */
	DXGI_SWAP_CHAIN_DESC swapChainDesc = {};
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
	swapChainDesc.Flags                              = 0;//DXGI_SWAP_CHAIN_FLAG_ALLOW_MODE_SWITCH;

	//Create the swap chain
	hr = pDXGIFactory->CreateSwapChain(pD3DDevice.Get(), &swapChainDesc, &pSwapChain);
	if(LOG_FAILED(hr)) {return false;}
	SetDebugObjectName(pSwapChain, "Swap Chain");

	if ( hr == DXGI_STATUS_OCCLUDED && !swapChainDesc.Windowed )
		LOG_WARNING(L"Failed to create a fullscreen swap chain. Falling back to windowed.");

	ret = CreateBackBufferView();
	if(!ret) {return false;}

	ret = UpdateAllowFullscreen();
	if(!ret) {return false;}

	return true;
}

bool RendererBase::GetWindowClientSize(UINT &width, UINT &height)
{
	HRESULT hr = -1;

	RECT rect = {};
	if ( GetClientRect(hwnd, &rect) )
	{
		width  = rect.right - rect.left;
		height = rect.bottom - rect.top;
	}
	else
	{
		hr = GetLastError();
		if ( !LOG_FAILED(hr) )
		{
			LOG_WARNING(L"GetClientRect failed, but the last error passed a FAILED. HR = " + to_wstring(hr));
		}
	}

	return true;
}

bool RendererBase::CreateBackBufferView()
{
	throw_assert(pD3DDevice, L"D3D device not initialized.");
	throw_assert(pSwapChain, L"The swap chain has not been initialized.");

	HRESULT hr;

	ComPtr<ID3D11Texture2D> pBackBuffer;
	hr = pSwapChain->GetBuffer(0, IID_PPV_ARGS(&pBackBuffer));
	if(LOG_FAILED(hr)) {return false;}
	SetDebugObjectName(pBackBuffer, "Back Buffer");

	//Create a render target view to the back buffer
	hr = pD3DDevice->CreateRenderTargetView(pBackBuffer.Get(), nullptr, &pRenderTargetView);
	if(LOG_FAILED(hr)) {return false;}
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

	hr = pDXGIFactory->MakeWindowAssociation(hwnd, flags);
	if(LOG_FAILED(hr)) {return false;}

	return true;
}

bool RendererBase::InitializeDepthBuffer()
{
	throw_assert(pD3DDevice, L"D3D device not initialized.");

	HRESULT hr;

	D3D11_TEXTURE2D_DESC depthDesc = {};
	depthDesc.Width              = width;
	depthDesc.Height             = height;
	depthDesc.MipLevels          = 1;
	depthDesc.ArraySize          = 1;
	depthDesc.Format             = DXGI_FORMAT_D24_UNORM_S8_UINT;
	depthDesc.SampleDesc.Count   = multiSampleCount;
	depthDesc.SampleDesc.Quality = numQualityLevels - 1;
	depthDesc.Usage              = D3D11_USAGE_DEFAULT;
	depthDesc.BindFlags          = D3D11_BIND_DEPTH_STENCIL;
	depthDesc.CPUAccessFlags     = 0;
	depthDesc.MiscFlags          = 0;

	ComPtr<ID3D11Texture2D> pDepthBuffer;
	hr = pD3DDevice->CreateTexture2D(&depthDesc, nullptr, &pDepthBuffer);
	if(LOG_FAILED(hr)) {return false;}
	SetDebugObjectName(pDepthBuffer, "Depth Buffer");

	hr = pD3DDevice->CreateDepthStencilView(pDepthBuffer.Get(), nullptr, &pDepthBufferView);
	if(LOG_FAILED(hr)) {return false;}
	SetDebugObjectName(pDepthBufferView, "Depth Buffer View");

	return true;
}

void RendererBase::InitializeOutputMerger()
{
	throw_assert(pD3DImmediateContext, L"D3D context not initialized.");

	pD3DImmediateContext->OMSetRenderTargets(1, pRenderTargetView.GetAddressOf(), pDepthBufferView.Get());
}

void RendererBase::InitializeViewport()
{
	throw_assert(pD3DImmediateContext, L"D3D context not initialized.");

	D3D11_VIEWPORT viewport = {};
	viewport.TopLeftX = 0;
	viewport.TopLeftY = 0;
	viewport.Width    = (FLOAT) width;
	viewport.Height   = (FLOAT) height;
	viewport.MinDepth = 0;
	viewport.MaxDepth = 1;

	pD3DImmediateContext->RSSetViewports(1, &viewport);
}


bool RendererBase::LogAdapters()
{
	HRESULT hr;

	ComPtr<IDXGIFactory1> pDXGIFactory;
	hr = CreateDXGIFactory1(IID_PPV_ARGS(&pDXGIFactory));
	if(LOG_FAILED(hr)) {return false;}

	UINT i = 0;
	while ( true )
	{
		ComPtr<IDXGIAdapter1> pDXGIAdapter;
		hr = pDXGIFactory->EnumAdapters1(i, &pDXGIAdapter);

		//We've run out of adapters
		if ( hr == DXGI_ERROR_NOT_FOUND ) { break; }

		//Other errors
		if(LOG_FAILED(hr)) {return false;}

		//Get adapter description
		DXGI_ADAPTER_DESC1 adapterDesc;
		hr = pDXGIAdapter->GetDesc1(&adapterDesc);
		if(LOG_FAILED(hr)) {return false;}

		static const float BytesInAMB = 1048576.0f;

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
		stream <<  L"DedicatedVideoMemory: " << adapterDesc.DedicatedVideoMemory  / BytesInAMB << L" MB" << endl;
		stream << L"DedicatedSystemMemory: " << adapterDesc.DedicatedSystemMemory / BytesInAMB << L" MB" << endl;
		stream <<    L"SharedSystemMemory: " << adapterDesc.SharedSystemMemory    / BytesInAMB << L" MB" << endl;
		stream <<                 L"Flags: " << adapterDesc.Flags                                        << endl;
		Logging::Log(stream);

		LogOutputs(pDXGIAdapter);

		++i;
	}

	return true;
}

bool RendererBase::LogOutputs(ComPtr<IDXGIAdapter1> pDXGIAdapter)
{
	throw_assert(pDXGIAdapter, L"pDXGIAdapter is null.");

	HRESULT hr;

	UINT i = 0;
	while ( true )
	{
		ComPtr<IDXGIOutput> pDXGIOutput;
		hr = pDXGIAdapter->EnumOutputs(i, &pDXGIOutput);

		//We've run out of outputs
		if ( hr == DXGI_ERROR_NOT_FOUND ) { break; }

		//Other errors
		if(LOG_FAILED(hr)) {return false;}

		DXGI_OUTPUT_DESC outputDesc = {};
		hr = pDXGIOutput->GetDesc(&outputDesc);
		if(LOG_FAILED(hr)) {return false;}

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

bool RendererBase::LogDisplayModes(ComPtr<IDXGIOutput> pDXGIOutput)
{
	HRESULT hr;

	DXGI_FORMAT format = DXGI_FORMAT_R8G8B8A8_UNORM;

	//Get the number of modes
	UINT numModes;
	hr = pDXGIOutput->GetDisplayModeList(format, 0, &numModes, nullptr);
	if(LOG_FAILED(hr)) {return false;}

	//Allocate space
	unique_ptr<DXGI_MODE_DESC[]> arrModeDesc(new DXGI_MODE_DESC[numModes]);

	//Get the actual display modes
	hr = pDXGIOutput->GetDisplayModeList(format, 0, &numModes, arrModeDesc.get());
	if(LOG_FAILED(hr)) {return false;}

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
	bool ret;

	//Get the new window size
	ret = GetWindowClientSize(width, height);
	if(!ret) {return false;}

	//Preserve the swap chain configuration
	DXGI_SWAP_CHAIN_DESC swapChainDesc = {};
	hr = pSwapChain->GetDesc(&swapChainDesc);
	if(LOG_FAILED(hr)) {return false;}

	//Skip resizing if it's not necessary
	if ( swapChainDesc.BufferDesc.Width  == width
	  && swapChainDesc.BufferDesc.Height == height )
	{
		return true;
	}

	//Release the old resource views (required to resize)
	pRenderTargetView.Reset();
	pDepthBufferView.Reset();

	hr = pSwapChain->ResizeBuffers(1, width, height, swapChainDesc.BufferDesc.Format, swapChainDesc.Flags);
	if(LOG_FAILED(hr)) {return false;}

	ret = CreateBackBufferView();
	if(!ret) {return false;}

	ret = InitializeDepthBuffer();
	if(!ret) {return false;}

	InitializeOutputMerger();
	InitializeViewport();

	ret = Resize();
	if(!ret) {return false;}

	return true;
}

bool RendererBase::Update(const GameTimer &gameTimer)
{
	throw_assert(pRenderTargetView, L"pRenderTargetView is not initialized");
	throw_assert(pDepthBufferView, L"pDepthBufferView is not initialized");
	throw_assert(pSwapChain, L"pSwapChain is not initialized");

	const float t = (float) gameTimer.Time();
	const float r = sinf(1.0f * t);
	const float g = sinf(2.0f * t);
	const float b = sinf(3.0f * t);

	backgroundColor = { r, g, b, 1.0f };

	UpdateFrameStatistics(gameTimer);

	return true;
}

bool RendererBase::Render()
{
	HRESULT hr;

	pD3DImmediateContext->ClearRenderTargetView(pRenderTargetView.Get(), (float*) &backgroundColor);
	pD3DImmediateContext->ClearDepthStencilView(pDepthBufferView.Get(), D3D11_CLEAR_DEPTH | D3D11_CLEAR_STENCIL, 1, 0);

	hr = pSwapChain->Present(0, 0);
	if(LOG_FAILED(hr)) {return false;}

	return true;
}


void RendererBase::UpdateFrameStatistics(const GameTimer &gameTimer)
{
	static const int bufferSize = 30;

	static char titleBuffer[128];
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

	//Update FPS in window title periodically
	static double lastFPSUpdateTime = numeric_limits<double>::epsilon();
	if ( gameTimer.RealTime() - lastFPSUpdateTime >= .5f )
	{
		lastFPSUpdateTime = gameTimer.RealTime();

		snprintf(titleBuffer, ArraySize(titleBuffer),
			"FPS: %.0f   Frame Time: %.2f ms (%d x %d)",
			1000 / averageFrameTime,
			averageFrameTime,
			width,
			height
		);

		SetWindowTextA(hwnd, titleBuffer);
	}

	return;
}

void RendererBase::Teardown()
{
	HRESULT hr;

	hwnd = nullptr;

	/* NOTE:
	 * A swap chain must not be released while fullscreened. That's just how the API works. It
	 * would also cause a reference count leak on the ID3D11Device. When entering fullscreen
	 * through Alt+Enter or swapChain->SetFullscreen(true, ...) the ref count is automatically
	 * incremented. It isn't decremented until leaving fullscreen. Thus closing the application
	 * while fullscreened leaks the device.
	 */
	if ( pSwapChain )
	{
		hr = pSwapChain->SetFullscreenState(false, nullptr);
		if ( FAILED(hr) )
		{
			LOG(L"Failed to disable fullscreen before releasing the swap chain. See the following message for more info.");
			LOG_FAILED(hr);
		}
	}

	//TODO: This doesn't need to be done, the ComPtrs will handle it.
	pRenderTargetView.Reset();
	pDepthBufferView.Reset();
	pSwapChain.Reset();
	pDXGIFactory.Reset();
	pD3DImmediateContext.Reset();
	pD3DDevice.Reset();

	isInitialized = false;

	//Check for leaks
	LogLiveObjects();
}

bool RendererBase::LogLiveObjects()
{
	HRESULT hr;

	//WinXP
	#if defined(DEBUG_11)
	/* TODO: This D3D API is poorly designed. You need to keep the device around to be able to get
	 * the debug interface, which means you're guaranteed to have live objects, defeating the
	 * purpose of even logging them. I could get the debug interface before releasing the device
	 * and then use it here, but that's ugly and requires a different code path. Since this is just
	 * debug code, I'd rather use the newest, most capable debug interface anyway, so this code
	 * path won't be used and is effectively unsupported.
	 */
	if ( !pD3DDevice )
	{
		LOG_WARNING(L"Failed to log live objects because the D3D device is not initialized.");
		return false;
	}

	ComPtr<ID3D11Debug> pD3DDebug;
	hr = pD3DDevice->QueryInterface(IID_PPV_ARGS(&pD3DDebug));
	if(LOG_FAILED(hr)) {return false;}

	//TODO: Test the differences in the output
	//D3D11_RLDO_SUMMARY
	//D3D11_RLDO_DETAIL
	//D3D11_RLDO_IGNORE_INTERNAL
	pD3DDebug->ReportLiveDeviceObjects(D3D11_RLDO_SUMMARY
	                                 | D3D11_RLDO_DETAIL
	                                 | D3D11_RLDO_IGNORE_INTERNAL
	);

	OutputDebugStringW(L"\n");

	//Win7
	#elif defined(DEBUG_11_1) || defined(DEBUG_11_1_Plus)

	//Win8
	#if defined(DEBUG_11_1_Plus)
	typedef decltype(&DXGIGetDebugInterface) fPtr;
	#else
	typedef HRESULT (WINAPI *fPtr)(REFIID, void**);
	#endif

	HMODULE dxgiDebugModule = LoadLibraryExW(L"dxgidebug.dll", nullptr, LOAD_LIBRARY_SEARCH_SYSTEM32);
	if ( !dxgiDebugModule )
	{
		LOG_ERROR(L"Failed to load dxgidebug.dll");
		return false;
	}

	fPtr DXGIGetDebugInterface = (fPtr) GetProcAddress(dxgiDebugModule, "DXGIGetDebugInterface");
	if ( !DXGIGetDebugInterface )
	{
		LOG_ERROR(L"Failed to obtain DXGIGetDebugInterface function pointer");
		return false;
	}

	ComPtr<IDXGIDebug> pDXGIDebug;
	hr = DXGIGetDebugInterface(IID_PPV_ARGS(&pDXGIDebug));
	if(LOG_FAILED(hr)) {return false;}

	hr = pDXGIDebug->ReportLiveObjects(DXGI_DEBUG_ALL, DXGI_DEBUG_RLO_IGNORE_INTERNAL);
	if(LOG_FAILED(hr)) {return false;}
	OutputDebugStringW(L"\n");

	//TODO: Smart pointer
	//unique_ptr<HMODULE> blah((HMODULE)nullptr, FreeLibrary);
	FreeLibrary(dxgiDebugModule);

	//Win8.1
	#elif defined(DEBUG_11_2)
	ComPtr<IDXGIDebug1> pDXGIDebug;
	hr = DXGIGetDebugInterface1(0, IID_PPV_ARGS(&pDXGIDebug));
	if(LOG_FAILED(hr)) {return false;}

	//TODO: Test the differences in the output
	//DXGI_DEBUG_RLO_ALL
	//DXGI_DEBUG_RLO_SUMMARY
	//DXGI_DEBUG_RLO_DETAIL
	hr = pDXGIDebug->ReportLiveObjects(DXGI_DEBUG_ALL, DXGI_DEBUG_RLO_IGNORE_INTERNAL);
	if(LOG_FAILED(hr)) {return false;}

	OutputDebugStringW(L"\n");

	#else
	UNREFERENCED_PARAMETER(hr);
	#endif

	return true;
}

RendererBase::~RendererBase()
{
	if ( isInitialized )
	{
		Teardown();
	}
}