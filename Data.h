#pragma once

#include "File.h"


enum class ColumnType {
	INTEGER = 0,
	FLOAT = 1,
	CHAR = 2 // Uses next 2 bytes for CHAR length. For example 02 0F 00 For CHAR(15)
};

class Data
{
	Data();


private:
	File data;
};

