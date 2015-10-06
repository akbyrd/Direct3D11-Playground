#pragma once

enum ExitCode : long
{
	//Ok
	Success = 0,
	Quit    = 1,

	//Errors
	WindowAllocFailed           = -1,
	RendererAllocFailed         = -2,
	TimerAllocFailed            = -3,
	PeekMessageFailed           = -4,
	D3DFeatureLevelNotSupported = -5,
};