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
#include <string>  //TODO: Eh...
#include "HLSL.h"
using namespace DirectX;
using namespace HLSL;

#pragma region Foward Declarations
bool InitializeSwapChain(struct RendererState*);
void UpdateRasterizeState(struct RendererState*);
#pragma endregion

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

struct DrawCall
{
	// NOTES:
	// Shaders and buffers can be shoved in a list in RendererState, references
	// here become raw pointers

	UINT                 vStride = 0;
	UINT                 vOffset = 0;
	ComPtr<ID3D11Buffer> vBuffer = nullptr;
	UINT                 iBase   = 0;
	UINT                 iCount  = 0;
	ComPtr<ID3D11Buffer> iBuffer = nullptr;
	DXGI_FORMAT          iFormat = {};
	//ID3D11VertexShader*  vShader = nullptr;
	//ID3D11PixelShader*   pShader = nullptr;
	XMFLOAT4X4           world   = {};
};

struct RendererState
{
	HWND                           hwnd                        = nullptr;
	ComPtr<ID3D11Device>           d3dDevice                   = nullptr;
	ComPtr<ID3D11DeviceContext>    d3dContext                  = nullptr;
	ComPtr<IDXGIFactory1>          dxgiFactory                 = nullptr;

	ComPtr<IDXGISwapChain>         dxgiSwapChain               = nullptr;
	ComPtr<ID3D11RenderTargetView> d3dRenderTargetView         = nullptr;
	ComPtr<ID3D11DepthStencilView> d3dDepthBufferView          = nullptr;

	ComPtr<ID3D11VertexShader>     d3dVertexShader             = nullptr;
	ComPtr<ID3D11Buffer>           d3dVSConstBuffer            = nullptr;
	ComPtr<ID3D11InputLayout>      d3dVSInputLayout            = nullptr;

	ComPtr<ID3D11PixelShader>      d3dPixelShader              = nullptr;
	ComPtr<ID3D11RasterizerState>  d3dRasterizerStateSolid     = nullptr;
	ComPtr<ID3D11RasterizerState>  d3dRasterizerStateWireframe = nullptr;

	XMFLOAT4X4 view               = {};
	XMFLOAT4X4 proj               = {};
	V2i        renderSize         = {};
	u32        multisampleCount   = 1;
	u32        qualityLevelCount  = 0;
	bool       allowFullscreen    = true;
	bool       isWireframeEnabled = true;
	DrawCall   drawCall           = {};
};

struct Vertex
{
	XMFLOAT3 position;
	XMFLOAT4 color;
};

