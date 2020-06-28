#include "File.h"

#include <fileapi.h>

File::File(std::wstring filename) {
	/*
	file.open(filename, std::ios::out | std::ios::app);
	file.close();
	file.open(filename, std::ios::binary | std::ios::in | std::ios::out);
	*/

	hFile = CreateFileW(filename.c_str(), GENERIC_READ | GENERIC_WRITE , 0/*FILE_SHARE_READ*/, NULL, OPEN_ALWAYS, /*FILE_FLAG_NO_BUFFERING*/ NULL, NULL);
	auto b = GetLastError();
	if (hFile == INVALID_HANDLE_VALUE) {
		exit(1);
	}

	if (GetBlockCount() == 0) { // Initialize if empty
		int emptyBlock[BLOCK_SIZE / sizeof(int)];
		for (int i = 0; i < BLOCK_SIZE / sizeof(int); i++) {
			emptyBlock[i] = INT_MAX;
		}
		SaveBlock(1, emptyBlock);
	}
}

File::~File() {
	//file.close();
	CloseHandle(hFile);
}

int File::GetBlockCount() {
	//file.seekp(0, std::ios::end);
	//return (int) (file.tellg() / BLOCK_SIZE);
	
	LARGE_INTEGER fileSize;
	GetFileSizeEx(hFile, &fileSize);
	return fileSize.QuadPart / BLOCK_SIZE;
	
}

void File::SaveBlock(int blockId, void* data) {
	//file.seekp(((long long) blockId - 1) * BLOCK_SIZE);
	//file.write((const char*)data, BLOCK_SIZE);
	
	DWORD written = 0;
	OVERLAPPED o = { 0, 0, 0, 0, NULL };
	LARGE_INTEGER offset;
	offset.QuadPart = (blockId - 1) * (LONGLONG)BLOCK_SIZE;
	o.Offset = offset.LowPart;
	o.OffsetHigh = offset.HighPart;
	if (WriteFile(hFile, data, BLOCK_SIZE, &written, &o) == 0) {
		DWORD a = GetLastError();
		exit(1);
	};

}

void File::ReadBlock(int blockId, void* dataOut) {
	//file.seekg(((long long) blockId - 1) * BLOCK_SIZE);
	//file.read((char*)dataOut, BLOCK_SIZE);
	
	DWORD read = 0;
	OVERLAPPED o = { 0, 0, 0, 0, NULL };
	LARGE_INTEGER offset;
	offset.QuadPart = (blockId - 1) * (LONGLONG) BLOCK_SIZE;
	o.Offset = offset.LowPart;
	o.OffsetHigh = offset.HighPart;
	if (ReadFile(hFile, dataOut, BLOCK_SIZE, &read, &o) == 0) {
		DWORD a = GetLastError();
		exit(1);
	}
	
}
