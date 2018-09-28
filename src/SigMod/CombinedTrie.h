#pragma once

#include <mutex>
#include <string>

#include "CombinedTrieNode.h"
#include "MemoryBuffer.h"
#include "base.h"

class CombinedTrie
{
public:
	CombinedTrie(MemoryBuffer* memBuffer);
	~CombinedTrie();
	void AddNgram(uchar* data, uint length);
	int FindNgram(uchar* data, uint length);
	void DeleteNgram(uchar* data, uint length);
	uint GetItemsCount();
	void ClearProcessedFlag();
	uint GetDepth() { return mDepth; }
private:
	uint mDepth;
	CombinedTrieNode* mRootNode;
	CombinedTrieNode* mLastNode;
	MemoryBuffer* mMemBuffer;
	std::mutex* mMutex;
	uchar** mProcessedFlags;
	uint mProccessedFlagsLength;
	const uchar INACTIVE_CHAR = 1;
	const uchar ACTIVE_CHAR = 2;
	const uchar END_OF_NGRAM = 4;
	const uchar ALREADY_PROCESSED = 8;
	const int TRIE_DEPTH_THRESHOLD = 60;
};