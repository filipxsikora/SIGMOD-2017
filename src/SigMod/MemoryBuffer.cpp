#include "MemoryBuffer.h"

MemoryBuffer::MemoryBuffer(uint memBlockSize, uint maxBlockCount, uint preallocateBlocks)
{
	mMemBlockSize = memBlockSize;
	mMaxBlockCount = maxBlockCount;
	mPreallocatedBlocks = preallocateBlocks;

	mMemory = new uchar*[mMaxBlockCount];

	for (int i = 0; i < mMaxBlockCount; ++i)
	{
		if (i < mPreallocatedBlocks || i == 0)
		{
			mMemory[i] = new uchar[memBlockSize];
			memset(mMemory[i], 0, mMemBlockSize);
		}
		else
		{
			mMemory[i] = nullptr;
		}
	}

	//mMemory[0] = new uchar[memBlockSize];
	memset(mMemory[0], 0, memBlockSize);
	mCurrentBlockOffset = 0;
	mBlockCount = 1;
}

MemoryBuffer::~MemoryBuffer()
{
	for (int i = 0; i < mBlockCount; ++i)
	{
		delete[] mMemory[i];
	}
	delete[] mMemory;
}

uchar * MemoryBuffer::GetMemory(uint memSize)
{
	std::unique_lock<std::mutex> locker(mMutex);

	if (memSize + mCurrentBlockOffset < mMemBlockSize)
	{
		uchar* ptr = mMemory[mBlockCount - 1] + mCurrentBlockOffset;
		mCurrentBlockOffset += memSize;

		return ptr;
	}
	else
	{
		if (memSize > mMemBlockSize)
		{
			printf("ERROR, cannot allocate memory");
			return nullptr;
		}
		if (mBlockCount == mMaxBlockCount - 1)
		{
			printf("ERROR, cannot allocate memory");
			return nullptr;
		}

		if (mBlockCount >= mPreallocatedBlocks)
		{
			mMemory[mBlockCount] = new uchar[mMemBlockSize];
			memset(mMemory[mBlockCount], 0, mMemBlockSize);
		}
		
		uchar* ptr = mMemory[mBlockCount];
		++mBlockCount;

		mCurrentBlockOffset = memSize;

		return ptr;
	}
}