bool
InitializeRenderer(RendererState* s)
{
	//TODO: Maybe asserts go with the usage site?
	Assert(s->hwnd                        != nullptr);
	Assert(s->d3dDevice                   == nullptr);
	Assert(s->d3dContext                  == nullptr);
	Assert(s->dxgiFactory                 == nullptr);
	Assert(s->dxgiSwapChain               == nullptr);
	Assert(s->d3dRenderTargetView         == nullptr);
	Assert(s->d3dDepthBufferView          == nullptr);
	Assert(s->d3dVertexShader             == nullptr);
	Assert(s->d3dVSConstBuffer            == nullptr);
	Assert(s->d3dVSInputLayout            == nullptr);
	Assert(s->d3dPixelShader              == nullptr);
	Assert(s->d3dRasterizerStateSolid     == nullptr);
	Assert(s->d3dRasterizerStateWireframe == nullptr);


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
				&s->d3dDevice,
				&featureLevel,
				&s->d3dContext
			),
			LOG_HRESULT, return false);
		SetDebugObjectName(s->d3dDevice,  "Device");
		SetDebugObjectName(s->d3dContext, "Device Context");

		//Check feature level
		if ((featureLevel & D3D_FEATURE_LEVEL_11_0) != D3D_FEATURE_LEVEL_11_0)
		{
			LOG_ERROR(L"Created device does not support D3D 11");
			return false;
		}

		//Obtain the DXGI factory used to create the current device.
		ComPtr<IDXGIDevice1> dxgiDevice;
		IF( s->d3dDevice.As(&dxgiDevice),
			LOG_HRESULT, return false);
		// NOTE: It looks like the IDXGIDevice is actually the same object as
		// the ID3D11Device. Using SetPrivateData to set its name clobbers the
		// D3D device name and outputs a warning.

		ComPtr<IDXGIAdapter> dxgiAdapter;
		IF( dxgiDevice->GetAdapter(&dxgiAdapter),
			LOG_HRESULT, return false);
		SetDebugObjectName(dxgiAdapter, "DXGI Adapter");

		IF( dxgiAdapter->GetParent(IID_PPV_ARGS(&s->dxgiFactory)),
			LOG_HRESULT, return false);
		SetDebugObjectName(s->dxgiFactory, "DXGI Factory");

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

		//Update 'allow fullscreen'
		UINT flags = 0;
		if (!s->allowFullscreen)
			flags |= DXGI_MWA_NO_ALT_ENTER;

		IF( s->dxgiFactory->MakeWindowAssociation(s->hwnd, flags),
			LOG_HRESULT, IGNORE);
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
		IF( s->d3dDevice->CheckMultisampleQualityLevels(
				DXGI_FORMAT_R8G8B8A8_UNORM,
				s->multisampleCount,
				&s->qualityLevelCount
			),
			LOG_HRESULT, return false);

		IF( GetWindowClientSize(s->hwnd, &s->renderSize),
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
		swapChainDesc.BufferDesc.Width                   = s->renderSize.x;
		swapChainDesc.BufferDesc.Height                  = s->renderSize.y;
		swapChainDesc.BufferDesc.RefreshRate.Numerator   = 60; //TODO: Un-hardcode (IDXGIOutput::FindClosestMatchingMode?)
		swapChainDesc.BufferDesc.RefreshRate.Denominator = 1;
		swapChainDesc.BufferDesc.Format                  = DXGI_FORMAT_R8G8B8A8_UNORM;
		swapChainDesc.BufferDesc.ScanlineOrdering        = DXGI_MODE_SCANLINE_ORDER_UNSPECIFIED;
		swapChainDesc.BufferDesc.Scaling                 = DXGI_MODE_SCALING_UNSPECIFIED;
		swapChainDesc.SampleDesc.Count                   = s->multisampleCount;
		swapChainDesc.SampleDesc.Quality                 = s->qualityLevelCount - 1;
		swapChainDesc.BufferUsage                        = DXGI_USAGE_RENDER_TARGET_OUTPUT;
		swapChainDesc.BufferCount                        = 1;
		swapChainDesc.OutputWindow                       = s->hwnd;
		swapChainDesc.Windowed                           = true;
		swapChainDesc.SwapEffect                         = DXGI_SWAP_EFFECT_DISCARD;
		swapChainDesc.Flags                              = 0;//DXGI_SWAP_CHAIN_FLAG_ALLOW_MODE_SWITCH;

		//Create swap chain
		HRESULT hr;
		IF( hr = s->dxgiFactory->CreateSwapChain(s->d3dDevice.Get(), &swapChainDesc, &s->dxgiSwapChain),
			LOG_HRESULT, return false);
		SetDebugObjectName(s->dxgiSwapChain, "Swap Chain");

		if (hr == DXGI_STATUS_OCCLUDED && !swapChainDesc.Windowed)
			LOG_WARNING(L"Failed to create a fullscreen swap chain. Falling back to windowed.");

		IF( InitializeSwapChain(s),
			IS_FALSE, return false);
	}


	//Create vertex shader
	{
		//Load
		unique_ptr<char[]> vsBytes;
		size_t vsBytesLength;
		IF( LoadFile(L"Basic Vertex Shader.cso", vsBytes, vsBytesLength),
			IS_FALSE, return false);

		//Create
		IF( s->d3dDevice->CreateVertexShader(vsBytes.get(), vsBytesLength, nullptr, &s->d3dVertexShader),
			LOG_HRESULT, return false);
		SetDebugObjectName(s->d3dVertexShader, "Vertex Shader");

		//Set
		s->d3dContext->VSSetShader(s->d3dVertexShader.Get(), nullptr, 0);

		//Per-object constant buffer
		D3D11_BUFFER_DESC vsConstBuffDes = {};
		vsConstBuffDes.ByteWidth           = sizeof(XMFLOAT4X4);
		vsConstBuffDes.Usage               = D3D11_USAGE_DYNAMIC;
		vsConstBuffDes.BindFlags           = D3D11_BIND_CONSTANT_BUFFER;
		vsConstBuffDes.CPUAccessFlags      = D3D11_CPU_ACCESS_WRITE;
		vsConstBuffDes.MiscFlags           = 0;
		vsConstBuffDes.StructureByteStride = 0;

		IF( s->d3dDevice->CreateBuffer(&vsConstBuffDes, nullptr, &s->d3dVSConstBuffer),
			LOG_HRESULT, return false);
		SetDebugObjectName(s->d3dVSConstBuffer, "VS Constant Buffer (Per-Object)");

		//Input layout
		D3D11_INPUT_ELEMENT_DESC vsInputDescs[] = {
			{ Semantic::Position, 0, DXGI_FORMAT_R32G32B32_FLOAT   , 0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0 },
			{ Semantic::Color   , 0, DXGI_FORMAT_R32G32B32A32_FLOAT, 0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0 }
		};

		IF( s->d3dDevice->CreateInputLayout(vsInputDescs, ArrayCount(vsInputDescs), vsBytes.get(), vsBytesLength, &s->d3dVSInputLayout),
			LOG_HRESULT, return false);
		SetDebugObjectName(s->d3dVSInputLayout, "Input Layout");

		s->d3dContext->IASetInputLayout(s->d3dVSInputLayout.Get());
		s->d3dContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
	}


	//Create pixel shader
	{
		//Load
		unique_ptr<char[]> psBytes;
		size_t psBytesLength;
		IF( LoadFile(L"Basic Pixel Shader.cso", psBytes, psBytesLength),
			IS_FALSE, return false);

		//Create
		IF( s->d3dDevice->CreatePixelShader(psBytes.get(), psBytesLength, nullptr, &s->d3dPixelShader),
			LOG_HRESULT, return false);
		SetDebugObjectName(s->d3dPixelShader, "Pixel Shader");

		//Set
		s->d3dContext->PSSetShader(s->d3dPixelShader.Get(), nullptr, 0);
	}


	//Initialize rasterizer state
	{
		// NOTE: MultisampleEnable toggles between quadrilateral AA (true) and alpha AA (false).
		// Alpha AA has a massive performance impact while quadrilateral is much smaller
		// (negligible for the mesh drawn here). Visually, it's hard to tell the difference between
		// quadrilateral AA on and off in this demo. Alpha AA on the other hand is more obvious. It
		// causes the wireframe to draw lines 2 px wide instead of 1.
		// 
		// See remarks: https://msdn.microsoft.com/en-us/library/windows/desktop/ff476198(v=vs.85).aspx
		const bool useQuadrilateralLineAA = true;

		//Solid
		D3D11_RASTERIZER_DESC rasterizerDesc = {};
		rasterizerDesc.FillMode              = D3D11_FILL_SOLID;
		rasterizerDesc.CullMode              = D3D11_CULL_NONE;
		rasterizerDesc.FrontCounterClockwise = false;
		rasterizerDesc.DepthBias             = 0;
		rasterizerDesc.DepthBiasClamp        = 0;
		rasterizerDesc.SlopeScaledDepthBias  = 0;
		rasterizerDesc.DepthClipEnable       = true;
		rasterizerDesc.ScissorEnable         = false;
		rasterizerDesc.MultisampleEnable     = s->multisampleCount > 1 && useQuadrilateralLineAA;
		rasterizerDesc.AntialiasedLineEnable = s->multisampleCount > 1;

		IF( s->d3dDevice->CreateRasterizerState(&rasterizerDesc, &s->d3dRasterizerStateSolid),
			LOG_HRESULT, return false);
		SetDebugObjectName(s->d3dRasterizerStateSolid, "Rasterizer State (Solid)");

		//Wireframe
		rasterizerDesc.FillMode = D3D11_FILL_WIREFRAME;

		IF( s->d3dDevice->CreateRasterizerState(&rasterizerDesc, &s->d3dRasterizerStateWireframe),
			LOG_HRESULT, return false);
		SetDebugObjectName(s->d3dRasterizerStateWireframe, "Rasterizer State (Wireframe)");

		//Start off in correct state
		UpdateRasterizeState(s);
	}


	//DEBUG: Create a draw call
	{
		//Create vertices
		Vertex vertices[4];

		vertices[0].position = XMFLOAT3(-1, -1, 0);
		vertices[1].position = XMFLOAT3(-1,  1, 0);
		vertices[2].position = XMFLOAT3( 1,  1, 0);
		vertices[3].position = XMFLOAT3( 1, -1, 0);

		XMStoreFloat4(&vertices[0].color, Colors::Red);
		XMStoreFloat4(&vertices[1].color, Colors::Green);
		XMStoreFloat4(&vertices[2].color, Colors::Blue);
		XMStoreFloat4(&vertices[3].color, Colors::White);

		//Create vertex buffer
		D3D11_BUFFER_DESC vertBuffDesc = {};
		vertBuffDesc.ByteWidth           = ArraySize(vertices);
		vertBuffDesc.Usage               = D3D11_USAGE_DYNAMIC;
		vertBuffDesc.BindFlags           = D3D11_BIND_VERTEX_BUFFER;
		vertBuffDesc.CPUAccessFlags      = D3D11_CPU_ACCESS_WRITE;
		vertBuffDesc.MiscFlags           = 0;
		vertBuffDesc.StructureByteStride = 0;

		D3D11_SUBRESOURCE_DATA vertBuffInitData = {};
		vertBuffInitData.pSysMem          = vertices;
		vertBuffInitData.SysMemPitch      = 0;
		vertBuffInitData.SysMemSlicePitch = 0;

		s->drawCall.vStride = sizeof(vertices[0]);
		s->drawCall.vOffset = 0;

		IF( s->d3dDevice->CreateBuffer(&vertBuffDesc, &vertBuffInitData, &s->drawCall.vBuffer),
			LOG_HRESULT, return false);
		SetDebugObjectName(s->drawCall.vBuffer, "Quad Vertices");

		//Create indices
		UINT indices[] = {
			0, 1, 2,
			2, 3, 0
		};

		//Create index buffer
		s->drawCall.iBase   = 0;
		s->drawCall.iCount  = ArrayCount(indices);
		s->drawCall.iFormat = DXGI_FORMAT_R32_UINT;

		D3D11_BUFFER_DESC indexBuffDesc = {};
		indexBuffDesc.ByteWidth           = ArraySize(indices);
		indexBuffDesc.Usage               = D3D11_USAGE_IMMUTABLE;
		indexBuffDesc.BindFlags           = D3D11_BIND_INDEX_BUFFER;
		indexBuffDesc.CPUAccessFlags      = 0;
		indexBuffDesc.MiscFlags           = 0;
		indexBuffDesc.StructureByteStride = 0;

		D3D11_SUBRESOURCE_DATA indexBuffInitData = {};
		indexBuffInitData.pSysMem          = indices;
		indexBuffInitData.SysMemPitch      = 0;
		indexBuffInitData.SysMemSlicePitch = 0;

		IF( s->d3dDevice->CreateBuffer(&indexBuffDesc, &indexBuffInitData, &s->drawCall.iBuffer),
			LOG_HRESULT, return false);
		SetDebugObjectName(s->drawCall.iBuffer, "Quad Indices");

		//World position
		XMStoreFloat4x4(&s->drawCall.world, XMMatrixIdentity());
	}

	return true;
}

