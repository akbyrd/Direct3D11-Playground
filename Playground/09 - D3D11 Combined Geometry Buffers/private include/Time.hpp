#pragma once

#include "Platform.h"

//TODO: Add scale factor and drift accumulator?
struct SimTime
{
	r32 t;
	r32 dt;
	u64 ticks;
};

static inline void
AddTicks(SimTime* simTime, u64 newTicks, u64 tickFrequency)
{
	simTime->ticks += newTicks;

	simTime->t  = (r32) simTime->ticks / tickFrequency;
	simTime->dt = (r32)       newTicks / tickFrequency;
}