#pragma once

#include "Platform.h"

void InitializeSimulation(void *simMemory, uint64 ticks);

void UpdateSimulation(uint64 ticks, InputQueue &inputQueue);