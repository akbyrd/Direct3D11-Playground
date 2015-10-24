#pragma once

#include "RendererBase.h"

class Renderer : public RendererBase
{
public:
	virtual long Update(const GameTimer&);
};