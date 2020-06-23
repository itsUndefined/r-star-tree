#pragma once

#define NOMINMAX

#include <fstream>
#include <Windows.h>

constexpr int BLOCK_SIZE = 32768; //Must be multiple of sizeof(T)

class File
{
public:
	File(std::wstring filename);
	~File();
	int GetBlockCount();
	void SaveBlock(int blockId, void* data);
	void ReadBlock(int blockId, void* dataOut);
private:
	HANDLE hFile;
	std::fstream file;
};

