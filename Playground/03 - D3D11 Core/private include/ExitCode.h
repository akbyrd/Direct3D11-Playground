#pragma once

enum ExitCode
{
	//Ok
	Success = 0,
	Quit    = 1,

	//Errors - Renderer
	BadHWNDProvided             = -1,
	D3DFeatureLevelNotSupported = -2,
	DXGIFactoryNotInitialized   = -3,
	D3DSwapChainNotInitialized  = -4,
	D3DDeviceNotInitialized     = -5,
	D3DContextNotInitialized    = -6,
	D3DResizeFailed             = -7,

	//Errors - File Reading
	FileOpenFailed    = -8,
	FileReadFailed    = -9
};