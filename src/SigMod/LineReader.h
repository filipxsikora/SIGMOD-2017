#pragma once

#include <iostream>
#include <fstream>
#include <string>
#include <cstring>
#include <chrono>

#include "base.h"

using namespace std;
using namespace std::chrono;

class LineReader {
	char* mBuffer;
	uint mBufferOffsetLo;
	uint mBufferOffsetHi;
	std::streamsize mBytesRead;

	char* mLine;
	uint mLineOffset;

	uint mLineCount;

	ifstream *mFile;
	ifstream *mTestFile;
	std::string mStringLine;

	std::chrono::duration<double, std::milli> mReadlineTime;

	static const uint BUFFER_SIZE = 500000;
	static const uint LINE_SIZE = 1000000;

	char* GetLineOwn(uint &lineLen);
	char* GetLineTest(uint &lineLen);

public:
	LineReader(char* filename, char* testFilename = NULL);
	~LineReader();

	char* GetLine(uint &lineLen);
	void Seek(uint position);
};
