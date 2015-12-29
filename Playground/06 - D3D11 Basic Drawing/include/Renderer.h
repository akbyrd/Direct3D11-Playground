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

	struct Vertex
	{
		DirectX::XMFLOAT3 Pos;
		DirectX::XMFLOAT4 Color;
	};

	bool InitializeInputLayout();
	ComPtr<ID3D11InputLayout> pInputLayout;

	bool InitializeBuffers();
	ComPtr<ID3D11Buffer> pVSConstBuffer;

	DirectX::XMFLOAT4X4 mWorld;
	DirectX::XMFLOAT4X4 mView;
	DirectX::XMFLOAT4X4 mProj;

	float theta  = DirectX::XM_PIDIV2*3;
	float phi    = DirectX::XM_PIDIV4;
	float radius = 5;

	//Debugging
	bool SetWireframeMode(bool enableWireframe);

	POINTS lastMousePosition;
};