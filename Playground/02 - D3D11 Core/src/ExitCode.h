#pragma once

enum ExitCode : long
{
	//Ok
	Success = 0,
	Quit    = 1,

	//Errors - Main
	WindowAllocFailed           = -1,
	RendererAllocFailed         = -2,
	TimerAllocFailed            = -3,
	PeekMessageFailed           = -4,

	//Errors - Renderer
	BadHWNDProvided             = -5,
	D3DFeatureLevelNotSupported = -6,
	SwapChainNotInitialized     = -7,
	D3DDeviceNotInitialized     = -8
};