bool
InitializeSwapChain(RendererState* s)
{
	Assert(s->hwnd                != nullptr);
	Assert(s->d3dDevice           != nullptr);
	Assert(s->d3dContext          != nullptr);
	Assert(s->dxgiFactory         != nullptr);
	Assert(s->dxgiSwapChain       != nullptr);
	Assert(s->d3dRenderTargetView == nullptr);
	Assert(s->d3dDepthBufferView  == nullptr);


	//Create back buffer
	{
		//Create a render target view to the back buffer
		ComPtr<ID3D11Texture2D> d3dBackBuffer;
		IF( s->dxgiSwapChain->GetBuffer(0, IID_PPV_ARGS(&d3dBackBuffer)),
			LOG_HRESULT, return false);
		SetDebugObjectName(d3dBackBuffer, "Back Buffer");

		IF( s->d3dDevice->CreateRenderTargetView(d3dBackBuffer.Get(), nullptr, &s->d3dRenderTargetView),
			LOG_HRESULT, return false);
		SetDebugObjectName(s->d3dRenderTargetView, "Render Target View");
	}


	//Create depth buffer
	{
		D3D11_TEXTURE2D_DESC depthDesc = {};
		depthDesc.Width              = s->renderSize.x;
		depthDesc.Height             = s->renderSize.y;
		depthDesc.MipLevels          = 1;
		depthDesc.ArraySize          = 1;
		depthDesc.Format             = DXGI_FORMAT_D24_UNORM_S8_UINT;
		depthDesc.SampleDesc.Count   = s->multisampleCount;
		depthDesc.SampleDesc.Quality = s->qualityLevelCount - 1;
		depthDesc.Usage              = D3D11_USAGE_DEFAULT;
		depthDesc.BindFlags          = D3D11_BIND_DEPTH_STENCIL;
		depthDesc.CPUAccessFlags     = 0;
		depthDesc.MiscFlags          = 0;

		ComPtr<ID3D11Texture2D> d3dDepthBuffer;
		IF( s->d3dDevice->CreateTexture2D(&depthDesc, nullptr, &d3dDepthBuffer),
			LOG_HRESULT, return false);
		SetDebugObjectName(d3dDepthBuffer, "Depth Buffer");

		IF( s->d3dDevice->CreateDepthStencilView(d3dDepthBuffer.Get(), nullptr, &s->d3dDepthBufferView),
			LOG_HRESULT, return false);
		SetDebugObjectName(s->d3dDepthBufferView, "Depth Buffer View");
	}


	//Initialize output merger
	s->d3dContext->OMSetRenderTargets(1, s->d3dRenderTargetView.GetAddressOf(), s->d3dDepthBufferView.Get());


	//Initialize viewport
	{
		D3D11_VIEWPORT viewport = {};
		viewport.TopLeftX = 0;
		viewport.TopLeftY = 0;
		viewport.Width    = (float) s->renderSize.x;
		viewport.Height   = (float) s->renderSize.y;
		viewport.MinDepth = 0;
		viewport.MaxDepth = 1;

		s->d3dContext->RSSetViewports(1, &viewport);
	}


	//Initialize projection matrix
	{
		float aspectRatio = (float) s->renderSize.x / (float) s->renderSize.y;
		XMMATRIX P = XMMatrixPerspectiveFovLH(XM_PIDIV4, aspectRatio, 0.1f, 100.0f);
		XMStoreFloat4x4(&s->proj, P);
	}

	return true;
}

