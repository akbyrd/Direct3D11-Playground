#include "stdafx.h"
#include "Renderer.h"
#include "ExitCode.h"

long Renderer::Update(const GameTimer &gameTimer)
{
	UpdateFrameStatistics(gameTimer);

	return ExitCode::Success;
}