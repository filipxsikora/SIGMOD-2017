#include "CombinedTrieNode.h"

CombinedTrieNode::CombinedTrieNode(MemoryBuffer * memBuffer, TrieNodeMode mode)
{
	mMode = mode;
	mData = memBuffer->GetMemory(256);
	mMemoryBuffer = memBuffer;

	if (mode == TrieNodeMode::Trie)
	{
		memset(mData, 1, 256);
		mNextNode = new CombinedTrieNode*[256];

		for (int i = 0; i < 256; i++)
		{
			mNextNode[i] = nullptr;
		}
	}
	else
	{
		mHashTable = new HashTable<HashTableItem*>(memBuffer, HASHTABLE_AVGITEMSIZE, HASHTABLE_ITEMCOUNT, HASHTABLE_BLOCKSIZE);
	}
}

CombinedTrieNode::~CombinedTrieNode()
{
	/*if (mMode == TrieNodeMode::Trie)
	{
		for (int i = 0; i < 256; i++)
		{
			if (mNextNode[i] != nullptr)
			{
				delete mNextNode[i];
			}
		}

		delete[] mNextNode;
	}
	else
	{
		delete mHashTable;
	}*/
}

CombinedTrieNode * CombinedTrieNode::AddNode(uint position, TrieNodeMode mode)
{
	if (mNextNode[position] == nullptr)
	{
		mNextNode[position] = new CombinedTrieNode(mMemoryBuffer, mode);
	}
	return mNextNode[position];
}
