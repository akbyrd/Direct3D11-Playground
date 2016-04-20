#include "Time.hpp"

struct SimState
{
	SimTime gameTime;
	SimTime realTime;
	bool    isPaused;
};

void
InitializeSimulation(void *simMemory, uint64 tickFrequency)
{
	//NOTE: simMemory is expected to be initialized to zeros.

	SimState simState = {};

	simState.realTime.tickFrequency = tickFrequency;
	simState.gameTime.tickFrequency = tickFrequency;
}

void
UpdateSimulation(uint64 ticks, InputQueue &inputQueue)
{
	//TODO: Remove
	SimState simState;

	InputMessage msg;
	while ( msg = inputQueue.GetInputMessage() )
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

	simState.realTime.AddTicks(ticks);
	if (!simState.isPaused)
		simState.gameTime.AddTicks(ticks);

	//Update sim?
	//Render
}