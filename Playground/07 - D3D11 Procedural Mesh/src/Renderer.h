#pragma once

#include <string>
#include <memory>

#include "RendererBase.h"

class Renderer : public RendererBase
{
public:
	virtual bool Initialize(HWND);
	virtual bool Update(const GameTimer &gameTimer);
	virtual bool Render();
	virtual void Teardown();

private:
	using XMFLOAT3 = DirectX::XMFLOAT3;
	using XMFLOAT4 = DirectX::XMFLOAT4;

	struct Vertex
	{
		XMFLOAT3 position;
		XMFLOAT4 color;
	};

	bool VSLoadCreateSet(const std::wstring&);
	ComPtr<ID3D11VertexShader> vs;
	ComPtr<ID3D11InputLayout> vsInputLayout;

	bool PSLoadCreateSet(const std::wstring&);
	ComPtr<ID3D11PixelShader> ps;

	bool InitializeMesh();
	ComPtr<ID3D11Buffer> meshVertexBuffer;
	ComPtr<ID3D11Buffer> meshIndexBuffer;
	std::unique_ptr<Vertex[]> meshVerts;

	uint16_t meshWidth  = 256;
	uint16_t meshHeight = 256;
};