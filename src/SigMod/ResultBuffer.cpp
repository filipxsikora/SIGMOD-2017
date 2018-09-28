#include "ResultBuffer.h"

ResultBuffer::ResultBuffer()
{
	mData = new uchar[1024 * 1024 * 10];
	mAlreadyProcessedLength = 0;
	mDataPosition = 0;
	mNoOfItems = 0;
}

ResultBuffer::~ResultBuffer()
{
	delete[] mData;
}

void ResultBuffer::Add(uchar * data, ushort length, uint position)
{
	std::unique_lock<std::mutex> locker(mMutex);
	int offset = 0;
	for (int i = 0; i < mNoOfItems; ++i)
	{
		ushort len = *((ushort*)(mData + offset + 4));
		if (length == len)
		{
			if (memcmp(data, mData + offset + 6/*SPRAVNE!!!*/, len) == 0)
			{
				uint pos = *((uint*)(mData + offset));
				if (position < pos)
				{
					memcpy(mData + offset, &position, 4);
				}
				return;
			}
		}

		offset += len + 7;
	}

	memcpy(mData + mDataPosition, &position, 4);
	memcpy(mData + mDataPosition + 4, &length, 2);
	memcpy(mData + mDataPosition + 6, data, length);
	mData[mDataPosition + 6 + length] = 0;
	mDataPosition += 7 + length;
	mNoOfItems++;
}

void ResultBuffer::Print()
{
	bool first = true;

	if (mNoOfItems == 0)
	{
		printf("-1\n");
		mNoOfItems = 0;
		mDataPosition = 0;
		mAlreadyProcessedLength = 0;
		return;
	}

	if (mNoOfItems == 1)
	{
		printf((char*)mData + 6);
		printf("\n");
		mNoOfItems = 0;
		mDataPosition = 0;
		mAlreadyProcessedLength = 0;
		return;
	}

	for (int i = 0; i < mNoOfItems; ++i)
	{
		uint low = UINT_MAX;
		uchar* ptr;
		bool found = false;
		int offset = 0;
		for (int i = 0; i < mNoOfItems; ++i)
		{
			ushort len = *((ushort*)(mData + offset + 4));
			uint pos = *((uint*)(mData + offset));

			if (pos < low && NotAlreadyProcessed(pos, mData + offset + 6))
			{
				found = true;
				low = pos;
				ptr = mData + offset + 6;
			}

			offset += len + 7;
		}
		if (found)
		{
			mAlreadyProcessed[mAlreadyProcessedLength++] = ResultBufferItem(ptr, low);

			if (first)
			{
				printf((char*)ptr);
				first = false;
			}
			else
			{
				printf("|");
				printf((char*)ptr);
			}
		}
	}

	printf("\n");

	mNoOfItems = 0;
	mDataPosition = 0;
	mAlreadyProcessedLength = 0;
}

bool ResultBuffer::NotAlreadyProcessed(uint index, uchar* ptr)
{
	for (int i = 0; i < mAlreadyProcessedLength; ++i)
	{
		if (index == mAlreadyProcessed[i].mPosition && ptr == mAlreadyProcessed[i].mPtr)
		{
			return false;
		}
	}

	return true;
}
