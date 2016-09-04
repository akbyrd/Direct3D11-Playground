#include <atlbase.h>
#include "Utility.h"
using namespace Utility;

//TODO: Eh...
#include <string>

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
#include <DirectXColors.h>

// REMEMBER:
// ComPtr
// operator&            - Get a ComPtrRef
// Get()                - Get the raw pointer
// GetAddressOf()       - Get a pointer to the raw pointer
//
// As()                 - Get some other interface
// Attach()             - Release existing pointer and assign a new one (does not AddRef)
// Detach()             - Get the raw pointer and leave the ComPtr empty (does not Release)
// Reset()              - Alias for Release
// Swap()               - Swap raw pointers (no calls to AddRef or Release)
//
// ComPtrRef
// operator(ComPtr<T>*) - Get raw pointer and leave ComPtr empty
// operator(void**)     - Alias for ReleaseAndGetAddressOf
// Operator(T**)        - Alias for ReleaseAndGetAddressOf

struct RendererState
{
	HWND                           hwnd                = nullptr;
	ComPtr<ID3D11Device>           d3dDevice           = nullptr;
	ComPtr<ID3D11DeviceContext>    d3dContext          = nullptr;
	ComPtr<IDXGIFactory1>          dxgiFactory         = nullptr;
	ComPtr<IDXGISwapChain>         dxgiSwapChain       = nullptr;
	ComPtr<ID3D11RenderTargetView> d3dRenderTargetView = nullptr;
	ComPtr<ID3D11DepthStencilView> d3dDepthBufferView  = nullptr;

	V2i  renderSize        = {};
	u32  multisampleCount  = 1;
	u32  qualityLevelCount = 0;
	bool allowFullscreen   = true;
};

bool InitializeSwapChain(RendererState*);

