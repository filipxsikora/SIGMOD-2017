#pragma once

#include "base.h"
#include <string>
#include <mutex>
#include <atomic>
#include "HashTable.h"
#include "MemoryBuffer.h"

struct ResultBufferItem
{
public:
	ResultBufferItem() {}
	ResultBufferItem(uchar* ptr, uint position)
	{
		mPtr = ptr;
		mPosition = position;
	}
	uchar* mPtr;
	uint mPosition;
};

class ResultBuffer
{
public:
	ResultBuffer();
	~ResultBuffer();
	void Add(uchar* data, ushort length, uint position);
	void Print();
	bool NotAlreadyProcessed(uint index, uchar* ptr);
private:
	uchar* mData;
	uint mDataPosition;
	uint mNoOfItems;
	ResultBufferItem mAlreadyProcessed[1000];
	uint mAlreadyProcessedLength;
	std::mutex mMutex;
};

