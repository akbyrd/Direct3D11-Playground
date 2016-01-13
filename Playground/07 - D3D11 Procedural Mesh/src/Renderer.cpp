#include "stdafx.h"

#include <locale>
#include <codecvt>

#include "Renderer.h"
#include "Utility.h"
#include "HLSL.h"

using namespace std;
using namespace DirectX;
using namespace Utility;
using namespace HLSL;

bool Renderer::Initialize(HWND hwnd)
{
	IF( __super::Initialize(hwnd),
		FALSE, return false);

	IF( VSLoadCreateSet(L"Basic Vertex Shader"),
		FALSE, return false);

	IF( PSLoadCreateSet(L"Basic Pixel Shader"),
		FALSE, return false);

	IF( InitializeInputLayout(),
		FALSE, return false);

	IF( InitializeMesh(),
		FALSE, return false);
}

bool Renderer::VSLoadCreateSet(const wstring &filename)
{
	//Load
	unique_ptr<char[]> vsBytes;
	size_t vsBytesLength;
	IF( LoadFile(filename, vsBytes, vsBytesLength),
		FALSE, return false);

	//Create
	IF( pD3DDevice->CreateVertexShader(vsBytes.get(), vsBytesLength, nullptr, &vs),
		LOG_FAILED, return false);

	string debugName = wstring_convert<codecvt_utf8<wchar_t>>().to_bytes(filename);
	SetDebugObjectName(vs, debugName);

	//Set
	pD3DImmediateContext->VSSetShader(vs.Get(), nullptr, 0);

	return true;
}

bool Renderer::PSLoadCreateSet(const wstring &filename)
{
	//Load
	unique_ptr<char[]> psBytes;
	size_t psBytesLength;
	IF( LoadFile(filename, psBytes, psBytesLength),
		FALSE, return false);

	//Create
	IF( pD3DDevice->CreatePixelShader(psBytes.get(), psBytesLength, nullptr, &ps),
		LOG_FAILED, return false);

	string debugName = wstring_convert<codecvt_utf8<wchar_t>>().to_bytes(filename);
	SetDebugObjectName(ps, debugName);

	//Set
	pD3DImmediateContext->PSSetShader(ps.Get(), nullptr, 0);

	return true;
}

bool Renderer::InitializeInputLayout() { return true; }

bool Renderer::InitializeMesh()
{
	const Vertex meshVerts[] = {
		{ XMFLOAT3(0, 0, 0), XMFLOAT4(0, 0, 0, 0) } //TODO: Fill
	};

	const D3D11_INPUT_ELEMENT_DESC vsInputDescs[] = {
		{ Semantic::Position, 0, DXGI_FORMAT_R32G32B32_FLOAT   , 0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0 },
		{ Semantic::Color   , 0, DXGI_FORMAT_R32G32B32A32_FLOAT, 0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0 }
	};

	IF( pD3DDevice->CreateInputLayout(vsInputDescs, ArraySize(vsInputDescs), vsBytes, vsBytesLength, &vsInputLayout),
		LOG_FAILED, return false);
	SetDebugObjectName(vsInputLayout, "Input Layout");


	pD3DImmediateContext->IASetInputLayout(vsInputLayout.Get());

	D3D11_BUFFER_DESC vertBuffDesc = {};
	vertBuffDesc.ByteWidth           = sizeof(meshVerts);
	vertBuffDesc.Usage               = D3D11_USAGE_IMMUTABLE;
	vertBuffDesc.BindFlags           = D3D11_BIND_VERTEX_BUFFER;
	vertBuffDesc.CPUAccessFlags      = 0;
	vertBuffDesc.MiscFlags           = 0;
	vertBuffDesc.StructureByteStride = 0;

	D3D11_SUBRESOURCE_DATA vertBufInitData = {};
	vertBufInitData.pSysMem          = meshVerts;
	vertBufInitData.SysMemPitch      = 0;
	vertBufInitData.SysMemSlicePitch = 0;

	ComPtr<ID3D11Buffer> pVertBuffer;
	IF( pD3DDevice->CreateBuffer(&vertBuffDesc, &vertBufInitData, &pVertBuffer),
		LOG_FAILED, return false);

	return true;
}

bool Renderer::Update(const GameTimer &gameTimer)
{
	return __super::Update(gameTimer);
}

bool Renderer::Render()
{
	return __super::Render();;
}