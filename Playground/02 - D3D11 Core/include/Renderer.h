#pragma once

#include "GameTimer.h"

class Renderer final
{
public:
	long Initialize(HWND);
	long Update(GameTimer*);
	long Teardown();

private:
	HWND hwnd = nullptr;

	long InitializeDevice();
	long CheckForWarpDriver();
	long ObtainDXGIFactory();
	ID3D11Device*        pD3DDevice           = nullptr;
	ID3D11DeviceContext* pD3DImmediateContext = nullptr;
	IDXGIFactory1*       pDXGIFactory         = nullptr;

	long InitializeSwapChain();
	long UpdateAllowFullscreen();
	IDXGISwapChain*         pSwapChain        = nullptr;
	ID3D11RenderTargetView* pRenderTargetView = nullptr;

	long InitializeDepthBuffer();
	ID3D11DepthStencilView* pDepthBufferView  = nullptr;

	long InitializeOutputMerger();
	long InitializeViewport();

	long LogAdapters();
	long LogOutputs(IDXGIAdapter1*);
	long LogDisplayModes(IDXGIOutput*);
	void LogLiveObjects();

	//TODO: Encapsulate in a struct?
	//Settings
	bool startFullscreen   = false;
	bool allowFullscreen   = true;
	UINT numQualityLevels  = 0;
	UINT multiSampleCount  = 4;
	UINT width             = 800;
	UINT height            = 600;
};