#pragma once

#include <iostream>
#include <chrono>

class Stopwatch
{
public:
	Stopwatch();
	~Stopwatch();
	void Start();
	void Stop();
	void Restart();
	long long ElapsedMilliseconds();

private:
	std::chrono::time_point<std::chrono::steady_clock> start;
	std::chrono::time_point<std::chrono::steady_clock> end;
	bool running;
};