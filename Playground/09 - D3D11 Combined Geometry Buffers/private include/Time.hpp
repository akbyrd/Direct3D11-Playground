#pragma once

#include "Platform.h"

//TODO: Add scale factor and drift accumulator?
struct SimTime
{
	real32 t;
	real32 dt;
	uint64 ticks;
};

static inline void
AddTicks(SimTime* simTime, uint64 newTicks, uint64 tickFrequency)
{
	simTime->ticks += newTicks;

	simTime->t  = (real32) simTime->ticks / tickFrequency;
	simTime->dt = (real32)       newTicks / tickFrequency;
}