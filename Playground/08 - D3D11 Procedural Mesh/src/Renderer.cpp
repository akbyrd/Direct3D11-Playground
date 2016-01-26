#include "stdafx.h"

#include <locale>
#include <codecvt>

#include "Renderer.h"
#include "Utility.h"
#include "HLSL.h"
#include "Logging.h"

using namespace std;
using namespace DirectX;
using namespace Utility;
using namespace HLSL;

bool Renderer::Initialize(HWND hwnd)
{
	IF( __super::Initialize(hwnd),
		IS_FALSE, return false);

	IF( VSLoadCreateSet(L"Basic Vertex Shader.cso"),
		IS_FALSE, return false);

	IF( PSLoadCreateSet(L"Basic Pixel Shader.cso"),
		IS_FALSE, return false);

	IF( InitializeMesh(),
		IS_FALSE, return false);

	IF( InitializeRasterizerStates(),
		IS_FALSE, return false);

	XMStoreFloat4x4(&world, XMMatrixIdentity());

	return true;
}

bool Renderer::VSLoadCreateSet(const wstring &filename)
{
	//Load
	unique_ptr<char[]> vsBytes;
	size_t vsBytesLength;
	IF( LoadFile(filename, vsBytes, vsBytesLength),
		IS_FALSE, return false);

	//Create
	IF( pD3DDevice->CreateVertexShader(vsBytes.get(), vsBytesLength, nullptr, &vs),
		LOG_FAILED, return false);

	string debugName = wstring_convert<codecvt_utf8<wchar_t>>().to_bytes(filename);
	SetDebugObjectName(vs, debugName);

	//Set
	pD3DImmediateContext->VSSetShader(vs.Get(), nullptr, 0);


	//Per-object constant buffer
	D3D11_BUFFER_DESC vsConstBuffDes = {};
	vsConstBuffDes.ByteWidth           = sizeof(XMFLOAT4X4);
	vsConstBuffDes.Usage               = D3D11_USAGE_DYNAMIC;
	vsConstBuffDes.BindFlags           = D3D11_BIND_CONSTANT_BUFFER;
	vsConstBuffDes.CPUAccessFlags      = D3D11_CPU_ACCESS_WRITE;
	vsConstBuffDes.MiscFlags           = 0;
	vsConstBuffDes.StructureByteStride = 0;

	IF( pD3DDevice->CreateBuffer(&vsConstBuffDes, nullptr, &vsConstBuffer),
		LOG_FAILED, return false);
	SetDebugObjectName(vsConstBuffer, "VS Constant Buffer (Per-Object)");


	//Input layout
	const D3D11_INPUT_ELEMENT_DESC vsInputDescs[] = {
		{ Semantic::Position, 0, DXGI_FORMAT_R32G32B32_FLOAT   , 0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0 },
		{ Semantic::Color   , 0, DXGI_FORMAT_R32G32B32A32_FLOAT, 0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0 }
	};

	IF( pD3DDevice->CreateInputLayout(vsInputDescs, ArraySize(vsInputDescs), vsBytes.get(), vsBytesLength, &vsInputLayout),
		LOG_FAILED, return false);
	SetDebugObjectName(vsInputLayout, "Input Layout");

	pD3DImmediateContext->IASetInputLayout(vsInputLayout.Get());
	pD3DImmediateContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

	return true;
}

