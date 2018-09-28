#include "Crc32.h"

unsigned int* Crc32::table;

void Crc32::Init()
{
	table = new unsigned int[256];

	for (int i = 0; i < 256; i++)
	{
		unsigned int entry = i;
		for (int j = 0; j < 8; j++)
			if ((entry & 1) == 1)
			{
				entry = (entry >> 1) ^ 0xedb88320u;
			}
			else
			{
				entry = entry >> 1;
			}
		table[i] = entry;
	}
}

void Crc32::Dispose()
{
	delete[] table;
}

unsigned int Crc32::Compute(unsigned char * data, int lenght)
{
	unsigned int hash = 0xffffffffu;
	for (int i = 0; i < lenght; i++)
		hash = (hash >> 8) ^ table[data[i] ^ hash & 0xff];
	return hash;
}
