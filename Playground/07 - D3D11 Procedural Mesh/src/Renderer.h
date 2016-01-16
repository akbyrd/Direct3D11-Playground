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

	//BUG: Doesn't handle failed allocations
	struct Mesh
	{
		XMFLOAT3 *vertices;
		XMFLOAT4 *colors;
		size_t    vertexCount;

		UINT    *indices;
		size_t   indexCount;

		Mesh(size_t vertexCount, size_t indexCount)
		{
			Mesh::vertexCount = vertexCount;
			Mesh::indexCount  = indexCount;

			vertices = new XMFLOAT3[vertexCount];
			colors   = new XMFLOAT4[vertexCount];

			indices = new UINT[indexCount];
		}

		~Mesh()
		{
			delete[] vertices;
			delete[] colors;
			delete[] indices;
		}

	private:
		Mesh(const Mesh&) = delete;
		Mesh& operator =(const Mesh&) = delete;
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
	//std::unique_ptr<Mesh> mesh;

	uint16_t meshWidth  = 256;
	uint16_t meshHeight = 256;
};