#pragma once

#include "RendererBase.h"

class Renderer : public RendererBase
{
public:
	virtual bool Initialize(HWND);
	virtual bool Update(const GameTimer &gameTimer);
	virtual bool Render();

private:
	using XMFLOAT3  = DirectX::XMFLOAT3;

	struct Vertex
	{
		XMFLOAT3 position;
		XMFLOAT4 color;
	};

	struct Mesh
	{
		XMFLOAT3 vertices[];
		UINT     indices[];
	};

	bool VSLoadCreateSet(const wstring&);
	ComPtr<ID3D11VertexShader> vs;

	bool PSLoadCreateSet(const wstring&);
	ComPtr<ID3D11PixelShader> ps;

	bool InitializeInputLayout();
	ComPtr<ID3D11InputLayout> vsInputLayout;

	bool InitializeMesh();
	ComPtr<ID3D11Buffer> meshVertexBuffer;
	ComPtr<ID3D11Buffer> meshIndexBuffer;
};