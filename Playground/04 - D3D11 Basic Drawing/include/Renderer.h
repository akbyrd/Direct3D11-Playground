#pragma once

#include "RendererBase.h"

class Renderer : public RendererBase
{
public:
	virtual long Update(const GameTimer&);
	virtual long Render();

	//TODO: This is awful. Implement proper input handling.
	void HandleInput(bool, bool, POINTS);

protected:
	virtual long OnInitialize();
	virtual long OnResize();
	virtual void OnTeardown();

private:
	struct Vertex
	{
		DirectX::XMFLOAT3 Pos;
		DirectX::XMFLOAT4 Color;
	};

	long InitializeInputLayout();
	ID3D11InputLayout* pInputLayout = nullptr;

	long InitializeBuffers();
	ID3D11Buffer* pVSConstBuffer = nullptr;

	DirectX::XMFLOAT4X4 mWorld;
	DirectX::XMFLOAT4X4 mView;
	DirectX::XMFLOAT4X4 mProj;

	float theta  = DirectX::XM_PIDIV2*3;
	float phi    = DirectX::XM_PIDIV4;
	float radius = 5;

	//Debugging
	long SetWireframeMode(bool enableWireframe);

	POINTS lastMousePosition;
};