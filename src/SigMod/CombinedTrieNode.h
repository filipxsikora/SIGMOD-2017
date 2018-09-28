#pragma once

#include "MemoryBuffer.h"
#include "HashTable.h"
#include "HashTableItem.h"

enum class TrieNodeMode { Trie, HashTable };


class CombinedTrieNode
{
public:
	CombinedTrieNode(MemoryBuffer* memBuffer, TrieNodeMode mode);
	~CombinedTrieNode();
	CombinedTrieNode* AddNode(uint position, TrieNodeMode mode);
	inline CombinedTrieNode* GetNextNode(uint position) { return mNextNode[position]; }
	HashTable<HashTableItem*>* GetHashTable() { return mHashTable; }
	inline uchar* GetData() { return mData; }
private:
	uchar* mData;
	CombinedTrieNode** mNextNode;
	TrieNodeMode mMode;
	HashTable<HashTableItem*>* mHashTable;
	MemoryBuffer* mMemoryBuffer;
	const int HASHTABLE_AVGITEMSIZE = 256;
	const int HASHTABLE_ITEMCOUNT = 32;
	const int HASHTABLE_BLOCKSIZE = 512;
};