void
UpdateRasterizeState(RendererState* s)
{
	Assert(s->d3dContext                  != nullptr);
	Assert(s->d3dRasterizerStateSolid     != nullptr);
	Assert(s->d3dRasterizerStateWireframe != nullptr);


	if ( s->isWireframeEnabled )
	{
		s->d3dContext->RSSetState(s->d3dRasterizerStateWireframe.Get());
	}
	else
	{
		s->d3dContext->RSSetState(s->d3dRasterizerStateSolid.Get());
	}
}

void
TeardownRenderer(RendererState* s)
{
	if (s == nullptr) {return;}

	s->hwnd = nullptr;

	if ( s->dxgiSwapChain )
	{
		// NOTE: A swap chain must not be released while fullscreened. That's just how the API
		// works. It would also cause a reference count leak on the ID3D11Device. When entering
		// fullscreen through Alt+Enter or swapChain->SetFullscreen(true, ...) the ref count is
		// automatically incremented. It isn't decremented until leaving fullscreen. Thus closing
		// the application while fullscreened leaks the device.

		IF( s->dxgiSwapChain->SetFullscreenState(false, nullptr),
			LOG_HRESULT, IGNORE);
	}

	s->d3dDevice                  .Reset();
	s->d3dContext                 .Reset();
	s->dxgiFactory                .Reset();
	s->dxgiSwapChain              .Reset();
	s->d3dRenderTargetView        .Reset();
	s->d3dDepthBufferView         .Reset();
	s->d3dVertexShader            .Reset();
	s->d3dVSConstBuffer           .Reset();
	s->d3dVSInputLayout           .Reset();
	s->d3dPixelShader             .Reset();
	s->d3dRasterizerStateSolid    .Reset();
	s->d3dRasterizerStateWireframe.Reset();


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
		ComPtr<IDXGIDebug1> dxgiDebug;
		IF( DXGIGetDebugInterface1(0, IID_PPV_ARGS(&dxgiDebug)),
			LOG_HRESULT, return);

		//TODO: Test the differences in the output
		//DXGI_DEBUG_RLO_ALL
		//DXGI_DEBUG_RLO_SUMMARY
		//DXGI_DEBUG_RLO_DETAIL
		IF( dxgiDebug->ReportLiveObjects(DXGI_DEBUG_ALL, DXGI_DEBUG_RLO_IGNORE_INTERNAL),
			LOG_HRESULT, return);

		OutputDebugStringW(L"\n");

		#else
		UNREFERENCED_PARAMETER(hr);
		#endif
	}
}


