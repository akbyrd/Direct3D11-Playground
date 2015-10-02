#include "stdafx.h"

class Renderer final
{
public:
	Renderer();

	long Init();

private:
	long InitializeDevice();
	ID3D11Device*        d3dDevice;
	ID3D11DeviceContext* d3dImmediateContext;

	long InitializeSwapChain();
	long InitializeDXGIFactory();
	long InitializeRenderTargetView();

	long InitializeDepthBuffer();

	long InitializeResourceBindings();

	long InitializeViewport();
};