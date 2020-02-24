#pragma once

#include <fstream>
#include <Windows.h>

constexpr int BLOCK_SIZE = 512; //Must be multiple of sizeof(T)

class Data
{
public:
	Data();
	~Data();
	int GetBlockCount();
	void SaveBlock(int blockId, void* data);
	void ReadBlock(int blockId, void* dataOut);
private:
	HANDLE hFile;
	std::fstream file;
};

