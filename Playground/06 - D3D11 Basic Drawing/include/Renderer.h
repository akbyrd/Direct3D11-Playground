#pragma once

#include <wrl\client.h>

#include "RendererBase.h"

class Renderer : public RendererBase
{
public:
	virtual bool Initialize(HWND);
	virtual bool Resize();
	virtual bool Update(const GameTimer&);
	virtual bool Render();
	virtual void Teardown();

	//TODO: This is awful. Implement proper input handling.
	void HandleInput(bool, bool, POINTS);

private:
	template<typename T>
	using ComPtr = Microsoft::WRL::ComPtr<T>;
	using XMFLOAT3 = DirectX::XMFLOAT3;
	using XMFLOAT4 = DirectX::XMFLOAT4;
	using XMFLOAT4X4 = DirectX::XMFLOAT4X4;

	struct Vertex
	{
		XMFLOAT3 Pos;
		XMFLOAT4 Color;
	};

	bool InitializeInputLayout();
	ComPtr<ID3D11InputLayout> pInputLayout;

	bool InitializeBuffers();
	ComPtr<ID3D11Buffer> pVSConstBuffer;

	XMFLOAT4X4 mWorld;
	XMFLOAT4X4 mView;
	XMFLOAT4X4 mProj;

	float theta  = DirectX::XM_PIDIV2*3;
	float phi    = DirectX::XM_PIDIV4;
	float radius = 5;

	//Debugging
	bool SetWireframeMode(bool enableWireframe);

	POINTS lastMousePosition;
};