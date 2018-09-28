#pragma once

#include "DataBlock.h"
#include "Helper.h"
#include "MemoryBuffer.h"

#include <stdio.h>

template <class T> class HashTable
{
public:
	HashTable(MemoryBuffer* memBuffer, int avgItemSize, int itemCount, int blockSize);
	~HashTable();
	bool Insert(const T& item);
	bool Find(const T& item, bool searchDeleted = false);
	uchar* FindAndSetProcessed(const T& item);
	void Delete(const T& item);
	void Print();
private:
	void RebuildHashTable();
	unsigned int mSize;
	unsigned int mCount;
	unsigned int mMaxStringLength;
	unsigned int mBlockSize;
	unsigned int mItemCount;
	unsigned int mAvgItemSize;
	DataBlock<T>** mHashTable;
	DataBlock<T>** mFirstDataBlocks;
	int* mStringLengths;
	const int MAX_STRING_LENGTH = 2;
	MemoryBuffer* mMemBuffer;
};

template<class T>
inline HashTable<T>::HashTable(MemoryBuffer* memBuffer, int avgItemSize, int itemCount, int blockSize)
{
	mMemBuffer = memBuffer;
	mBlockSize = blockSize;
	mItemCount = itemCount;
	mAvgItemSize = avgItemSize;
	mSize = (int)(mItemCount / ((mBlockSize / avgItemSize) * 0.9));
	mHashTable = new DataBlock<T>*[mSize];
	mFirstDataBlocks = new DataBlock<T>*[mSize];
	mStringLengths = new int[mSize];

	for (unsigned int i = 0; i < mSize; ++i)
	{
		mHashTable[i] = nullptr;//new DataBlock<T>(mMemBuffer->GetMemory(mBlockSize), mBlockSize);
		mFirstDataBlocks[i] = nullptr;
		//mFirstDataBlocks[i] = mHashTable[i];
		mStringLengths[i] = 0;
	}
}

template<class T>
inline HashTable<T>::~HashTable()
{
	for (int i = 0; i < mSize; ++i)
	{
		if (mHashTable[i] == nullptr)
		{
			delete mHashTable[i];
		}
	}
	delete[] mHashTable;
	delete[] mFirstDataBlocks;
	delete[] mStringLengths;
}

template<class T>
inline bool HashTable<T>::Insert(const T& item)
{
	if (!Find(item, true))
	{
		unsigned int hash = item->Hash() % mSize;
		if (mHashTable[hash] == nullptr)
		{
			mFirstDataBlocks[hash] = mHashTable[hash] = new DataBlock<T>(mMemBuffer->GetMemory(mBlockSize), mBlockSize);
		}

		if (!mHashTable[hash]->Add(item))
		{
			if (mHashTable[hash] == mFirstDataBlocks[hash])
			{
				mHashTable[hash] = mHashTable[hash]->AppendNextBlock(mMemBuffer->GetMemory(mBlockSize), mBlockSize);
				mFirstDataBlocks[hash]->mNextBlock = mHashTable[hash];
			}
			else
			{
				mHashTable[hash] = mHashTable[hash]->AppendNextBlock(mMemBuffer->GetMemory(mBlockSize), mBlockSize);
			}

			mHashTable[hash]->Add(item);

			/*if (++mStringLengths[hash] > MAX_STRING_LENGTH)
			{
				RebuildHashTable();
			}*/
		}

		return true;
	}

	return false;
}

template<class T>
inline bool HashTable<T>::Find(const T& item, bool searchDeleted)
{
	unsigned int hash = item->Hash() % mSize;

	if (mFirstDataBlocks[hash] == nullptr)
	{
		return false;
	}

	return mFirstDataBlocks[hash]->Find(item, searchDeleted);
}

template<class T>
inline uchar * HashTable<T>::FindAndSetProcessed(const T & item)
{
	unsigned int hash = item->Hash() % mSize;

	if (mFirstDataBlocks[hash] == nullptr)
	{
		return nullptr;
	}

	return mFirstDataBlocks[hash]->FindSetProcessed(item);
}

template<class T>
inline void HashTable<T>::Delete(const T & item)
{
	unsigned int hash = item->Hash() % mSize;

	if (mFirstDataBlocks[hash] == nullptr)
	{
		return;
	}

	mFirstDataBlocks[hash]->Delete(item);
}

