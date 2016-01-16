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

	IF( VSLoadCreateSet(L"Basic Vertex Shader.cso"),
		FALSE, return false);

	IF( PSLoadCreateSet(L"Basic Pixel Shader.cso"),
		FALSE, return false);

	IF( InitializeMesh(),
		FALSE, return false);

	return true;
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


	//Input layout
	const D3D11_INPUT_ELEMENT_DESC vsInputDescs[] = {
		{ Semantic::Position, 0, DXGI_FORMAT_R32G32B32_FLOAT   , 0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0 },
		{ Semantic::Color   , 0, DXGI_FORMAT_R32G32B32A32_FLOAT, 0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0 }
	};

	IF( pD3DDevice->CreateInputLayout(vsInputDescs, ArraySize(vsInputDescs), vsBytes.get(), vsBytesLength, &vsInputLayout),
		LOG_FAILED, return false);
	SetDebugObjectName(vsInputLayout, "Input Layout");

	pD3DImmediateContext->IASetInputLayout(vsInputLayout.Get());

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

bool Renderer::InitializeMesh()
{
	size_t vertexCount = (meshWidth + 1) * (meshHeight + 1);
	size_t  indexCount = meshWidth * meshHeight * 6;

	//Vertices
	float dx = 10.0f / meshWidth;
	float dy = 10.0f / meshHeight;

	//mesh.reset(new Mesh(vertexCount, indexCount));
	//IF( mesh,
	//	FALSE, return false);

	meshVerts.reset(new Vertex[vertexCount]);
	IF( meshVerts,
		FALSE, return false);

	for ( size_t y = 0; y <= meshHeight; ++y )
	{
		size_t stride = y*meshWidth;

		for ( size_t x = 0; x <= meshWidth; ++x )
		{
			//TODO: Is this less efficient than offsetting the pointer directly? Check the asm.
			//Vertex *v = &meshVerts[x + stride];
			//
			//v->position = XMFLOAT3(x*dx, y*dy, 0);
			//v->color    = XMFLOAT4(1, 1, 1, 1);

			meshVerts[x + stride].position = XMFLOAT3(x*dx, y*dy, 0);
			meshVerts[x + stride].color    = XMFLOAT4(1, 1, 1, 1);
		}
	}

	D3D11_BUFFER_DESC vertBuffDesc = {};
	vertBuffDesc.ByteWidth           = sizeof(Vertex) * vertexCount;
	vertBuffDesc.Usage               = D3D11_USAGE_IMMUTABLE;
	vertBuffDesc.BindFlags           = D3D11_BIND_VERTEX_BUFFER;
	vertBuffDesc.CPUAccessFlags      = 0;
	vertBuffDesc.MiscFlags           = 0;
	vertBuffDesc.StructureByteStride = 0;

	D3D11_SUBRESOURCE_DATA vertBuffInitData = {};
	vertBuffInitData.pSysMem          = meshVerts.get();
	vertBuffInitData.SysMemPitch      = 0;
	vertBuffInitData.SysMemSlicePitch = 0;

	ComPtr<ID3D11Buffer> vertBuffer;
	IF( pD3DDevice->CreateBuffer(&vertBuffDesc, &vertBuffInitData, &vertBuffer),
		LOG_FAILED, return false);


	//Indices
	unique_ptr<UINT[]> meshIndices(new UINT[indexCount]);
	IF( meshIndices,
		FALSE, return false);

	for ( size_t y = 0; y < meshHeight; ++y )
	{
		size_t rowOffset = y*meshWidth;

		for ( size_t x = 0; x < meshWidth; ++x )
		{
			size_t offset = 6*x + rowOffset;

			//Top left triangle
			meshIndices[offset + 0] = (y+1)*(meshWidth+1) + x;
			meshIndices[offset + 1] = (y  )*(meshWidth+1) + x;
			meshIndices[offset + 2] = (y  )*(meshWidth+1) + x + 1;

			//Bottom right triangle
			meshIndices[offset + 3] = meshIndices[offset + 2];
			meshIndices[offset + 4] = (y+1)*(meshWidth+1) + x + 1;
			meshIndices[offset + 5] = meshIndices[offset + 0];
		}
	}

	D3D11_BUFFER_DESC indexBuffDesc = {};
	indexBuffDesc.ByteWidth           = sizeof(UINT) * indexCount;
	indexBuffDesc.Usage               = D3D11_USAGE_IMMUTABLE;
	indexBuffDesc.BindFlags           = D3D11_BIND_INDEX_BUFFER;
	indexBuffDesc.CPUAccessFlags      = 0;
	indexBuffDesc.MiscFlags           = 0;
	indexBuffDesc.StructureByteStride = 0;

	D3D11_SUBRESOURCE_DATA indexBuffInitData = {};
	indexBuffInitData.pSysMem          = meshIndices.get();
	indexBuffInitData.SysMemPitch      = 0;
	indexBuffInitData.SysMemSlicePitch = 0;

	ComPtr<ID3D11Buffer> indexBuffer;
	IF( pD3DDevice->CreateBuffer(&indexBuffDesc, &indexBuffInitData, &indexBuffer),
		LOG_FAILED, return false);

	return true;
}

bool Renderer::Update(const GameTimer &gameTimer)
{
	IF( __super::Update(gameTimer),
		FALSE, return false);

	//TODO: pD3DImmediateContext->UpdateSubresource
	//TODO: It might be faster to use a dynamic buffer and map/unmap

	return true;
}

bool Renderer::Render()
{
	IF( __super::Render(),
		FALSE, return false);

	//...

	return true;
}

void Renderer::Teardown()
{
	vs.Reset();
	vsInputLayout.Reset();
	ps.Reset();
	meshVertexBuffer.Reset();
	meshIndexBuffer.Reset();
	meshVerts.reset();

	__super::Teardown();
}