bool
ResizeRenderer(RendererState* s)
{
	Assert(s->dxgiSwapChain != nullptr);


	//Get the new window size
	IF( GetWindowClientSize(s->hwnd, &s->renderSize),
		IS_FALSE, return false);

	//Preserve the swap chain configuration
	DXGI_SWAP_CHAIN_DESC swapChainDesc = {};
	IF( s->dxgiSwapChain->GetDesc(&swapChainDesc),
		LOG_HRESULT, return false);

	//Skip resizing if it's not necessary
	if ( swapChainDesc.BufferDesc.Width  == s->renderSize.x
	  && swapChainDesc.BufferDesc.Height == s->renderSize.y )
	{
		LOG_WARNING(L"Attempting to resize swap chain, but it is already the correct size.");
		return true;
	}

	IF( swapChainDesc.BufferDesc.Width  == s->renderSize.x
	 && swapChainDesc.BufferDesc.Height == s->renderSize.y,
		IS_TRUE, LOG_WARNING(L"Attempting to resize swap chain, but it is already the correct size."); return true);

	//Release the old resource views (required to resize)
	s->d3dRenderTargetView.Reset();
	s->d3dDepthBufferView.Reset();

	IF( s->dxgiSwapChain->ResizeBuffers(
			1,
			s->renderSize.x, s->renderSize.y,
			swapChainDesc.BufferDesc.Format,
			swapChainDesc.Flags
		),
		LOG_HRESULT, return false);

	IF( InitializeSwapChain(s),
		IS_FALSE, return false);

	return true;
}

