#pragma once

#define NOMINMAX

#include <fstream>
#include <Windows.h>

constexpr int BLOCK_SIZE = 32768; //Must be multiple of sizeof(T) // was 104

class File
{
public:
	File(std::wstring filename);
	~File();
	// returns the host of the blocks in the file
	int GetBlockCount();
	// saves a data in a specific block in the file
	void SaveBlock(int blockId, void* data);
	// reads a specific block of the file
	void ReadBlock(int blockId, void* dataOut);
private:
	HANDLE hFile;
	std::fstream file;
};

