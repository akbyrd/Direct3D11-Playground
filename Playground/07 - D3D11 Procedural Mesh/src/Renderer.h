#pragma once

#include <string>

#include "RendererBase.h"

class Renderer : public RendererBase
{
public:
	virtual bool Initialize(HWND);
	virtual bool Update(const GameTimer &gameTimer);
	virtual bool Render();

private:
	using XMFLOAT3 = DirectX::XMFLOAT3;

	struct Vertex
	{
		XMFLOAT3 position;
		XMFLOAT4 color;
	};

	struct Mesh
	{
		//UINT     indices[];
		XMFLOAT3 vertices[];
	};

	bool VSLoadCreateSet(const std::wstring&);
	ComPtr<ID3D11VertexShader> vs;
	ComPtr<ID3D11InputLayout> vsInputLayout;

	bool PSLoadCreateSet(const std::wstring&);
	ComPtr<ID3D11PixelShader> ps;

	bool InitializeMesh();
	ComPtr<ID3D11Buffer> meshVertexBuffer;
	ComPtr<ID3D11Buffer> meshIndexBuffer;
};