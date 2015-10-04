#pragma once

class Renderer final
{
public:
	Renderer(HWND);

	long Initialize();
	long Update();
	long Teardown();

private:
	HWND hwnd = nullptr;

	long InitializeDevice();
	ID3D11Device2*        pD3DDevice           = nullptr;
	ID3D11DeviceContext2* pD3DImmediateContext = nullptr;

	long InitializeSwapChain();
	IDXGISwapChain*  pSwapChain       = nullptr;
	UINT             multiSampleCount = 4;
	UINT             numQualityLevels = 0;
	UINT             width            = 800;
	UINT             height           = 600;

	long InitializeDXGIFactory();
	long InitializeRenderTargetView();

	long InitializeDepthBuffer();

	long InitializeResourceBindings();

	long InitializeViewport();
};