#pragma once

#include <string>

class Helper
{
public:
	static std::string NumToClosestOrder(double num)
	{
		const char* prefix[] = { "", "k", "M", "G" };
		char buffer[256];

		int i = 0;

		while (num >= 1000 && i < 3)
		{
			i++;
			num /= 1000;
		}

		sprintf_s(buffer, "%.2f %s", num, prefix[i]);

		return std::string(buffer);
	}

	static std::string NumToClosestOrderBytes(double num)
	{
		const char* prefix[] = { "", "k", "M", "G" };
		char buffer[256];

		int i = 0;

		while (num >= 1024 && i < 3)
		{
			i++;
			num /= 1024;
		}

		sprintf_s(buffer, "%.2f %s", num, prefix[i]);

		return std::string(buffer);
	}
};