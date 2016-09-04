struct SimState
{
	//r32  gameTime;
	//r32  realTime;
	bool isPaused;
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
InitializeSimulation(SimMemory *simMemory, V2i* windowSize)
{
	SimState* simState = (SimState*) simMemory->bytes;

	*windowSize = {800, 600};
}

void
UpdateSimulation(SimMemory *simMemory)
{
	SimState* simState = (SimState*) simMemory->bytes;

	//Update sim?
	//Render
}