#include "Stopwatch.h"

Stopwatch::Stopwatch()
{
	running = false;
}

Stopwatch::~Stopwatch()
{
}

void Stopwatch::Start()
{
	start = std::chrono::steady_clock::now();
	running = true;
}

void Stopwatch::Stop()
{
	end = std::chrono::steady_clock::now();
	running = false;
}

void Stopwatch::Restart()
{
	Start();
}

long long Stopwatch::ElapsedMilliseconds()
{
	if (running)
	{
		auto duration = std::chrono::duration_cast<std::chrono::microseconds>
			(std::chrono::steady_clock::now() - start);
		return duration.count();
	}
	else
	{
		auto duration = std::chrono::duration_cast<std::chrono::microseconds>
			(end - start);
		return duration.count();
	}
}
