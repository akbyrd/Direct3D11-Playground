#pragma once

#include "GameTimer.h"

class RendererBase
{
public:
	bool Initialize(HWND);
	bool Resize();
	virtual bool Update(const GameTimer&);
	void Teardown();

protected:
	HWND hwnd = nullptr;

	void SetHwnd(HWND);
	bool InitializeDevice();
	bool CheckForWarpDriver();
	bool ObtainDXGIFactory();
	ID3D11Device*        pD3DDevice           = nullptr;
	ID3D11DeviceContext* pD3DImmediateContext = nullptr;
	IDXGIFactory1*       pDXGIFactory         = nullptr;

	bool InitializeSwapChain();
	bool CreateBackBufferView();
	bool UpdateAllowFullscreen();
	IDXGISwapChain*         pSwapChain        = nullptr;
	ID3D11RenderTargetView* pRenderTargetView = nullptr;

	bool InitializeDepthBuffer();
	ID3D11DepthStencilView* pDepthBufferView  = nullptr;

	void InitializeOutputMerger();
	void InitializeViewport();

	void UpdateFrameStatistics(const GameTimer&);
	double averageFrameTime = 0;

	bool LogAdapters();
	bool LogOutputs(IDXGIAdapter1*);
	bool LogDisplayModes(IDXGIOutput*);
	void LogLiveObjects();

	virtual bool OnInitialize();
	virtual bool OnResize();
	virtual void OnTeardown();

	//TODO: Encapsulate in a struct?
	//Settings
	bool startFullscreen   = false;
	bool allowFullscreen   = true;
	UINT numQualityLevels  = 0;
	UINT multiSampleCount  = 4;
	UINT width             = 800;
	UINT height            = 600;
};