#pragma once

#include <cstdint>

class GameTimer final
{
public:
	GameTimer();

	double          Time() const;
	double      RealTime() const;
	double     DeltaTime() const;
	double RealDeltaTime() const;

	void Start();
	void Stop();
	void Tick();

private:
	bool isStopped;

	 double time;
	 uint64_t timeTicks;

	 double realTime;
	 uint64_t realTimeTicks;

	 double deltaTime;
	 uint64_t deltaTimeTicks;

	 double realDeltaTime;
	 uint64_t realDeltaTimeTicks;

	double secondsPerTick;

	uint64_t previousTick;
};