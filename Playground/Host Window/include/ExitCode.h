enum ExitCode
{
	//Ok
	Success = 0,
	Quit    = 1,

	//Errors
	ApplicationPeekMessageFailed = -1,
	WindowPeekMessageFailed      = -2,
};