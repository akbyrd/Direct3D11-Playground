#pragma once

#include <string>
#include <memory>

#include "RendererBase.h"
#include "HostWindow.h"

class Renderer : public RendererBase
{
public:
	virtual bool Initialize(HWND);
	virtual bool Resize();
	virtual bool Update(const GameTimer &gameTimer, const HostWindow::Input*);
	virtual bool Render();
	virtual void Teardown();


private:
	using XMFLOAT3   = DirectX::XMFLOAT3;
	using XMFLOAT4   = DirectX::XMFLOAT4;
	using XMFLOAT4X4 = DirectX::XMFLOAT4X4;

	struct Vertex
	{
		XMFLOAT3 position;
		XMFLOAT4 color;
	};

	bool VSLoadCreateSet(const std::wstring&);
	ComPtr<ID3D11VertexShader> vs;
	ComPtr<ID3D11InputLayout> vsInputLayout;
	ComPtr<ID3D11Buffer> vsConstBuffer;

	bool PSLoadCreateSet(const std::wstring&);
	ComPtr<ID3D11PixelShader> ps;

	bool InitializeMesh();
	ComPtr<ID3D11Buffer> meshVertexBuffer;
	ComPtr<ID3D11Buffer> meshIndexBuffer;
	std::unique_ptr<Vertex[]> meshVerts;

	bool InitializeRasterizerStates();
	void UpdateRasterizeState();
	ComPtr<ID3D11RasterizerState> rasterizerStateSolid;
	ComPtr<ID3D11RasterizerState> rasterizerStateWireframe;

	void ProcessInput(const HostWindow::Input*);

	uint16_t meshResolutionX     = 64;
	uint16_t meshResolutionZ     = 64;
	float    meshWidth           = 10;
	float    meshHeight          = 10;
	float    meshMaxAmplitude    = 1;
	float    meshAmplitudePeriod = 4;

	float  radius            = 14;
	float  theta             = DirectX::XM_PIDIV4;
	float  phi               = DirectX::XM_PIDIV4 * 1.5f;
	POINTS lastMousePosition = {};
	bool  isWireframeEnabled = true;

	XMFLOAT4X4 world;
	XMFLOAT4X4 view;
	XMFLOAT4X4 proj;
};