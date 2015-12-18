#pragma once

enum ExitCode
{
	//Ok
	Success = 0,
	Quit    = 1,

	//Errors - Renderer
	D3DFeatureLevelNotSupported = -2,
	D3DResizeFailed             = -7,

	//Errors - File Reading
	FileOpenFailed    = -8,
	FileReadFailed    = -9
};