#pragma once

enum ExitCode
{
	//Ok
	Success = 0,
	Quit    = 1,

	//Errors - Main
	PeekMessageFailed           = -1,

	//Errors - Renderer
	BadHWNDProvided             = -2,
	D3DFeatureLevelNotSupported = -3,
	DXGIFactoryNotInitialized   = -4,
	D3DSwapChainNotInitialized  = -5,
	D3DDeviceNotInitialized     = -6,
	D3DContextNotInitialized    = -7,
	D3DResizeFailed             = -8
};