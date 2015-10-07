#pragma once

enum ExitCode
{
	//Ok
	Success = 0,
	Quit    = 1,

	//Errors
	PeekMessageFailed      = -1,
	WindowNewFailed        = -2,
	WindowInitializeFailed = -3
};