bool
InitializeRenderer(RendererState* state)
{
	Assert(state->hwnd        != nullptr);
	Assert(state->d3dDevice   == nullptr);
	Assert(state->d3dContext  == nullptr);
	Assert(state->dxgiFactory == nullptr);


	//Create device
	{
		UINT createDeviceFlags = D3D11_CREATE_DEVICE_SINGLETHREADED;
		#if _DEBUG
		createDeviceFlags |= D3D11_CREATE_DEVICE_DEBUG;
		//createDeviceFlags |= D3D11_CREATE_DEVICE_DEBUGGABLE; //11_1+
		#endif

		D3D_FEATURE_LEVEL featureLevel  = {};

		IF( D3D11CreateDevice(
				nullptr,
				D3D_DRIVER_TYPE_HARDWARE,
				nullptr,
				createDeviceFlags,
				nullptr, 0, //NOTE: 11_1 will never be created by default
				D3D11_SDK_VERSION,
				&state->d3dDevice,
				&featureLevel,
				&state->d3dContext
			),
			LOG_HRESULT, return false);
		SetDebugObjectName(state->d3dDevice,  "Device");
		SetDebugObjectName(state->d3dContext, "Device Context");

		//Check feature level
		if ((featureLevel & D3D_FEATURE_LEVEL_11_0) != D3D_FEATURE_LEVEL_11_0)
		{
			LOG_ERROR(L"Created device does not support D3D 11");
			return false;
		}

		//Obtain the DXGI factory used to create the current device.
		ComPtr<IDXGIDevice1> dxgiDevice;
		IF( state->d3dDevice.As(&dxgiDevice),
			LOG_HRESULT, return false);
		// NOTE: It looks like the IDXGIDevice is actually the same object as
		// the ID3D11Device. Using SetPrivateData to set its name clobbers the
		// D3D device name and outputs a warning.

		ComPtr<IDXGIAdapter> dxgiAdapter;
		IF( dxgiDevice->GetAdapter(&dxgiAdapter),
			LOG_HRESULT, return false);
		SetDebugObjectName(dxgiAdapter, "DXGI Adapter");

		IF( dxgiAdapter->GetParent(IID_PPV_ARGS(&state->dxgiFactory)),
			LOG_HRESULT, return false);
		SetDebugObjectName(state->dxgiFactory, "DXGI Factory");

		//Check for the WARP driver
		DXGI_ADAPTER_DESC desc = {};
		HRESULT hr = dxgiAdapter->GetDesc(&desc);
		if (!LOG_HRESULT(hr))
		{
			if ( (desc.VendorId == 0x1414) && (desc.DeviceId == 0x8c) )
			{
				// WARNING: Microsoft Basic Render Driver is active. Performance of this
				// application may be unsatisfactory. Please ensure that your video card is
				// Direct3D10/11 capable and has the appropriate driver installed.
				LOG_WARNING(L"WARP driver in use.");
			}
		}
	}


	//Configure debugging
	{
		//TODO: Debug with the highest available interface
		//WinXP
		#if defined(DEBUG_11)
		throw_assert(pD3DDevice, L"D3D device not initialized.");

		ComPtr<ID3D11Debug> pD3DDebug;
		hr = pD3DDevice->QueryInterface(IID_PPV_ARGS(&pD3DDebug));
		if(LOG_HRESULT(hr)) {return false;}

		ComPtr<ID3D11InfoQueue> pD3DInfoQueue;
		hr = pD3DDebug->QueryInterface(IID_PPV_ARGS(&pD3DInfoQueue));
		if(LOG_HRESULT(hr)) {return false;}

		hr = pD3DInfoQueue->SetBreakOnSeverity(D3D11_MESSAGE_SEVERITY_CORRUPTION, true);
		if(LOG_HRESULT(hr)) {return false;}
		hr = pD3DInfoQueue->SetBreakOnSeverity(D3D11_MESSAGE_SEVERITY_ERROR     , true);
		if(LOG_HRESULT(hr)) {return false;}

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
		if(LOG_HRESULT(hr)) {return false;}

		hr = dxgiInfoQueue->SetBreakOnSeverity(DXGI_DEBUG_ALL, DXGI_INFO_QUEUE_MESSAGE_SEVERITY_ERROR     , true);
		if(LOG_HRESULT(hr)) {return false;}
		hr = dxgiInfoQueue->SetBreakOnSeverity(DXGI_DEBUG_ALL, DXGI_INFO_QUEUE_MESSAGE_SEVERITY_CORRUPTION, true);
		if(LOG_HRESULT(hr)) {return false;}

		//TODO: Smart pointer
		FreeLibrary(dxgiDebugModule);

		//Win8.1
		#elif defined(DEBUG_11_2)
		ComPtr<IDXGIDebug1> pDXGIDebug;
		IF( DXGIGetDebugInterface1(0, IID_PPV_ARGS(&pDXGIDebug)),
			LOG_HRESULT, return false);

		pDXGIDebug->EnableLeakTrackingForThread();

		ComPtr<IDXGIInfoQueue> pDXGIInfoQueue;
		IF( DXGIGetDebugInterface1(0, IID_PPV_ARGS(&pDXGIInfoQueue)),
			LOG_HRESULT, return false);

		IF( pDXGIInfoQueue->SetBreakOnSeverity(DXGI_DEBUG_ALL, DXGI_INFO_QUEUE_MESSAGE_SEVERITY_ERROR,      true),
			LOG_HRESULT, IGNORE);
		IF( pDXGIInfoQueue->SetBreakOnSeverity(DXGI_DEBUG_ALL, DXGI_INFO_QUEUE_MESSAGE_SEVERITY_CORRUPTION, true),
			LOG_HRESULT, IGNORE);

		#else
		UNREFERENCED_PARAMETER(hr);
		#endif
	}


	//Create swap chain
	{
		//Query and set MSAA quality levels
		IF( state->d3dDevice->CheckMultisampleQualityLevels(
				DXGI_FORMAT_R8G8B8A8_UNORM,
				state->multisampleCount,
				&state->qualityLevelCount
			),
			LOG_HRESULT, return false);

		IF( GetWindowClientSize(state->hwnd, &state->renderSize),
			IS_FALSE, return false);

		// TODO: When using fullscreen, the display mode should be chosen by enumerating supported
		// modes. If a mode is chosen that isn't supported, a performance penalty will be incurred
		// due to Present performing a blit instead of a swap (does this apply to incorrect refresh
		// rates or only incorrect resolutions?).

		// Set swap chain properties
		// 
		// NOTE: If the DXGI_SWAP_CHAIN_FLAG_ALLOW_MODE_SWITCH flag is used, the display mode that
		// most closely matches the back buffer will be used when entering fullscreen. If this
		// happens to be the same size as the back buffer, no WM_SIZE event is sent to the
		// application (I'm only assuming it *does* get sent if the size changes, I haven't tested
		// it). If the flag is not used, the display mode will be changed to match that of the
		// desktop (usually the monitors native display mode). This generally results in a WM_SIZE
		// event (again, I'm only assuming one will not be sent if the window happens to already be
		// the same size as the desktop). For now, I think it makes the most sense to use the
		// native display mode when entering fullscreen, so I'm removing the flag.

		DXGI_SWAP_CHAIN_DESC swapChainDesc = {};
		swapChainDesc.BufferDesc.Width                   = state->renderSize.x;
		swapChainDesc.BufferDesc.Height                  = state->renderSize.y;
		swapChainDesc.BufferDesc.RefreshRate.Numerator   = 60; //TODO: Un-hardcode (IDXGIOutput::FindClosestMatchingMode?)
		swapChainDesc.BufferDesc.RefreshRate.Denominator = 1;
		swapChainDesc.BufferDesc.Format                  = DXGI_FORMAT_R8G8B8A8_UNORM;
		swapChainDesc.BufferDesc.ScanlineOrdering        = DXGI_MODE_SCANLINE_ORDER_UNSPECIFIED;
		swapChainDesc.BufferDesc.Scaling                 = DXGI_MODE_SCALING_UNSPECIFIED;
		swapChainDesc.SampleDesc.Count                   = state->multisampleCount;
		swapChainDesc.SampleDesc.Quality                 = state->qualityLevelCount - 1;
		swapChainDesc.BufferUsage                        = DXGI_USAGE_RENDER_TARGET_OUTPUT;
		swapChainDesc.BufferCount                        = 1;
		swapChainDesc.OutputWindow                       = state->hwnd;
		swapChainDesc.Windowed                           = true;
		swapChainDesc.SwapEffect                         = DXGI_SWAP_EFFECT_DISCARD;
		swapChainDesc.Flags                              = 0;//DXGI_SWAP_CHAIN_FLAG_ALLOW_MODE_SWITCH;

		//Create swap chain
		HRESULT hr;
		IF( hr = state->dxgiFactory->CreateSwapChain(state->d3dDevice.Get(), &swapChainDesc, &state->dxgiSwapChain),
			LOG_HRESULT, return false);
		SetDebugObjectName(state->dxgiSwapChain, "Swap Chain");

		if (hr == DXGI_STATUS_OCCLUDED && !swapChainDesc.Windowed)
			LOG_WARNING(L"Failed to create a fullscreen swap chain. Falling back to windowed.");

		IF( InitializeSwapChain(state),
			IS_FALSE, return false);
	}

	return true;
}

