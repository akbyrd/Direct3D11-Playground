#include "stdafx.h"

#include "Renderer.h"
#include "ExitCode.h"
#include "Utility.h"
#include "Color.h"

using namespace std;
using namespace Utility;
using namespace DirectX;

//TODO: Refactor error handling

bool Renderer::OnInitialize()
{
	long ret;

	XMMATRIX I = XMMatrixIdentity();
	XMStoreFloat4x4(&mWorld, I);
	XMStoreFloat4x4(&mView , I);
	XMStoreFloat4x4(&mProj , I);

	ret = InitializeInputLayout(); CHECK_RET(ret);
	ret = InitializeBuffers();     CHECK_RET(ret);

	//ret = SetWireframeMode(true);  CHECK_RET(ret);
	ret = OnResize(); CHECK_RET(ret);

	ret = ExitCode::Success;

Cleanup:

	return ret;
}

long Renderer::InitializeInputLayout()
{
	HRESULT hr;

	const D3D11_INPUT_ELEMENT_DESC arrVertShaderInputDescs[] = {
		{ "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT   , 0,  0, D3D11_INPUT_PER_VERTEX_DATA, 0 },
		{ "COLOR"   , 0, DXGI_FORMAT_R32G32B32A32_FLOAT, 0, 12, D3D11_INPUT_PER_VERTEX_DATA, 0 }
	};

	//Load vertex shader bytecode
	char*  vertShaderBytes = nullptr;
	SIZE_T vertShaderBytesLength;
	hr = LoadFile(L"Basic Vertex Shader.cso", vertShaderBytes, vertShaderBytesLength); CHECK_RET(hr);

	//Create and set input layout
	hr = pD3DDevice->CreateInputLayout(arrVertShaderInputDescs, 2, vertShaderBytes, vertShaderBytesLength, &pInputLayout); CHECK_HR(hr);
	SetDebugObjectName(pInputLayout, "Input Layout");

	pD3DImmediateContext->IASetInputLayout(pInputLayout);
	pD3DImmediateContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

	//Create and set vertex shader
	ID3D11VertexShader* pVertShader = nullptr;
	hr = pD3DDevice->CreateVertexShader(vertShaderBytes, vertShaderBytesLength, nullptr, &pVertShader); CHECK_HR(hr);
	SetDebugObjectName(pVertShader, "Basic Vertex Shader");

	pD3DImmediateContext->VSSetShader(pVertShader, nullptr, 0);

	//Load pixel shader bytecode
	char*  pixelShaderBytes = nullptr;
	SIZE_T pixelShaderBytesLength;
	hr = LoadFile(L"Basic Pixel Shader.cso", pixelShaderBytes, pixelShaderBytesLength); CHECK_RET(hr);

	//Create pixel shader
	ID3D11PixelShader* pPixelShader = nullptr;
	hr = pD3DDevice->CreatePixelShader(pixelShaderBytes, pixelShaderBytesLength, nullptr, &pPixelShader); CHECK_HR(hr);
	SetDebugObjectName(pPixelShader, "Basic Pixel Shader");

	pD3DImmediateContext->PSSetShader(pPixelShader, nullptr, 0);

	hr = ExitCode::Success;

Cleanup:
	SafeDelete(vertShaderBytes);
	SafeRelease(pVertShader);
	SafeDelete(pixelShaderBytes);
	SafeRelease(pPixelShader);

	return hr;
}

long Renderer::InitializeBuffers()
{
	long hr;

	//TODO: Best practices for types
	//Create and set vertex buffer
	const Vertex cubeVerts[] = {
		{ XMFLOAT3(-1.0f, -1.0f, -1.0f), (XMFLOAT4) Color::Red     },
		{ XMFLOAT3(-1.0f,  1.0f, -1.0f), (XMFLOAT4) Color::Green   },
		{ XMFLOAT3( 1.0f,  1.0f, -1.0f), (XMFLOAT4) Color::Blue    },
		{ XMFLOAT3( 1.0f, -1.0f, -1.0f), (XMFLOAT4) Color::Cyan    },
		{ XMFLOAT3(-1.0f, -1.0f,  1.0f), (XMFLOAT4) Color::Magenta },
		{ XMFLOAT3(-1.0f,  1.0f,  1.0f), (XMFLOAT4) Color::Yellow  },
		{ XMFLOAT3( 1.0f,  1.0f,  1.0f), (XMFLOAT4) Color::Black   },
		{ XMFLOAT3( 1.0f, -1.0f,  1.0f), (XMFLOAT4) Color::White   }
	};

	D3D11_BUFFER_DESC vertBufDesc;
	vertBufDesc.ByteWidth           = sizeof(Vertex) * 8;
	vertBufDesc.Usage               = D3D11_USAGE_IMMUTABLE;
	vertBufDesc.BindFlags           = D3D11_BIND_VERTEX_BUFFER;
	vertBufDesc.CPUAccessFlags      = 0;
	vertBufDesc.MiscFlags           = 0;
	vertBufDesc.StructureByteStride = 0;

	D3D11_SUBRESOURCE_DATA vertBufInitData;
	vertBufInitData.pSysMem          = cubeVerts;
	vertBufInitData.SysMemPitch      = 0;
	vertBufInitData.SysMemSlicePitch = 0;

	ID3D11Buffer* pVertBuffer = nullptr;
	hr = pD3DDevice->CreateBuffer(&vertBufDesc, &vertBufInitData, &pVertBuffer); CHECK_HR(hr);
	SetDebugObjectName(pVertBuffer, "Cube Vertex Buffer");

	const UINT stride = sizeof(Vertex);
	const UINT offset = 0;
	pD3DImmediateContext->IASetVertexBuffers(0, 1, &pVertBuffer, &stride, &offset);


	//Create and set index buffer
	const USHORT cubeIndices[] = {
		0, 1, 2, //Front
		0, 2, 3,
		3, 2, 6, //Right
		3, 6, 7,
		7, 6, 5, //Back
		7, 5, 4,
		1, 5, 6, //Top
		1, 6, 2,
		4, 5, 1, //Left
		4, 1, 0,
		4, 0, 3, //Bottom
		4, 3, 7
	};

	D3D11_BUFFER_DESC indexBufDesc;
	indexBufDesc.ByteWidth           = sizeof(UINT)*3 * 2*6;
	indexBufDesc.Usage               = D3D11_USAGE_IMMUTABLE;
	indexBufDesc.BindFlags           = D3D11_BIND_INDEX_BUFFER;
	indexBufDesc.CPUAccessFlags      = 0;
	indexBufDesc.MiscFlags           = 0;
	indexBufDesc.StructureByteStride = 0;

	D3D11_SUBRESOURCE_DATA indexBufInitData;
	indexBufInitData.pSysMem          = cubeIndices;
	indexBufInitData.SysMemPitch      = 0;
	indexBufInitData.SysMemSlicePitch = 0;

	ID3D11Buffer* pIndexBuffer = nullptr;
	hr = pD3DDevice->CreateBuffer(&indexBufDesc, &indexBufInitData, &pIndexBuffer); CHECK_HR(hr);
	SetDebugObjectName(pIndexBuffer, "Cube Index Buffer");

	pD3DImmediateContext->IASetIndexBuffer(pIndexBuffer, DXGI_FORMAT_R16_UINT, 0);


	//Create and set vertex shader constant buffer
	D3D11_BUFFER_DESC vsConstBufDesc;
	vsConstBufDesc.ByteWidth           = sizeof(float) * 16;
	vsConstBufDesc.Usage               = D3D11_USAGE_DEFAULT;
	vsConstBufDesc.BindFlags           = D3D11_BIND_CONSTANT_BUFFER;
	vsConstBufDesc.CPUAccessFlags      = 0;
	vsConstBufDesc.MiscFlags           = 0;
	vsConstBufDesc.StructureByteStride = 0;

	//Junk, it'll get updated before rendering
	D3D11_SUBRESOURCE_DATA vsConstBufInitData;
	vsConstBufInitData.pSysMem          = &mWorld;
	vsConstBufInitData.SysMemPitch      = 0;
	vsConstBufInitData.SysMemSlicePitch = 0;

	hr = pD3DDevice->CreateBuffer(&vsConstBufDesc, &vsConstBufInitData, &pVSConstBuffer); CHECK_HR(hr);
	SetDebugObjectName(pVSConstBuffer, "Vertex Shader WVP Constant Buffer");

	pD3DImmediateContext->VSSetConstantBuffers(0, 1, &pVSConstBuffer);

	hr = ExitCode::Success;

Cleanup:
	SafeRelease(pVertBuffer);
	SafeRelease(pIndexBuffer);

	return hr;
}

long Renderer::SetWireframeMode(bool enableWireframe)
{
	HRESULT hr;

	ID3D11RasterizerState *pRasterizerState = nullptr;
	pD3DImmediateContext->RSGetState(&pRasterizerState);

	D3D11_RASTERIZER_DESC rasterizerDesc;
	if ( pRasterizerState != nullptr )
	{
		pRasterizerState->GetDesc(&rasterizerDesc);
		SafeRelease(pRasterizerState);
	}
	else
	{
		rasterizerDesc.CullMode              = D3D11_CULL_BACK;
		rasterizerDesc.FrontCounterClockwise = false;
		rasterizerDesc.DepthBias             = 0;
		rasterizerDesc.DepthBiasClamp        = 0;
		rasterizerDesc.SlopeScaledDepthBias  = 0;
		rasterizerDesc.DepthClipEnable       = true;
		rasterizerDesc.ScissorEnable         = false;
		rasterizerDesc.MultisampleEnable     = false;
		rasterizerDesc.AntialiasedLineEnable = multiSampleCount > 0;
	}

	if ( enableWireframe )
		rasterizerDesc.FillMode = D3D11_FILL_WIREFRAME;
	else
		rasterizerDesc.FillMode = D3D11_FILL_SOLID;

	hr = pD3DDevice->CreateRasterizerState(&rasterizerDesc, &pRasterizerState); CHECK_HR(hr);
	pD3DImmediateContext->RSSetState(pRasterizerState);

	hr = ExitCode::Success;

Cleanup:
	SafeRelease(pRasterizerState);

	return hr;
}

bool Renderer::OnResize()
{
	long ret;

	///////////////////////////////////////////////////////////////////////////////////////////////
	// The window resized, so update the aspect ratio and recompute the projection matrix.
	XMMATRIX P = XMMatrixPerspectiveFovLH(XM_PIDIV4, (float) width / height, 0.1f, 100.0f);
	XMStoreFloat4x4(&mProj, P);
	///////////////////////////////////////////////////////////////////////////////////////////////

	ret = ExitCode::Success;

	return ret;
}

bool Renderer::Render()
{
	HRESULT hr;

	const XMVECTORF32 color = { 0.5f, 0.5f, 0.5f, 1.0f };

	pD3DImmediateContext->ClearRenderTargetView(pRenderTargetView, color);
	pD3DImmediateContext->ClearDepthStencilView(pDepthBufferView, D3D11_CLEAR_DEPTH | D3D11_CLEAR_STENCIL, 1, 0);

	pD3DImmediateContext->DrawIndexed(36, 0, 0);

	hr = pSwapChain->Present(0, 0); CHECK_HR(hr);

	hr = ExitCode::Success;

Cleanup:

	return hr;
}

bool Renderer::Update(const GameTimer &gameTimer)
{
	HRESULT hr;

	// Convert Spherical to Cartesian coordinates.
	float x = radius*sinf(phi)*cosf(theta);
	float z = radius*sinf(phi)*sinf(theta);
	float y = radius*cosf(phi);

	///////////////////////////////////////////////////////////////////////////////////////////////
	// Build the view matrix.
	XMVECTOR pos    = XMVectorSet(x, y, z, 1.0f);
	XMVECTOR target = XMVectorZero();
	XMVECTOR up     = XMVectorSet(0.0f, 1.0f, 0.0f, 0.0f);

	XMMATRIX V = XMMatrixLookAtLH(pos, target, up);
	XMStoreFloat4x4(&mView, V);

	XMMATRIX world = XMLoadFloat4x4(&mWorld);
	XMMATRIX view  = XMLoadFloat4x4(&mView);
	XMMATRIX proj  = XMLoadFloat4x4(&mProj);
	XMMATRIX worldViewProj = XMMatrixTranspose(world * view * proj);
	///////////////////////////////////////////////////////////////////////////////////////////////

	//Update the vertex shaders WVP constant buffer
	pD3DImmediateContext->UpdateSubresource(pVSConstBuffer, 0, nullptr, &worldViewProj, 0, 0);

	UpdateFrameStatistics(gameTimer);

	hr = ExitCode::Success;

Cleanup:

	return hr;
}

void Renderer::HandleInput(bool leftMouseDown, bool rightMouseDown, POINTS mousePosition)
{
	static float epsilon = numeric_limits<float>::epsilon();

	if ( leftMouseDown )
	{
		short dx = lastMousePosition.x - mousePosition.x;
		short dy = lastMousePosition.y - mousePosition.y;

		theta += .006f * dx;
		  phi += .006f * dy;

		if ( phi <     0 ) phi = epsilon;
		if ( phi > XM_PI ) phi = XM_PI - epsilon;
		SetCapture(hwnd);
	}
	else if ( rightMouseDown )
	{
		short dx = lastMousePosition.x - mousePosition.x;
		short dy = lastMousePosition.y - mousePosition.y;

		radius += .01f * (dx - dy);

		if ( radius <  0 ) radius = epsilon;
		if ( radius > 50 ) radius = 50;

		SetCapture(hwnd);
	}
	else
	{
		ReleaseCapture();
	}

	lastMousePosition = mousePosition;
}

void Renderer::OnTeardown()
{
	SafeRelease(pVSConstBuffer);
	SafeRelease(pInputLayout);
}