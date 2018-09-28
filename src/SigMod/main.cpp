#include "base.h"
#include "LineReader.h"
#include "Stopwatch.h"
#include "Helper.h"
#include "MemoryBuffer.h"
#include "main.h"
#include "CRC32.h"
#include "ResultBuffer.h"

#include <thread>
#include <queue>
#include <mutex>
#include <atomic>
#include <condition_variable>
#include <Windows.h>

#define THREADS
//#define NO_THREADS

#define FROM_FILE
//#define FROM_STDIN

#ifdef THREADS
const int NO_OF_THREADS = 4;
const int NO_OF_ADD_DEL_THREADS = 6;
#endif

#ifdef NO_THREADS
const int NO_OF_THREADS = 1;
#endif

uint* gSpaceIndexes;
atomic_bool gFinish = 0;
std::queue<QueueItem*>* gQueue;
std::mutex gQueueMutex;
std::mutex gCurrentItemMutex;
condition_variable gAddDelConditionVar;
condition_variable gQueryConditionVar;
#ifdef THREADS
std::thread gQueryThreads[NO_OF_THREADS - 1];
std::thread gAddDelThread[NO_OF_ADD_DEL_THREADS];
#endif
ResultBuffer* gResultBuffer;
volatile atomic_uint gAddDelToProcess = 0;
volatile QueueItem* gCurrentQuery = nullptr;
volatile uint gSpacesFound = 0;
volatile atomic_uint gQueryRun;

void QueryFunc(CombinedTrie* trie, uint index)
{
	uint offset = (1 << index);

	while (!gFinish)
	{
		std::unique_lock<std::mutex> locker(gCurrentItemMutex);

		gQueryConditionVar.wait(locker, [&]()
		{
			return gFinish || gCurrentQuery != nullptr;
		});

		if (gCurrentQuery != nullptr && (gQueryRun & offset))
		{
			locker.unlock();
			QueryNGram(trie, gCurrentQuery->mLine, gCurrentQuery->mLineLength, gSpacesFound, index);
			{
				gQueryRun &= ~offset;
			}
		}
	}
}

void AddDelThreadFunc(CombinedTrie* trie)
{
	QueueItem* item;

	while (!gFinish || !gQueue->empty())
	{
		std::unique_lock<std::mutex> locker(gQueueMutex);

		if (gAddDelToProcess == 0)
		{
			gAddDelConditionVar.wait(locker, [&]()
			{
				return gFinish || !gQueue->empty();
			});
		}

		if (!gQueue->empty())
		{
			item = gQueue->front();
			gQueue->pop();
		}
		else
		{
			item = nullptr;
		}

		locker.unlock();

		if (item != nullptr)
		{

			switch (item->mType)
			{
			case QueueItem::TYPE_ADD:
				trie->AddNgram((uchar*)item->mLine, item->mLineLength);
				gAddDelToProcess--;
				delete item;
				break;
			case QueueItem::TYPE_DELETE:
				trie->DeleteNgram((uchar*)item->mLine, item->mLineLength);
				gAddDelToProcess--;
				delete item;
				break;
			}
		}
	}
}

void QueryNGram(CombinedTrie* trie, char* line, uint lineLength, uint foundSpaces, uint index)
{
	uint threadWorkLen = foundSpaces / NO_OF_THREADS;
	uint threadWorkOffset = index * threadWorkLen;
	uint threadWorkEnd = threadWorkOffset + threadWorkLen;

	if (index == NO_OF_THREADS - 1)
	{
		threadWorkEnd = foundSpaces + 1;
	}

	for (int i = threadWorkOffset; i < threadWorkEnd; ++i)
	{
		int minus = ((i == foundSpaces + 1) ? 0 : 1);

		for (int y = i + 1; y < foundSpaces + 1; ++y)
		{
			uint length = gSpaceIndexes[y] - gSpaceIndexes[i] - minus;

			int result = trie->FindNgram((uchar*)line + gSpaceIndexes[i], length);
			if (result > 0)
			{
				if (result == 2)
				{
					gResultBuffer->Add((uchar*)line + gSpaceIndexes[i], length, gSpaceIndexes[i]);
				}
			}
			else
			{
				break;
			}
		}
	}
}