template<class T>
inline void HashTable<T>::Print()
{
	int blockCount;

	int itemsSum = 0;
	int itemsSize = 0;
	int itemsCount = 0;
	int blockSum = 0;
	int maxBlockLength = 0;
	int emptySlots = 0;
	int oneCount = 0;
	double avgBlockLength = 0;
	double avgUtilization = 0;

	for (unsigned int i = 0; i < mSize; i++)
	{
		blockCount = mFirstDataBlocks[i]->GetNoOfPages();
		itemsCount = mFirstDataBlocks[i]->GetNoOfItems();
		itemsSize += mFirstDataBlocks[i]->GetItemsSize();

		itemsSum += itemsCount;
		blockSum += blockCount;
		avgBlockLength += blockCount;

		if (blockCount > maxBlockLength)
		{
			maxBlockLength = blockCount;
		}

		if (itemsCount == 0)
		{
			emptySlots++;
		}

		if (blockCount == 1)
		{
			oneCount++;
		}

		//printf("Hash table slot %d has %d pages\n", i, mFirstDataBlocks[i]->GetNoOfPages());
	}

	double avgItemSize = (double)itemsSize / itemsSum;

	avgUtilization = ((double)itemsSum / ((blockSum * mBlockSize) / avgItemSize)) * 100;

	printf("Total items: %d\n", itemsSum);
	printf("Total blocks: %d\n", blockSum);
	printf("No of slots: %d\n", mSize);
	printf("Empty slots: %d\n", emptySlots);
	printf("One block slots: %d (%.2f %%)\n", oneCount, ((double)oneCount / blockSum) * 100);
	printf("Average utilization: %.2f%%\n", avgUtilization);
	printf("Max block length: %d\n", maxBlockLength);
	printf("Average block string length: %.2f\n", avgBlockLength / mSize);
	printf("Total size: %sB\n", Helper::NumToClosestOrderBytes(blockSum * mBlockSize).c_str());
}

template<class T>
inline void HashTable<T>::RebuildHashTable()
{
	mItemCount = mItemCount * 2;

	int newSize = (int)(mItemCount / ((mBlockSize / mAvgItemSize) * 0.9));

	DataBlock<T>** newHashTable = new DataBlock<T>*[newSize];
	DataBlock<T>** newFirstDataBlocks = new DataBlock<T>*[newSize];
	int* newStringLengths = new int[newSize];

	for (unsigned int i = 0; i < newSize; i++)
	{
		newHashTable[i] = new DataBlock<T>(mBlockSize);
		newFirstDataBlocks[i] = newHashTable[i];
		newStringLengths[i] = 1;
	}

	for (int i = 0; i < mSize; i++)
	{
		DataBlock<T>* ptr = mFirstDataBlocks[i];
		do
		{
			if (ptr->mOrder > 0)
			{
				for (int x = 0; x < ptr->mOrder - 9;)
				{
					if (ptr->mData[8 + x])
					{
						int hash = Term::Hash(ptr->mData + x) % newSize;
						///
						Term t = Term(&ptr->mData[9 + x], ptr->mData[8 + x]);
						t.SetId(Term::GetId(&ptr->mData[x]));
						if (!newHashTable[hash]->Add(t))
						{
							if (newHashTable[hash] == newFirstDataBlocks[hash])
							{
								newHashTable[hash] = newHashTable[hash]->AppendNextBlock();
								newFirstDataBlocks[hash]->mNextBlock = newHashTable[hash];
							}
							else
							{
								newHashTable[hash] = newHashTable[hash]->AppendNextBlock();
							}

							newHashTable[hash]->Add(t);
						}
						///
						x += 9 + ptr->mData[8 + x];
					}
				}
			}
			ptr = ptr->mNextBlock;
		} while (ptr != NULL);
	}

	for (unsigned int i = 0; i < mSize; i++)
	{
		delete mHashTable[i];
	}
	delete[] mHashTable;
	delete[] mFirstDataBlocks;
	delete[] mStringLengths;

	mSize = newSize;
	mHashTable = newHashTable;
	mFirstDataBlocks = newFirstDataBlocks;
	mStringLengths = newStringLengths;
}
