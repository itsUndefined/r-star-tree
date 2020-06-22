#pragma once

#include "File.h"
#include "vector"


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
public:
	Data();
	void PrintData();
	void InsertRow(std::unique_ptr<char> data);


private:
	File data;
	std::vector<Column> columns;
};