bool Renderer::PSLoadCreateSet(const wstring &filename)
{
	//Load
	unique_ptr<char[]> psBytes;
	size_t psBytesLength;
	IF( LoadFile(filename, psBytes, psBytesLength),
		IS_FALSE, return false);

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
	size_t vertexCount = (meshResolutionX + 1) * (meshResolutionZ + 1);
	size_t  indexCount = meshResolutionX * meshResolutionZ * 6;

	//Vertices
	float inverseArea = 1 / (meshWidth*meshHeight);
	float halfWidth  = .5f * meshWidth;
	float halfHeight = .5f * meshHeight;

	XMVECTORF32 tl = Colors::Red;
	XMVECTORF32 tr = Colors::Green;
	XMVECTORF32 bl = Colors::Blue;
	XMVECTORF32 br = Colors::White;

	float dx = meshWidth  / meshResolutionX;
	float dz = meshHeight / meshResolutionZ;

	meshVerts = (Vertex*) _aligned_malloc(vertexCount * sizeof(Vertex), alignof(Vertex));
	IF( meshVerts,
		IS_FALSE, return false);

	for ( size_t z = 0; z <= meshResolutionZ; ++z )
	{
		size_t stride = z*(meshResolutionX + 1);

		for ( size_t x = 0; x <= meshResolutionX; ++x )
		{
			float vx = x*dx;
			float vz = z*dz;

			meshVerts[x + stride].position = XMFLOAT3(vx - halfWidth, 0, vz - halfHeight);

			float xl = vx;
			float xr = meshWidth - vx;
			float zt = vz;
			float zb = meshHeight - vz;

			XMStoreFloat4(&meshVerts[x + stride].color, (
				  tl * (xr * zb * inverseArea)
				+ tr * (xl * zb * inverseArea)
				+ bl * (xr * zt * inverseArea)
				+ br * (xl * zt * inverseArea)
			));
		}
	}

	D3D11_BUFFER_DESC vertBuffDesc = {};
	vertBuffDesc.ByteWidth           = sizeof(Vertex) * vertexCount;
	vertBuffDesc.Usage               = D3D11_USAGE_DYNAMIC;
	vertBuffDesc.BindFlags           = D3D11_BIND_VERTEX_BUFFER;
	vertBuffDesc.CPUAccessFlags      = D3D11_CPU_ACCESS_WRITE;
	vertBuffDesc.MiscFlags           = 0;
	vertBuffDesc.StructureByteStride = 0;

	D3D11_SUBRESOURCE_DATA vertBuffInitData = {};
	vertBuffInitData.pSysMem          = meshVerts;
	vertBuffInitData.SysMemPitch      = 0;
	vertBuffInitData.SysMemSlicePitch = 0;

	IF( pD3DDevice->CreateBuffer(&vertBuffDesc, &vertBuffInitData, &meshVertexBuffer),
		LOG_FAILED, return false);
	SetDebugObjectName(meshVertexBuffer, "Mesh Vertex Buffer");

	const UINT stride = sizeof(Vertex);
	const UINT offset = 0;
	pD3DImmediateContext->IASetVertexBuffers(0, 1, meshVertexBuffer.GetAddressOf(), &stride, &offset);
	pD3DImmediateContext->VSSetConstantBuffers(0, 1, vsConstBuffer.GetAddressOf());


	//Indices
	unique_ptr<UINT[]> meshIndices(new UINT[indexCount]);
	IF( meshIndices,
		IS_FALSE, return false);

	for ( size_t z = 0; z < meshResolutionZ; ++z )
	{
		size_t rowOffset = z*6*meshResolutionX;

		for ( size_t x = 0; x < meshResolutionX; ++x )
		{
			size_t offset = 6*x + rowOffset;

			//Top left triangle
			meshIndices[offset + 0] = (z+1)*(meshResolutionX+1) + x;
			meshIndices[offset + 1] = (z  )*(meshResolutionX+1) + x;
			meshIndices[offset + 2] = (z  )*(meshResolutionX+1) + x + 1;

			//Bottom right triangle
			meshIndices[offset + 3] = meshIndices[offset + 2];
			meshIndices[offset + 4] = (z+1)*(meshResolutionX+1) + x + 1;
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

	IF( pD3DDevice->CreateBuffer(&indexBuffDesc, &indexBuffInitData, &meshIndexBuffer),
		LOG_FAILED, return false);
	SetDebugObjectName(meshIndexBuffer, "Mesh Index Buffer");

	pD3DImmediateContext->IASetIndexBuffer(meshIndexBuffer.Get(), DXGI_FORMAT_R32_UINT, 0);

	return true;
}

bool Renderer::InitializeRasterizerStates()
{
	/* NOTE: MultisampleEnable toggles between quadrilateral AA (true) and alpha AA (false).
	 * Alpha AA has a massive performance impact while quadrilateral is much smaller (negligible
	 * for the mesh drawn here). Visually, it's hard to tell the difference between quadrilateral
	 * AA on and off in this demo. Alpha AA on the other is more obvious. It causes the wireframe
	 * to draw lines 2 px wide instead of 1.
	 * 
	 * See remarks: https://msdn.microsoft.com/en-us/library/windows/desktop/ff476198(v=vs.85).aspx
	 */
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
	rasterizerDesc.MultisampleEnable     = multiSampleCount > 1 && useQuadrilateralLineAA;
	rasterizerDesc.AntialiasedLineEnable = multiSampleCount > 1;

	IF( pD3DDevice->CreateRasterizerState(&rasterizerDesc, &rasterizerStateSolid),
		LOG_FAILED, return false);
	SetDebugObjectName(rasterizerStateSolid, "Rasterizer State (Solid)");


	//Wireframe
	rasterizerDesc.FillMode = D3D11_FILL_WIREFRAME;

	IF( pD3DDevice->CreateRasterizerState(&rasterizerDesc, &rasterizerStateWireframe),
		LOG_FAILED, return false);
	SetDebugObjectName(rasterizerStateWireframe, "Rasterizer State (Wireframe)");

	//Start off in correct state
	UpdateRasterizeState();

	return true;
}

bool Renderer::Resize()
{
	IF( __super::Resize(),
		IS_FALSE, return false);

	XMMATRIX P = XMMatrixPerspectiveFovLH(XM_PIDIV4, (float) width / height, 0.1f, 100.0f);
	XMStoreFloat4x4(&proj, P);

	return true;
}

void Renderer::UpdateRasterizeState()
{
	if ( isWireframeEnabled )
	{
		pD3DImmediateContext->RSSetState(rasterizerStateWireframe.Get());
	}
	else
	{
		pD3DImmediateContext->RSSetState(rasterizerStateSolid.Get());
	}
}

bool Renderer::Update(const GameTimer &gameTimer, const HostWindow::Input* input)
{
	IF( __super::Update(gameTimer),
		IS_FALSE, return false);

	ProcessInput(input);

	/*TODO: Figure out performance
			- Default vs immutable
			- Debug vs release
			- Fullscreen is slow
			- UpdateSubresource vs Map
			- Performance counters
	*/

	//Animate mesh
	double t = gameTimer.Time();
	float scaledT = (float) t * XM_2PI / meshAmplitudePeriod;

	Vertex *vertex = meshVerts;
	float xPhaseScale = XM_2PI / meshResolutionX;
	float zPhaseScale = XM_2PI / meshResolutionZ;

	for ( float z = 0; z <= meshResolutionZ; ++z )
	{
		float zHeight = sinf(scaledT + z*zPhaseScale);

		for ( float x = 0; x <= meshResolutionX; ++x )
		{
			(vertex++)->position.y = meshMaxAmplitude * (
				sinf(scaledT + x*xPhaseScale)
				+ zHeight
			);
		}
	}

	/* NOTE:
	 *   UpdateSubresource  - .39 ms
	 *   Map, loop, Unmap   - .63 ms
	 *   Map, memcpy, Unmap - .32 ms
	 */
	D3D11_MAPPED_SUBRESOURCE meshVertexBuffMap = {};
	IF( pD3DImmediateContext->Map(meshVertexBuffer.Get(), 0, D3D11_MAP_WRITE_DISCARD, 0, &meshVertexBuffMap),
		LOG_FAILED, return false);

	size_t vertCount = (meshResolutionX + 1) * (meshResolutionZ + 1);
	memcpy(meshVertexBuffMap.pData, meshVerts, sizeof(Vertex)*vertCount);

	pD3DImmediateContext->Unmap(meshVertexBuffer.Get(), 0);

	//Update Camera
	float x = radius*sinf(phi)*cosf(theta);
	float z = radius*sinf(phi)*sinf(theta);
	float y = radius*cosf(phi);

	XMVECTOR pos    = XMVectorSet(x, y, z, 1);
	XMVECTOR target = XMVectorZero();
	XMVECTOR up     = XMVectorSet(0, 1, 0, 0);

	XMMATRIX V = XMMatrixLookAtLH(pos, target, up);
	XMStoreFloat4x4(&view, V);

	XMMATRIX W = XMLoadFloat4x4(&world);
	XMMATRIX P = XMLoadFloat4x4(&proj);
	XMMATRIX WVP = XMMatrixTranspose(W * V * P);

	//Update VS cbuffer
	D3D11_MAPPED_SUBRESOURCE vsConstBuffMap = {};
	IF( pD3DImmediateContext->Map(vsConstBuffer.Get(), 0, D3D11_MAP_WRITE_DISCARD, 0, &vsConstBuffMap),
		LOG_FAILED, return false);

	memcpy(vsConstBuffMap.pData, &WVP, sizeof(WVP));
	pD3DImmediateContext->Unmap(vsConstBuffer.Get(), 0);

	return true;
}

void Renderer::ProcessInput(const HostWindow::Input* input)
{
	const float epsilon = numeric_limits<float>::epsilon();

	//Rotate
	if ( input->mouseLeft.isDown )
	{
		int16f dx = input->mouseX - lastMousePosition.x;
		int16f dy = input->mouseY - lastMousePosition.y;

		theta -= .006f * dx;
		phi   -= .006f * dy;

		     if ( phi <  epsilon ) phi = epsilon;
		else if ( phi >= XM_PI   ) phi = XM_PI - epsilon;

		SetCapture(hwnd);
	}
	//Zoom
	else if ( input->mouseRight.isDown )
	{
		int16f dx = input->mouseX - lastMousePosition.x;
		int16f dy = input->mouseY - lastMousePosition.y;

		radius += .01f * (dy - dx);

		SetCapture(hwnd);
	}
	else
	{
		ReleaseCapture();

		//Toggle wireframe
		if ( input->mouseMiddle.isDown && input->mouseMiddle.transitionCount % 2 == 1 )
		{
			isWireframeEnabled = !isWireframeEnabled;
			UpdateRasterizeState();
		}
	}

	//Zoom
	int8f zoomSteps = 0;
	zoomSteps += input->mouseWheelUp.transitionCount;
	zoomSteps -= input->mouseWheelDown.transitionCount;

	radius -= .4f * zoomSteps;

	     if ( radius < epsilon ) radius = epsilon;
	else if ( radius > 50      ) radius = 50;

	lastMousePosition.x = input->mouseX;
	lastMousePosition.y = input->mouseY;
}

bool Renderer::Render()
{
	pD3DImmediateContext->ClearRenderTargetView(pRenderTargetView.Get(), Colors::Black);
	pD3DImmediateContext->ClearDepthStencilView(pDepthBufferView.Get(), D3D11_CLEAR_DEPTH | D3D11_CLEAR_STENCIL, 1, 0);

	size_t indexCount = meshResolutionX * meshResolutionZ * 6;
	pD3DImmediateContext->DrawIndexed(indexCount, 0, 0);

	IF( pSwapChain->Present(0, 0),
		LOG_FAILED, return false);

	return true;
}

void Renderer::Teardown()
{
	vs.Reset();
	vsInputLayout.Reset();
	ps.Reset();
	meshVertexBuffer.Reset();
	meshIndexBuffer.Reset();
	_aligned_free(meshVerts);

	__super::Teardown();
}