#include "CombinedTrie.h"

CombinedTrie::CombinedTrie(MemoryBuffer* memBuffer)
{
	mMemBuffer = memBuffer;
	mDepth = 1;
	mLastNode = mRootNode = new CombinedTrieNode(mMemBuffer, TrieNodeMode::Trie);
	mMutex = new std::mutex[256];
	mProcessedFlags = new uchar*[10000];
	mProccessedFlagsLength = 0;
}


CombinedTrie::~CombinedTrie()
{
	delete[] mMutex;
	delete[] mProcessedFlags;
	delete mRootNode;
}

void CombinedTrie::AddNgram(uchar * data, uint length)
{
	std::unique_lock<std::mutex> lock(mMutex[data[0]]);
	CombinedTrieNode* node = mRootNode;
	uchar* nodeData;

	if (length > mDepth)
	{
		mDepth = length;
	}

	for (int i = 0; i < length - 1; i++)
	{
		if (i == TRIE_DEPTH_THRESHOLD)
		{
			node = node->AddNode(data[i], TrieNodeMode::HashTable);

			auto hashTable = node->GetHashTable();
			HashTableItem item(&data[TRIE_DEPTH_THRESHOLD], length - TRIE_DEPTH_THRESHOLD);

			hashTable->Insert(&item);
			return;
		}

		nodeData = &node->GetData()[data[i]];
		*nodeData &= ~(INACTIVE_CHAR);
		*nodeData |= ACTIVE_CHAR;
		node = node->AddNode(data[i], TrieNodeMode::Trie);
	}

	nodeData = &node->GetData()[data[length - 1]];
	*nodeData &= ~(INACTIVE_CHAR);
	*nodeData |= (ACTIVE_CHAR | END_OF_NGRAM);
}

int CombinedTrie::FindNgram(uchar * data, uint length)
{
	if (length > mDepth)
	{
		return 0;
	}

	CombinedTrieNode* node = mRootNode;
	uchar* nodeData = node->GetData();

	for (int i = 0; i < length - 1; i++)
	{
		if (i == TRIE_DEPTH_THRESHOLD)
		{
			node = node->GetNextNode(data[i]);
			if (node == nullptr)
			{
				return 0;
			}
			int targetLen = length - TRIE_DEPTH_THRESHOLD;
			auto hashTable = node->GetHashTable();
			HashTableItem item(&data[TRIE_DEPTH_THRESHOLD], targetLen);

			uchar* ptr1 = hashTable->FindAndSetProcessed(&item);
			if (ptr1 != nullptr)
			{
				//mProcessedFlags[mProccessedFlagsLength++] = ptr1;
				return 2;
			}
			
			return 1;
		}

		if (nodeData[data[i]] & INACTIVE_CHAR)
		{
			return 0;
		}

		node = node->GetNextNode(data[i]);
		if (node == nullptr)
		{
			return 0;
		}
		nodeData = node->GetData();
	}

	uchar* ptr = &nodeData[data[length - 1]];

	if (*ptr & END_OF_NGRAM/* && !(*ptr & ALREADY_PROCESSED)*/)
	{
		//mProcessedFlags[mProccessedFlagsLength++] = ptr;
		//*ptr |= ALREADY_PROCESSED;
		return 2;
	}

	return 1;
}

void CombinedTrie::DeleteNgram(uchar * data, uint length)
{
	std::unique_lock<std::mutex> lock(mMutex[data[0]]);
	if (length > mDepth)
	{
		return;
	}

	CombinedTrieNode* node = mRootNode;
	uchar* nodeData = node->GetData();

	for (int i = 0; i < length - 1; i++)
	{
		if (i == TRIE_DEPTH_THRESHOLD)
		{
			node = node->GetNextNode(data[i]);
			if (node == nullptr)
			{
				return;
			}
			int targetLen = length - TRIE_DEPTH_THRESHOLD;
			auto hashTable = node->GetHashTable();
			HashTableItem item(&data[TRIE_DEPTH_THRESHOLD], targetLen);

			hashTable->Delete(&item);
			return;
		}

		if (i == TRIE_DEPTH_THRESHOLD - 1)
		{
			printf("");
		}

		if (nodeData[data[i]] & INACTIVE_CHAR)
		{
			return;
		}

		node = node->GetNextNode(data[i]);
		if (node == nullptr)
		{
			return;
		}
		nodeData = node->GetData();
	}

	node->GetData()[data[length - 1]] &= ~(END_OF_NGRAM);
}

void CombinedTrie::ClearProcessedFlag()
{
	for (int i = 0; i < mProccessedFlagsLength; ++i)
	{
		*mProcessedFlags[i] &= 0xF7;
	}
	mProccessedFlagsLength = 0;
}
