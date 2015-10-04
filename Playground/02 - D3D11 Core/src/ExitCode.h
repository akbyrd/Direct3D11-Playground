#pragma once

enum ExitCode : long
{
	//Ok
	Success = 0,
	Quit    = 1,

	//Errors
	WindowAllocFailed           = -1,
	RendererAllocFailed         = -2,
	PeekMessageFailed           = -3,
	D3DFeatureLevelNotSupported = -4,
};