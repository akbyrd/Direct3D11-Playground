#include "stdafx.h"
#include "GameTimer.h"

double GameTimer::Time()          const { return time;          }
double GameTimer::RealTime()      const { return realTime;      }
double GameTimer::DeltaTime()     const { return deltaTime;     }
double GameTimer::RealDeltaTime() const { return realDeltaTime; }

GameTimer::GameTimer()
{
	isStopped = true;

	           time      = 0;
	           timeTicks = 0;
	       realTime      = 0;
	       realTimeTicks = 0;
	      deltaTime      = 0;
	      deltaTimeTicks = 0;
	  realDeltaTime      = 0;
	  realDeltaTimeTicks = 0;

	//Initialize frequency info
	__int64 ticksPerSecond;
	QueryPerformanceFrequency((LARGE_INTEGER*) &ticksPerSecond);

	secondsPerTick = 1. / ticksPerSecond;

	//Initialize the timer
	QueryPerformanceCounter((LARGE_INTEGER*) &previousTick);
	isStopped = true;
}

void GameTimer::Start()
{
	Tick();
	isStopped = false;
}

void GameTimer::Stop()
{
	Tick();
	isStopped = true;
}

void GameTimer::Tick()
{
	__int64 currentTick;
	QueryPerformanceCounter((LARGE_INTEGER*) &currentTick);

	__int64 newTicks = currentTick - previousTick;
	previousTick = currentTick;

	/* This can happen after entering a power saving mode or if
	 * the thread is shuffled to another processor.
	 */
	if ( newTicks < 0 ) { return; }

	//Update real time
	realTimeTicks += newTicks;
	realTime = realTimeTicks * secondsPerTick;

	//Update real delta time
	realDeltaTimeTicks = newTicks;
	realDeltaTime = realDeltaTimeTicks * secondsPerTick;

	if ( !isStopped )
	{
		//Update time
		timeTicks += newTicks;
		time = timeTicks * secondsPerTick;

		//Update delta time
		deltaTimeTicks = newTicks;
		deltaTime = deltaTimeTicks * secondsPerTick;
	}
	else
	{
		//No delta time while stopped
		deltaTime = 0;
	}
}