bool
InitializeSwapChain(RendererState* state)
{
	Assert(state->hwnd                != nullptr);
	Assert(state->d3dDevice           != nullptr);
	Assert(state->d3dContext          != nullptr);
	Assert(state->dxgiFactory         != nullptr);
	Assert(state->dxgiSwapChain       != nullptr);
	Assert(state->d3dRenderTargetView == nullptr);
	Assert(state->d3dDepthBufferView  == nullptr);


	//Create back buffer
	{
		//Create a render target view to the back buffer
		ComPtr<ID3D11Texture2D> d3dBackBuffer;
		IF( state->dxgiSwapChain->GetBuffer(0, IID_PPV_ARGS(&d3dBackBuffer)),
			LOG_HRESULT, return false);
		SetDebugObjectName(d3dBackBuffer, "Back Buffer");

		IF( state->d3dDevice->CreateRenderTargetView(d3dBackBuffer.Get(), nullptr, &state->d3dRenderTargetView),
			LOG_HRESULT, return false);
		SetDebugObjectName(state->d3dRenderTargetView, "Render Target View");

		//Update 'allow fullscreen'
		UINT flags = 0;
		if (!state->allowFullscreen)
			flags |= DXGI_MWA_NO_ALT_ENTER;

		IF( state->dxgiFactory->MakeWindowAssociation(state->hwnd, flags),
			LOG_HRESULT, IGNORE);
	}


	//Create depth buffer
	{
		D3D11_TEXTURE2D_DESC depthDesc = {};
		depthDesc.Width              = state->renderSize.x;
		depthDesc.Height             = state->renderSize.y;
		depthDesc.MipLevels          = 1;
		depthDesc.ArraySize          = 1;
		depthDesc.Format             = DXGI_FORMAT_D24_UNORM_S8_UINT;
		depthDesc.SampleDesc.Count   = state->multisampleCount;
		depthDesc.SampleDesc.Quality = state->qualityLevelCount - 1;
		depthDesc.Usage              = D3D11_USAGE_DEFAULT;
		depthDesc.BindFlags          = D3D11_BIND_DEPTH_STENCIL;
		depthDesc.CPUAccessFlags     = 0;
		depthDesc.MiscFlags          = 0;

		ComPtr<ID3D11Texture2D> d3dDepthBuffer;
		IF( state->d3dDevice->CreateTexture2D(&depthDesc, nullptr, &d3dDepthBuffer),
			LOG_HRESULT, return false);
		SetDebugObjectName(d3dDepthBuffer, "Depth Buffer");

		IF( state->d3dDevice->CreateDepthStencilView(d3dDepthBuffer.Get(), nullptr, &state->d3dDepthBufferView),
			LOG_HRESULT, return false);
		SetDebugObjectName(state->d3dDepthBufferView, "Depth Buffer View");
	}


	//Initialize output merger
	state->d3dContext->OMSetRenderTargets(1, state->d3dRenderTargetView.GetAddressOf(), state->d3dDepthBufferView.Get());


	//Initialize viewport
	{
		D3D11_VIEWPORT viewport = {};
		viewport.TopLeftX = 0;
		viewport.TopLeftY = 0;
		viewport.Width    = (float) state->renderSize.x;
		viewport.Height   = (float) state->renderSize.y;
		viewport.MinDepth = 0;
		viewport.MaxDepth = 1;

		state->d3dContext->RSSetViewports(1, &viewport);
	}
}

