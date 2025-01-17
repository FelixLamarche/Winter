#pragma once
#include <queue>

class FPSCounter
{
public:
	FPSCounter(float timeForCalcs);
	void update(float time);
	float getFPS() const;
	void showFPS() const;
private:
	std::queue<float> frameTimes;
	float timeForCalcs;
};