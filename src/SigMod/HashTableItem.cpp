#include "HashTableItem.h"



HashTableItem::HashTableItem(uchar* data, ushort length)
{
	mData = data;
	mLength = length;
}


HashTableItem::~HashTableItem()
{
}

uint HashTableItem::Hash()
{
	return Crc32::Compute(mData, mLength);
}

bool HashTableItem::Compare(uchar * data, ushort length)
{
	if (mLength == length)
	{
		return memcmp(data, mData, length) == 0;
	}
	return false;
}
