#pragma once

#include "RendererBase.h"

class Renderer : public RendererBase
{
public:
	virtual bool Update(const GameTimer&);
	virtual bool Render();

	//TODO: This is awful. Implement proper input handling.
	void HandleInput(bool, bool, POINTS);

protected:
	virtual bool OnInitialize();
	virtual bool OnResize();
	virtual void OnTeardown();

private:
	struct Vertex
	{
		DirectX::XMFLOAT3 Pos;
		DirectX::XMFLOAT4 Color;
	};

	bool InitializeInputLayout();
	CComPtr<ID3D11InputLayout> pInputLayout;

	bool InitializeBuffers();
	CComPtr<ID3D11Buffer> pVSConstBuffer;

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