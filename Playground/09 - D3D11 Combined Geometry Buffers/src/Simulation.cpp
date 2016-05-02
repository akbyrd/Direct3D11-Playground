#include "Platform.h"
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
UpdateSimulation(SimMemory *simMemory, uint64 ticks)
{
	SimState* simState = (SimState*) simMemory->bytes;

	InputMessage msg;
	while ( simMemory->input.count > 0 )
	{
		msg = GetInputMessage(&simMemory->input);
		switch ( msg )
		{
			case FocusGained:
				simState->isPaused = false;
				break;

			case FocusLost:
				simState->isPaused = true;
				break;

			case Quit:
				//TOOD: ???
				break;

			InvalidDefaultCase;
		}
	}

	AddTicks(&simState->realTime, ticks, simMemory->tickFrequency);
	if (!simState->isPaused)
		AddTicks(&simState->gameTime, ticks, simMemory->tickFrequency);

	//Update sim?
	//Render
}