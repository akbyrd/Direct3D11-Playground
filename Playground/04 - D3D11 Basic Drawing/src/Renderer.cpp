#include "stdafx.h"
#include "Renderer.h"
#include "ExitCode.h"
#include "Logging.h"
#include "Utility.h"
#include "Color.h"

using namespace std;
using namespace Utility;
using namespace DirectX;

long Renderer::OnInitialize()
{
	long ret;

	XMMATRIX I = XMMatrixIdentity();
	XMStoreFloat4x4(&mWorld, I);
	XMStoreFloat4x4(&mView , I);
	XMStoreFloat4x4(&mProj , I);

	ret = InitializeInputLayout(); CHECK_RET(ret);
	ret = InitializeBuffers();     CHECK_RET(ret);

	ret = ExitCode::Success;

Cleanup:

	return ret;
}

long Renderer::InitializeInputLayout()
{
	HRESULT hr;

	D3D11_INPUT_ELEMENT_DESC arrVertShaderInputDescs[] = {
		{ "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT   ,  0, 0, D3D11_INPUT_PER_VERTEX_DATA, 0 },
		{ "COLOR"   , 0, DXGI_FORMAT_R32G32B32A32_FLOAT, 12, 0, D3D11_INPUT_PER_VERTEX_DATA, 0 }
	};

	//Load vertex shader bytecode
	char*  vertShaderBytes = nullptr;
	SIZE_T vertShaderBytesLength;
	hr = LoadFile(L"Basic Vertex Shader.cso", vertShaderBytes, vertShaderBytesLength); CHECK_RET(hr);

	//Create and set input layout
	hr = pD3DDevice->CreateInputLayout(arrVertShaderInputDescs, 2, vertShaderBytes, vertShaderBytesLength, &pInputLayout); CHECK_HR(hr);

	pD3DImmediateContext->IASetInputLayout(pInputLayout);
	pD3DImmediateContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

	//Create and set vertex shader
	ID3D11VertexShader* pVertShader = nullptr;
	hr = pD3DDevice->CreateVertexShader(vertShaderBytes, vertShaderBytesLength, nullptr, &pVertShader); CHECK_HR(hr);
	pD3DImmediateContext->VSSetShader(pVertShader, nullptr, 0);

	//Load pixel shader bytecode
	char*  pixelShaderBytes = nullptr;
	SIZE_T pixelShaderBytesLength;
	hr = LoadFile(L"Basic Pixel Shader.cso", pixelShaderBytes, pixelShaderBytesLength); CHECK_RET(hr);

	//Create pixel shader
	ID3D11PixelShader* pPixelShader = nullptr;
	hr = pD3DDevice->CreatePixelShader(pixelShaderBytes, pixelShaderBytesLength, nullptr, &pPixelShader); CHECK_HR(hr);
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
	Vertex cubeVerts[] = {
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

	UINT stride = sizeof(Vertex);
	UINT offset = 0;
	pD3DImmediateContext->IASetVertexBuffers(0, 1, &pVertBuffer, &stride, &offset);


	//Create and set index buffer
	UINT cubeIndeces[] = {
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
	indexBufInitData.pSysMem          = &indexBufDesc;
	indexBufInitData.SysMemPitch      = 0;
	indexBufInitData.SysMemSlicePitch = 0;

	ID3D11Buffer* pIndexBuffer = nullptr;
	hr = pD3DDevice->CreateBuffer(&indexBufDesc, &indexBufInitData, &pIndexBuffer); CHECK_HR(hr);

	pD3DImmediateContext->IASetIndexBuffer(pIndexBuffer, DXGI_FORMAT_R32_UINT, 0);


	//Create and set vertex shader constant buffer
	///////////////////////////////////////////////////////////////////////////////////////////////
	// The window resized, so update the aspect ratio and recompute the projection matrix.
	XMMATRIX P = XMMatrixPerspectiveFovLH(0.25f*DirectX::XM_PI, width/height, 1.0f, 1000.0f);
	XMStoreFloat4x4(&mProj, P);

	// Convert Spherical to Cartesian coordinates.
	float x = radius*sinf(phi)*cosf(theta);
	float z = radius*sinf(phi)*sinf(theta);
	float y = radius*cosf(phi);

	// Build the view matrix.
	XMVECTOR pos    = XMVectorSet(x, y, z, 1.0f);
	XMVECTOR target = XMVectorZero();
	XMVECTOR up     = XMVectorSet(0.0f, 1.0f, 0.0f, 0.0f);

	XMMATRIX V = XMMatrixLookAtLH(pos, target, up);
	XMStoreFloat4x4(&mView, V);

	XMMATRIX world = XMLoadFloat4x4(&mWorld);
	XMMATRIX view  = XMLoadFloat4x4(&mView);
	XMMATRIX proj  = XMLoadFloat4x4(&mProj);
	XMMATRIX worldViewProj = world * view * proj;
	///////////////////////////////////////////////////////////////////////////////////////////////

	D3D11_BUFFER_DESC vsConstBufDesc;
	vsConstBufDesc.ByteWidth           = sizeof(float) * 16;
	vsConstBufDesc.Usage               = D3D11_USAGE_DEFAULT;
	vsConstBufDesc.BindFlags           = D3D11_BIND_CONSTANT_BUFFER;
	vsConstBufDesc.CPUAccessFlags      = 0;
	vsConstBufDesc.MiscFlags           = 0;
	vsConstBufDesc.StructureByteStride = 0;

	D3D11_SUBRESOURCE_DATA vsConstBufInitData;
	vsConstBufInitData.pSysMem          = &worldViewProj;
	vsConstBufInitData.SysMemPitch      = 0;
	vsConstBufInitData.SysMemSlicePitch = 0;

	hr = pD3DDevice->CreateBuffer(&vsConstBufDesc, &vsConstBufInitData, &pVSConstBuffer); CHECK_HR(hr);

	pD3DImmediateContext->VSSetConstantBuffers(0, 1, &pVSConstBuffer);

	hr = ExitCode::Success;

Cleanup:
	SafeRelease(pVertBuffer);
	SafeRelease(pIndexBuffer);

	return hr;
}

long Renderer::Update(const GameTimer &gameTimer)
{
	HRESULT hr;

	double t = gameTimer.Time();
	float  r = sinf(1.0f * t);
	float  g = sinf(2.0f * t);
	float  b = sinf(3.0f * t);

	XMVECTORF32 color = { r, g, b, 1.0f };

	pD3DImmediateContext->ClearRenderTargetView(pRenderTargetView, color);
	pD3DImmediateContext->ClearDepthStencilView(pDepthBufferView, D3D11_CLEAR_DEPTH | D3D11_CLEAR_STENCIL, 1, 0);

	//TODO: Set view position
	//pD3DImmediateContext->VSSetConstantBuffers(0, 1, nullptr);

	pD3DImmediateContext->DrawIndexed(36, 0, 0);

	hr = pSwapChain->Present(0, 0); CHECK_HR(hr);

	UpdateFrameStatistics(gameTimer);

	hr = ExitCode::Success;

Cleanup:

	return hr;
}

void Renderer::OnTeardown()
{
	SafeRelease(pVSConstBuffer);
	SafeRelease(pInputLayout);
}