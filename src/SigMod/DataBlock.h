#pragma once

#include <string>

#include "base.h"

template <class T> class DataBlock
{
public:
	DataBlock(uchar* memory, uint size);
	~DataBlock();
	bool Add(const T& item);
	bool Find(const T& item, bool searchDeleted);
	uchar* FindSetProcessed(const T& item);
	void Delete(const T& item);
	DataBlock<T>* AppendNextBlock(uchar* memory, uint size);
	DataBlock<T>* mNextBlock;
	unsigned char* mData;
	int GetNoOfItems();
	int GetNoOfPages();
private:
	unsigned int mOrder;
	unsigned int mCapacity;
};

template<class T>
DataBlock<T>::DataBlock(uchar* memory, uint size)
{
	mNextBlock = NULL;
	mData = memory;
	mOrder = 0;
	mCapacity = size;
}

template<class T>
DataBlock<T>::~DataBlock()
{

}

template<class T>
inline bool DataBlock<T>::Add(const T& item)
{
	ushort size = (ushort)item->GetSize();

	if (mOrder + size + 3 < mCapacity)
	{
		memcpy(mData + mOrder, &size, 2);
		mData[mOrder + 2] = 1;
		memcpy(mData + mOrder + 3, item->GetData(), size);
		mOrder += size + 3;
		return true;
	}

	return false;
}

template<class T>
inline bool DataBlock<T>::Find(const T& item, bool searchDeleted)
{
	DataBlock<T>* ptr = this;
	do
	{
		for (unsigned int i = 0; i < ptr->mOrder;)
		{
			ushort length = *((ushort*)(&ptr->mData[i]));

			if (length == item->GetSize())
			{
				if (ptr->mData[i + 2] || searchDeleted)
				{
					if (item->Compare(ptr->mData + i + 3, length))
					{
						ptr->mData[i + 2] = 1;
						return true;
					}
				}
			}

			i += length + 3;
		}

		ptr = ptr->mNextBlock;
	} while (ptr != NULL);

	return false;
}

template<class T>
inline uchar * DataBlock<T>::FindSetProcessed(const T & item)
{
	DataBlock<T>* ptr = this;
	do
	{
		for (unsigned int i = 0; i < ptr->mOrder;)
		{
			ushort length = *((ushort*)(&ptr->mData[i]));

			if (length == item->GetSize())
			{
				if ((ptr->mData[i + 2] & 0x09) == 0x01)
				{
					if (item->Compare(ptr->mData + i + 3, length))
					{
						//ptr->mData[i + 2] |= 8;
						return &ptr->mData[i + 2];
					}
				}
			}

			i += length + 3;
		}

		ptr = ptr->mNextBlock;
	} while (ptr != NULL);

	return nullptr;
}

template<class T>
inline void DataBlock<T>::Delete(const T & item)
{
	DataBlock<T>* ptr = this;
	do
	{
		for (unsigned int i = 0; i < ptr->mOrder;)
		{
			ushort length = *((ushort*)(&ptr->mData[i]));

			if (length == item->GetSize())
			{
				if (ptr->mData[i + 2])
				{
					if (item->Compare(ptr->mData + i + 3, length))
					{
						ptr->mData[i + 2] = 0;
						return;
					}
				}
			}

			i += length + 3;
		}

		ptr = ptr->mNextBlock;
	} while (ptr != NULL);
}

template<class T>
inline DataBlock<T>* DataBlock<T>::AppendNextBlock(uchar* memory, uint size)
{
	mNextBlock = new DataBlock<T>(memory, size);
	return mNextBlock;
}

template<class T>
inline int DataBlock<T>::GetNoOfPages()
{
	int pages = 0;
	DataBlock<T>* ptr = this;
	do
	{
		++pages;
		ptr = ptr->mNextBlock;
	} while (ptr != NULL);

	return pages;
}
