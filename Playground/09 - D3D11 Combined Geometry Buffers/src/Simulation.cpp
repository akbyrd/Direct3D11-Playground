#include "Platform.h"
#include "Time.hpp"

struct SimState
{
	SimTime gameTime;
	SimTime realTime;
	bool    isPaused;
};

void
InitializeSimulation(SimMemory *simMemory)
{
	//NOTE: simMemory is expected to be initialized to zeros.

	SimState simState = {};

	simState.realTime.tickFrequency = simMemory->tickFrequency;
	simState.gameTime.tickFrequency = simMemory->tickFrequency;
}

void
UpdateSimulation(SimMemory *simMemory)
{
	//TODO: Remove
	SimState simState;

	InputMessage msg;
	while ( msg = simMemory->input.GetInputMessage() )
	{
		switch ( msg )
		{
			case FocusGained:
				simState.isPaused = false;
				break;

			case FocusLost:
				simState.isPaused = true;
				break;

			case Quit:
				//TOOD: ???
				break;

			default:
				//TODO: ???
				*((uint8 *) 0) = 0;
				break;
		}
	}

	simState.realTime.AddTicks(simMemory->ticks);
	if (!simState.isPaused)
		simState.gameTime.AddTicks(simMemory->ticks);

	//Update sim?
	//Render
}