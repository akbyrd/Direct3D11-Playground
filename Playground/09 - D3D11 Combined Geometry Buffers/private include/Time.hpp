#pragma once

#include "Platform.h"

//TODO: Add scale factor and drift accumulator?
struct SimTime
{
	float32 t;
	float32 dt;
	uint64  ticks;

	uint64 tickFrequency;

	//TODO: Ensure platform never gives negative ticks (possible when thread is shuffled)
	inline void
	AddTicks(uint64 newTicks)
	{
		ticks += newTicks;

		 t = (float32)    ticks / tickFrequency;
		dt = (float32) newTicks / tickFrequency;
	}
};