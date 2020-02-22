#pragma once

#include <fstream>

constexpr int BLOCK_SIZE = 24; //TODO check padding

class Data
{
public:
	Data();
	~Data();
	int GetBlockCount();
	void SaveBlock(int blockId, void* data);
	void ReadBlock(int blockId, void* dataOut);
private:
	std::fstream file;
};