inline bool
Render(RendererState* s, r64 t)
{
	// TODO: Should we assert things that will immediately crash the program
	// anyway (e.g. dereferenced below). It probably gives us a better error
	// message.
	Assert(s->d3dContext    != nullptr);
	Assert(s->dxgiSwapChain != nullptr);


	s->d3dContext->ClearRenderTargetView(s->d3dRenderTargetView.Get(), DirectX::Colors::Magenta);
	s->d3dContext->ClearDepthStencilView(s->d3dDepthBufferView.Get(), D3D11_CLEAR_DEPTH | D3D11_CLEAR_STENCIL, 1, 0);

	//Update Camera
	float radius = 10;
	float phi    = XM_PIDIV4;
	float theta  = XM_PIDIV4 * 1.5f;

	float x = radius*sinf(phi)*cosf(theta);
	float z = radius*sinf(phi)*sinf(theta);
	float y = radius*cosf(phi);

	XMVECTOR pos    = XMVectorSet(x, y, z, 1);
	XMVECTOR target = XMVectorZero();
	XMVECTOR up     = XMVectorSet(0, 1, 0, 0);

	XMMATRIX V = XMMatrixLookAtLH(pos, target, up);
	XMStoreFloat4x4(&s->view, V);

	DrawCall* dc = &s->drawCall;
	XMMATRIX W = XMLoadFloat4x4(&dc->world);
	XMMATRIX P = XMLoadFloat4x4(&s->proj);
	XMMATRIX WVP = XMMatrixTranspose(W * V * P);

	//Update VS cbuffer
	D3D11_MAPPED_SUBRESOURCE vsConstBuffMap = {};
	IF( s->d3dContext->Map(s->d3dVSConstBuffer.Get(), 0, D3D11_MAP_WRITE_DISCARD, 0, &vsConstBuffMap),
		LOG_HRESULT, return false);

	memcpy(vsConstBuffMap.pData, &WVP, sizeof(WVP));
	s->d3dContext->Unmap(s->d3dVSConstBuffer.Get(), 0);

	s->d3dContext->IASetVertexBuffers(0, 1, dc->vBuffer.GetAddressOf(), &dc->vStride, &dc->vOffset);
	s->d3dContext->VSSetConstantBuffers(0, 1, s->d3dVSConstBuffer.GetAddressOf());
	s->d3dContext->IASetIndexBuffer(dc->iBuffer.Get(), DXGI_FORMAT_R32_UINT, 0);

	s->d3dContext->DrawIndexed(dc->iCount, 0, dc->vOffset);

	IF( s->dxgiSwapChain->Present(0, 0),
		LOG_HRESULT, return false);

	return true;
}