#pragma once
#include "base.h"
#include <stdio.h>
#include <string>
#include <mutex>

class MemoryBuffer
{
public:
	MemoryBuffer(uint memBlockSize, uint maxBlockCount, uint preallocateBlocks);
	~MemoryBuffer();
	uchar* GetMemory(uint memSize);
	uint GetBlocks() { return mBlockCount; }

private:
	uchar** mMemory;
	uint mBlockCount;
	uint mMaxBlockCount;
	uint mMemBlockSize;
	uint mCurrentBlockOffset;
	uint mPreallocatedBlocks;
	std::mutex mMutex;
};