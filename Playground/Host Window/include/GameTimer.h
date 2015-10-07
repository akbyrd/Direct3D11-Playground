#pragma once

class GameTimer final
{
public:
	GameTimer();

	double          Time() const;
	double      RealTime() const;
	double     DeltaTime() const;
	double RealDeltaTime() const;

	void Reset();
	void Start();
	void Stop();
	void Tick();

private:
	bool isStopped;

	 double time;
	__int64 timeTicks;

	 double realTime;
	__int64 realTimeTicks;

	 double deltaTime;
	__int64 deltaTimeTicks;

	 double realDeltaTime;
	__int64 realDeltaTimeTicks;

	double secondsPerTick;

	__int64    startTick;
	__int64 previousTick;
};