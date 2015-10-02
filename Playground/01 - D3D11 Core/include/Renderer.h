#include "stdafx.h"

class Renderer final
{
public:
	Renderer();

	void Init();

private:
	bool InitializeDevice();
	ID3D11Device*        d3dDevice;
	ID3D11DeviceContext* d3dDeviceContext;

	void InitializeSwapChain();
	void InitializeDXGIFactory();
	void InitializeRenderTargetView();

	void InitializeDepthBuffer();

	void InitializeResourceBindings();

	void InitializeViewport();
};