#include "Simulation.h"
#include "Time.hpp"

struct SimState
{
	SimTime gameTime;
	SimTime realTime;
	bool    isPaused;
};

/*
#define PushMemory(sm, type) (type*) PushMemory_(sm, type, sizeof(type))
internal void*
PushMemory_(SimState* simState, void* memory, uint32 size)
{
	Assert(simMemory->size + size <= simMemory->maxSize);

	void* p = simMemory->bytes[simMemory->size];
	simMemory->size += size;

	return p;
};
*/

void
InitializeSimulation(SimMemory *simMemory)
{
	SimState* simState = (SimState*) simMemory->bytes;
}

void
UpdateSimulation(SimMemory *simMemory)
{
	SimState* simState = (SimState*) simMemory->bytes;

	AddTicks(&simState->realTime, simMemory->input.newTicks, simMemory->tickFrequency);
	if (!simState->isPaused)
		AddTicks(&simState->gameTime, simMemory->input.newTicks, simMemory->tickFrequency);

	//Update sim?
	//Render
}