void
TeardownRenderer(RendererState* state)
{
	if (state == nullptr) {return;}

	HRESULT hr;

	state->hwnd = nullptr;

	if ( state->dxgiSwapChain )
	{
		// NOTE: A swap chain must not be released while fullscreened. That's just how the API
		// works. It would also cause a reference count leak on the ID3D11Device. When entering
		// fullscreen through Alt+Enter or swapChain->SetFullscreen(true, ...) the ref count is
		// automatically incremented. It isn't decremented until leaving fullscreen. Thus closing
		// the application while fullscreened leaks the device.

		IF( hr = state->dxgiSwapChain->SetFullscreenState(false, nullptr),
			LOG_HRESULT, IGNORE);
	}

	state->d3dDevice          .Reset();
	state->d3dContext         .Reset();
	state->dxgiFactory        .Reset();
	state->dxgiSwapChain      .Reset();
	state->d3dRenderTargetView.Reset();
	state->d3dDepthBufferView .Reset();


	//Log live objects
	{
		//WinXP
		#if defined(DEBUG_11)
		// TODO: This D3D API is poorly designed. You need to keep the device around to be able to
		// get the debug interface, which means you're guaranteed to have live objects, defeating
		// the purpose of even logging them. I could get the debug interface before releasing the
		// device and then use it here, but that's ugly and requires a different code path. Since
		// this is just debug code, I'd rather use the newest, most capable debug interface anyway,
		// so this code path won't be used and is effectively unsupported.
		if ( !pD3DDevice )
		{
			LOG_WARNING(L"Failed to log live objects because the D3D device is not initialized.");
			return;
		}

		ComPtr<ID3D11Debug> pD3DDebug;
		hr = pD3DDevice->QueryInterface(IID_PPV_ARGS(&pD3DDebug));
		if(LOG_HRESULT(hr)) {return;}

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
			return;
		}

		fPtr DXGIGetDebugInterface = (fPtr) GetProcAddress(dxgiDebugModule, "DXGIGetDebugInterface");
		if ( !DXGIGetDebugInterface )
		{
			LOG_ERROR(L"Failed to obtain DXGIGetDebugInterface function pointer");
			return;
		}

		ComPtr<IDXGIDebug> pDXGIDebug;
		hr = DXGIGetDebugInterface(IID_PPV_ARGS(&pDXGIDebug));
		if(LOG_HRESULT(hr)) {return;}

		hr = pDXGIDebug->ReportLiveObjects(DXGI_DEBUG_ALL, DXGI_DEBUG_RLO_IGNORE_INTERNAL);
		if(LOG_HRESULT(hr)) {return;}
		OutputDebugStringW(L"\n");

		//TODO: Smart pointer
		//unique_ptr<HMODULE> blah((HMODULE)nullptr, FreeLibrary);
		FreeLibrary(dxgiDebugModule);

		//Win8.1
		#elif defined(DEBUG_11_2)
		ComPtr<IDXGIDebug1> pDXGIDebug;
		IF( hr = DXGIGetDebugInterface1(0, IID_PPV_ARGS(&pDXGIDebug)),
			LOG_HRESULT, return);

		//TODO: Test the differences in the output
		//DXGI_DEBUG_RLO_ALL
		//DXGI_DEBUG_RLO_SUMMARY
		//DXGI_DEBUG_RLO_DETAIL
		IF( hr = pDXGIDebug->ReportLiveObjects(DXGI_DEBUG_ALL, DXGI_DEBUG_RLO_IGNORE_INTERNAL),
			LOG_HRESULT, return);

		OutputDebugStringW(L"\n");

		#else
		UNREFERENCED_PARAMETER(hr);
		#endif
	}
}


