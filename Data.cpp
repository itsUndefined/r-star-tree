#include "Data.h"

Data::Data() {
	file.open("data.bin", std::ios::out | std::ios::app);
	file.close();
	file.open("data.bin", std::ios::binary | std::ios::in | std::ios::out);
	
	if (GetBlockCount() == 0) {
		int emptyBlock[BLOCK_SIZE / sizeof(int)];
		for (int i = 0; i < BLOCK_SIZE / sizeof(int); i++) {
			emptyBlock[i] = INT_MAX;
		}
		
		SaveBlock(1, emptyBlock);
	}
}

Data::~Data() {
	file.close();
}

int Data::GetBlockCount() {
	file.seekp(0, std::ios::end);
	return (int) (file.tellg() / BLOCK_SIZE);
}

void Data::SaveBlock(int blockId, void* data) {
	file.seekp(((long long) blockId - 1) * BLOCK_SIZE);
	file.write((const char*)data, BLOCK_SIZE);
}

void Data::ReadBlock(int blockId, void* dataOut) {
	file.seekg(((long long) blockId - 1) * BLOCK_SIZE);
	file.read((char*)dataOut, BLOCK_SIZE);
}
