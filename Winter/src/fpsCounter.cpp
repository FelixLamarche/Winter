#include "fpsCounter.h"

#include <iostream>

FPSCounter::FPSCounter(float timeForCalcs)
	: frameTimes(), timeForCalcs(timeForCalcs)
{
}

void FPSCounter::update(float time)
{
	while (!frameTimes.empty() && frameTimes.front() < time - timeForCalcs)
	{
		frameTimes.pop();
	}
	frameTimes.push(time);
}

float FPSCounter::getFPS() const
{
	return static_cast<float>(frameTimes.size()) / timeForCalcs;
}

void FPSCounter::showFPS() const
{
	std::cout << "FPS: " << static_cast<int>(getFPS()) << std::endl;
}