bool
ResizeRenderer(RendererState* state)
{
	Assert(state->dxgiSwapChain != nullptr);

	//Get the new window size
	IF( GetWindowClientSize(state->hwnd, &state->renderSize),
		IS_FALSE, return false);

	//Preserve the swap chain configuration
	DXGI_SWAP_CHAIN_DESC swapChainDesc = {};
	IF( state->dxgiSwapChain->GetDesc(&swapChainDesc),
		LOG_HRESULT, return false);

	//Skip resizing if it's not necessary
	if ( swapChainDesc.BufferDesc.Width  == state->renderSize.x
	  && swapChainDesc.BufferDesc.Height == state->renderSize.y )
	{
		return true;
	}

	//Release the old resource views (required to resize)
	state->d3dRenderTargetView.Reset();
	state->d3dDepthBufferView.Reset();

	IF( state->dxgiSwapChain->ResizeBuffers(
			1,
			state->renderSize.x, state->renderSize.y,
			swapChainDesc.BufferDesc.Format,
			swapChainDesc.Flags
		),
		LOG_HRESULT, return false);

	IF( InitializeSwapChain(state),
		IS_FALSE, return false);

	return true;
}

//TODO: Move this?
void
UpdateFrameStatistics(RendererState* state, r64 t)
{
	static const int bufferSize = 30;

	static char titleBuffer[128];
	static double buffer[bufferSize];
	static int head = -1;
	static int length = 0;
	static double deltaToMS;

	//HACK: Hate this
	if ( length == 0 )
		buffer[bufferSize - 1] = t;

	//Update the head position and length
	head = (head + 1) % bufferSize;
	if ( length < bufferSize - 1 )
	{
		++length;
		deltaToMS = 1000. / length;
	}

	//Update the head value
	buffer[head] = t;

	int tail = (head - length) % bufferSize;
	if ( tail < 0 )
		tail += bufferSize;

	//Update FPS in window title periodically
	static double lastFPSUpdateTime = 0;
	if ( t - lastFPSUpdateTime >= .5 )
	{
		lastFPSUpdateTime = t;

		r64 delta = buffer[head] - buffer[tail];
		r64 averageFrameTime = delta * deltaToMS;

		snprintf(titleBuffer, ArraySize(titleBuffer),
				 "FPS: %.0f   Frame Time: %.4f ms (%d x %d)",
				 1000 / averageFrameTime,
				 averageFrameTime,
				 state->renderSize.x,
				 state->renderSize.y
		);

		SetWindowTextA(state->hwnd, titleBuffer);
	}

	return;
}

inline bool
Render(RendererState* state, r64 t)
{
	HRESULT hr;

	state->d3dContext->ClearRenderTargetView(state->d3dRenderTargetView.Get(), DirectX::Colors::Magenta);
	state->d3dContext->ClearDepthStencilView(state->d3dDepthBufferView.Get(), D3D11_CLEAR_DEPTH | D3D11_CLEAR_STENCIL, 1, 0);

	hr = state->dxgiSwapChain->Present(0, 0);
	if(LOG_HRESULT(hr)) {return false;}

	UpdateFrameStatistics(state, t);

	return true;
}