int main()
{
	//SetPriorityClass(GetCurrentProcess(), REALTIME_PRIORITY_CLASS);
	if (!SetPriorityClass(GetCurrentProcess(), REALTIME_PRIORITY_CLASS))
	{
		printf("Cannot set RT class\n");
	}
	if (!SetThreadPriority(GetCurrentThread(), THREAD_PRIORITY_TIME_CRITICAL))
	{
		printf("Cannot set RT global\n");
	}
	Crc32::Init();
	MemoryBuffer* buffer = new MemoryBuffer(512 * 1024, 1000, 10);
	MemoryBuffer* trieBuffer = new MemoryBuffer(1024 * 1024, 20000, 1500);
	CombinedTrie* trie = new CombinedTrie(trieBuffer);
	LineReader* lineReader = new LineReader("large_small.complete");
	Stopwatch* sw = new Stopwatch();
	gQueue = new std::queue<QueueItem*>();
	gResultBuffer = new ResultBuffer();

	gSpaceIndexes = (uint*)buffer->GetMemory(sizeof(uint) * 100000);
	memset(gSpaceIndexes, 0, sizeof(uint) * 100000);

	string sLine;
	char* line;
	uint lineLength;

#ifdef THREADS
	for (int i = 0; i < NO_OF_ADD_DEL_THREADS; ++i)
	{
		gAddDelThread[i] = std::thread(AddDelThreadFunc, trie);
		if (!SetThreadPriority(gAddDelThread[i].native_handle(), THREAD_PRIORITY_TIME_CRITICAL))
		{
			printf("Cannot set RT AddDel\n");
		}
	}
#endif

	sw->Start();
#ifdef FROM_FILE
	while ((line = lineReader->GetLine(lineLength)) != NULL)
	{
#endif
#ifdef FROM_STDIN
	while (getline(std::cin, sLine))
	{
		lineLength = sLine.length();
		line = (char*)sLine.c_str();
#endif

		if (lineLength == 1 && line[0] == 'S')
		{
			break;
		}
#ifdef THREADS
		{
			unique_lock<std::mutex> locker(gQueueMutex);
			gQueue->push(new QueueItem(line, lineLength, QueueItem::TYPE_ADD));
			gAddDelToProcess++;
			locker.unlock();
			gAddDelConditionVar.notify_one();
		}
#endif
#ifdef NO_THREADS
		trie->AddNgram((uchar*)line, lineLength);
#endif
	}
	while (gAddDelToProcess != 0);
	sw->Stop();

	printf("Initial insert took %.1f ms\n", sw->ElapsedMilliseconds() / 1000.0);

	//uint index = 0;
	//uint readedLines = 0;
#ifdef FROM_FILE
	freopen("out.txt", "w", stdout);
#endif
#ifdef THREADS
	for (int i = 0; i < NO_OF_THREADS - 1; ++i)
	{
		gQueryThreads[i] = std::thread(QueryFunc, trie, i);
		SetThreadPriority(gQueryThreads[i].native_handle(), THREAD_PRIORITY_TIME_CRITICAL);
	}
#endif

	sw->Restart();

#ifdef FROM_FILE
	while ((line = lineReader->GetLine(lineLength)) != NULL)
	{
#endif
#ifdef FROM_STDIN
	while (getline(std::cin, sLine))
	{
		lineLength = sLine.length();
		line = (char*)sLine.c_str();
#endif
#ifdef THREADS
		switch (line[0])
		{
		case 'A':
		{
			unique_lock<std::mutex> locker(gQueueMutex);
			gQueue->push(new QueueItem(line + 2, lineLength - 2, QueueItem::TYPE_ADD));
			gAddDelToProcess++;
			locker.unlock();
			gAddDelConditionVar.notify_one();
			break;
		}
		break;
		case 'D':
		{
			unique_lock<std::mutex> locker(gQueueMutex);
			gQueue->push(new QueueItem(line + 2, lineLength - 2, QueueItem::TYPE_DELETE));
			gAddDelToProcess++;
			locker.unlock();
			gAddDelConditionVar.notify_one();
			break;
		}
		break;
		case 'Q':
		{
			while (gAddDelToProcess != 0);
			gQueryRun = 0;
			for (int i = 0; i < NO_OF_THREADS - 1; ++i)
			{
				gQueryRun |= (1 << i);
			}
			gSpacesFound = 0;

			for (int i = 0; i < lineLength - 2; ++i)
			{
				if (line[i + 2] == ' ')
				{
					gSpaceIndexes[++gSpacesFound] = i + 1;
				}
			}

			gSpaceIndexes[++gSpacesFound] = lineLength - 1;

			std::unique_lock<std::mutex> locker(gCurrentItemMutex);
			gCurrentQuery = new QueueItem(line + 2, lineLength - 2, QueueItem::TYPE_QUERY);
			locker.unlock();
			gQueryConditionVar.notify_all();
			QueryNGram(trie, line + 2, lineLength - 1, gSpacesFound, 3);

			while (gQueryRun != 0);

			delete gCurrentQuery;
			gCurrentQuery = nullptr;
			gResultBuffer->Print();
		}
		break;
		case 'F':
			//gRun = false;
			break;
		}
#endif
#ifdef NO_THREADS
		switch (line[0])
		{
		case 'A':
			trie->AddNgram((uchar*)line + 2, lineLength - 2);
			break;
		case 'D':
			trie->DeleteNgram((uchar*)line + 2, lineLength - 2);
			break;
		case 'Q':
			gSpacesFound = 0;

			for (int i = 0; i < lineLength - 2; ++i)
			{
				if (line[i + 2] == ' ')
				{
					gSpaceIndexes[++gSpacesFound] = i + 1;
				}
			}

			gSpaceIndexes[++gSpacesFound] = lineLength - 1;

			QueryNGram(trie, line + 2, lineLength - 2, gSpacesFound, 0);
			gResultBuffer->Print();
			break;
		case 'F':
			//gRun = false;
			break;
		}
#endif
		}
#ifdef THREADS
	gFinish = true;

	gAddDelConditionVar.notify_all();
	gQueryConditionVar.notify_all();

	for (int i = 0; i < NO_OF_ADD_DEL_THREADS; ++i)
	{
		gAddDelThread[i].join();
	}
	for (int i = 0; i < NO_OF_THREADS - 1; ++i)
	{
		gQueryThreads[i].join();
	}
#endif
	sw->Stop();

#ifdef FROM_FILE
	freopen("CON", "w", stdout);
#endif

	printf("\n\nprocessing took: %.1f ms\n", sw->ElapsedMilliseconds() / 1000.0);
	printf("trie depth: %d\n", trie->GetDepth());
	printf("block count: %d", trieBuffer->GetBlocks());
	//getchar();
	delete trie;
	delete buffer;
	delete gResultBuffer;
	delete trieBuffer;
	delete gQueue;
	Crc32::Dispose();
	//delete[] gSpaceIndexes;
	}

QueueItem::QueueItem(char * line, uint lineLength, uint type)
{
	mLine = new char[lineLength];
	mLineLength = lineLength;
	mType = type;

	memcpy(mLine, line, lineLength);
}

QueueItem::~QueueItem()
{
	if (mLine != nullptr)
	{
		delete[] mLine;
		mLine = nullptr;
	}
}
