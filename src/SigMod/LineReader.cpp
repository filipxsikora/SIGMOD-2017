#include "LineReader.h"

LineReader::LineReader(char *filename, char* testFilename) {
	mBuffer = new char[BUFFER_SIZE];
	mLine = new char[LINE_SIZE];

	mBufferOffsetLo = 0;
	mBufferOffsetHi = 0;
	mBytesRead = 0;
	mLineOffset = 0;
	mLineCount = 0;

	mFile = new ifstream(filename);

	if (!mFile->is_open())
	{
		printf("Error: File Opening Failed!\n");
	}

	if (testFilename != NULL) {
		mTestFile = new ifstream(testFilename);
		if (!mTestFile->is_open())
		{
			printf("Error: Test File Opening Failed!\n");
			delete mTestFile;
			mTestFile = NULL;
		}
	}
	else
	{
		mTestFile = NULL;
	}
}

LineReader::~LineReader() {
	// std::cout << "Read Line Time: " << (mReadlineTime.count() / 1000) << "s.\n";

	if (mBuffer != NULL) {
		delete mBuffer;
		mBuffer = NULL;
	}
	if (mLine != NULL) {
		delete mLine;
		mLine = NULL;
	}

	if (mFile != NULL) {
		mFile->close();
		delete mFile;
		mFile = NULL;
	}
	if (mTestFile != NULL) {
		mTestFile->close();
		delete mTestFile;
		mTestFile = NULL;
	}
}

char* LineReader::GetLine(uint &lineLen) {
	uint strLen1;

	// auto t1 = high_resolution_clock::now();

	char* str1 = GetLineOwn(strLen1);
	// char* str1 = GetLineTest(strLen1);

	// auto t2 = high_resolution_clock::now();
	// mReadlineTime += t2 - t1;

	/*if (str1 != NULL && str2 != NULL)
	{
	// compare those strings
	if (strLen1 != strLen2) {
	printf("Error: The lengths of the strings are not the same!\n");
	}
	else {
	for (uint i = 0; i < strLen1; i++) {
	if (str1[i] != str2[i]) {
	printf("Error: The strings are not the same!\n");
	}
	}
	}
	}*/

	lineLen = strLen1;
	return str1;
}

void LineReader::Seek(uint position)
{
	mFile->clear();
	mFile->seekg(position, mFile->beg);
}

char* LineReader::GetLineTest(uint &lineLen) {
	char* str = NULL;

	if (getline(*mTestFile, mStringLine)) {
		str = (char*)(mStringLine.c_str());
		lineLen = mStringLine.length();
	}
	return str;
}

char* LineReader::GetLineOwn(uint &lineLen) {
	char* line = NULL;

	while (line == NULL) {
		if (mBufferOffsetHi >= mBytesRead) {
			if (mFile->eof()) {
				break;
			}

			// copy the rest of the buffer into the line
			if (mBufferOffsetHi == mBytesRead) {
				uint len = mBufferOffsetHi - mBufferOffsetLo;
				if (len != 0) {
					memcpy(mLine + mLineOffset, mBuffer + mBufferOffsetLo, len);
					mLineOffset += len;
				}
			}

			mFile->read(mBuffer, BUFFER_SIZE);
			mBytesRead = mFile->gcount();
			mBufferOffsetLo = mBufferOffsetHi = 0;
		}

		for (mBufferOffsetHi = mBufferOffsetLo; mBufferOffsetHi < mBytesRead; mBufferOffsetHi++)
		{
			if (mBuffer[mBufferOffsetHi] == '\n')
			{
				lineLen = mBufferOffsetHi - mBufferOffsetLo;
				memcpy(mLine + mLineOffset, mBuffer + mBufferOffsetLo, lineLen);

				mBufferOffsetLo += (lineLen + 1);
				mBufferOffsetHi++;

				lineLen += mLineOffset; // add the previously read string
				mLineOffset = 0;
				line = mLine;
				line[lineLen] = 0;
				break;
			}
		}
	}
	return line;
}


// while (getline(std::cin, line))
// while (getline(loadFile, line))
