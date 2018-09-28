#pragma once

class Crc32
{
public:
	static void Init();
	static void Dispose();
	static unsigned int Compute(unsigned char* data, int lenght);

	static unsigned int* table;
};