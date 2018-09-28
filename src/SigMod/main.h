#pragma once

#include "CombinedTrie.h"

enum class CurrentProcessing { None, Queries, AddDel };

struct QueueItem
{
public:
	QueueItem(char* line, uint lineLength, uint type);
	~QueueItem();
	char* mLine;
	uint mLineLength;
	uint mType;
	static const uint TYPE_QUERY = 1;
	static const uint TYPE_ADD = 2;
	static const uint TYPE_DELETE = 3;
};

void QueryNGram(CombinedTrie* trie, char* line, uint lineLength, uint foundSpaces, uint index);