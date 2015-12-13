#pragma once

#include "RendererBase.h"

class Renderer : public RendererBase
{
public:
	virtual long Update(const GameTimer&);

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

	long UpdateView(float);
	DirectX::XMFLOAT4X4 mWorld;
	DirectX::XMFLOAT4X4 mView;
	DirectX::XMFLOAT4X4 mProj;
	DirectX::XMFLOAT4X4 mWVP;

	float theta  = DirectX::XM_PIDIV4;
	float phi    = DirectX::XM_PIDIV4;
	float radius = 10;

	//Debugging
	long SetWireframeMode(bool enableWireframe);
};