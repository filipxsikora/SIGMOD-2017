#pragma once

#include "base.h"
#include "CRC32.h"

#include <string>

class HashTableItem
{
public:
	HashTableItem(uchar* data, ushort length);
	~HashTableItem();
	uint Hash();
	bool Compare(uchar* data, ushort length);
	inline ushort GetSize() { return mLength; }
	inline uchar* GetData() { return mData; }
private:
	uchar* mData;
	ushort mLength;
};

