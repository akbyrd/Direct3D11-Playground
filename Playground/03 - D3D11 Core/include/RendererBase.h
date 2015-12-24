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
	CComPtr<ID3D11Device>        pD3DDevice;
	CComPtr<ID3D11DeviceContext> pD3DImmediateContext;
	CComPtr<IDXGIFactory1>       pDXGIFactory;

	bool InitializeSwapChain();
	bool CreateBackBufferView();
	bool UpdateAllowFullscreen();
	CComPtr<IDXGISwapChain>         pSwapChain;
	CComPtr<ID3D11RenderTargetView> pRenderTargetView;

	bool InitializeDepthBuffer();
	CComPtr<ID3D11DepthStencilView> pDepthBufferView;

	void InitializeOutputMerger();
	void InitializeViewport();

	void UpdateFrameStatistics(const GameTimer&);
	double averageFrameTime = 0;

	bool LogAdapters();
	bool LogOutputs(CComPtr<IDXGIAdapter1>);
	bool LogDisplayModes(CComPtr<IDXGIOutput>);
	bool LogLiveObjects();

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