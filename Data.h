#pragma once

#include "File.h"
#include "vector"
#include "RStarTree.h"


enum class ColumnType {
	INTEGER = 0,
	FLOAT = 1,
	CHAR = 2 // Uses next 2 bytes for CHAR length. For example 02 0F 00 For CHAR(15)
};

struct Column {
	ColumnType type;
	unsigned int parameter;
};

class Data
{

	class RecordBuilder
	{

	public:
		RecordBuilder(Data* data);
		// inserts an integer to the memory
		RecordBuilder& InsertInteger(int data);
		// inserts a float to the memory
		RecordBuilder& InsertFloat(float data);
		// insret a char to the memory
		RecordBuilder& InsertCharN(std::wstring data);
		// create a file from the data that is to the memory (InsertInteger, InsertFloat, InsertCharN)
		void BuildAndSave();
	private:
		Data* data;
		std::vector<Column>::iterator nextColumn;
		std::unique_ptr<char[]> builtData;
		char* currentWritePtr;
	};

public:
	Data();
	void PrintData();
	void InsertRow(std::unique_ptr<char[]> data);
	RecordBuilder GetRecordBuilder();


private:
	File data;
	RStarTree<float> index;
	std::vector<Column> columns;
	int rowSize = 0;
	std::unique_ptr<char[]> currentBlockForWrite;
	int currentBlockId;
	int currentIndexAvailableForWrite;
};

