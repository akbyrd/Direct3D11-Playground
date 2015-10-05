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
	long CheckForWarpDriver();
	ID3D11Device*        pD3DDevice           = nullptr;
	ID3D11DeviceContext* pD3DImmediateContext = nullptr;

	long InitializeSwapChain();
	IDXGISwapChain*         pSwapChain        = nullptr;
	ID3D11RenderTargetView* pRenderTargetView = nullptr;
	UINT                    multiSampleCount  = 4;
	UINT                    numQualityLevels  = 0;
	UINT                    width             = 800;
	UINT                    height            = 600;

	long InitializeDepthBuffer();
	ID3D11DepthStencilView* pDepthBufferView = nullptr;

	long InitializeOutputMerger();
	long InitializeViewport();
};