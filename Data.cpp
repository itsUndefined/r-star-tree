#include "Data.h"
#include <fstream>
#include <iostream>

Data::Data() : data(L"data.bin") {
	std::fstream metadataFile;
	metadataFile.open("data_metadata.bin", std::ios::binary | std::ios::in | std::ios::out);
	
	char columnCount; 
	metadataFile.read(&columnCount, 1);

	for (int i = 0; i < columnCount; i++) {
		ColumnType columnType = ColumnType::INTEGER;
		metadataFile.read((char*) &columnType, 1);

		if (columnType == ColumnType::CHAR) {
			unsigned int charLength = 0;
			metadataFile.read((char*)&charLength, 2);

			columns.push_back({ ColumnType::CHAR, charLength });
		}
		else {
			columns.push_back({ columnType, 0 });
		}
	}
	metadataFile.close();

	for (auto& column : columns) {
		switch (column.type) {
		case ColumnType::INTEGER:
			rowSize += sizeof(int);
			break;
		case ColumnType::FLOAT:
			rowSize += sizeof(float);
			break;
		case ColumnType::CHAR:
			rowSize += column.parameter * sizeof(wchar_t);
			break;
		}
	}

	this->currentBlockId = data.GetBlockCount();
	this->currentIndexAvailableForWrite = -1;
	this->currentBlockForWrite = std::unique_ptr<char[]>(new char[BLOCK_SIZE]);
	
	data.ReadBlock(currentBlockId, currentBlockForWrite.get());

	for (int i = 0; i < BLOCK_SIZE - 1; i += rowSize) {
		if (*(int*)(currentBlockForWrite.get() + i) == INT_MAX && *(int*)(currentBlockForWrite.get() + i + sizeof(int)) == INT_MAX) {
			currentIndexAvailableForWrite = i / rowSize;
			break;
		}
	}
	if (currentIndexAvailableForWrite == -1) {
		currentBlockId++;
		currentIndexAvailableForWrite = 0;
	}
	

}

void Data::PrintData() {
	std::unique_ptr<char> tempData(new char[512]);
	int position = 0;
	data.ReadBlock(1, tempData.get());

	for (auto& column : columns) {
		if (column.type == ColumnType::INTEGER) {
			int i = *(int*)(tempData.get() + position);
			std::wcout << "integer: " << i << std::endl;
			position += sizeof(int);
			continue;
		}
	
		if (column.type == ColumnType::FLOAT) {
			float f = *(float*)(tempData.get() + position);
			std::wcout << "float: " << f << std::endl;
			position += sizeof(float);
			continue;
		}
		
		if (column.type == ColumnType::CHAR) {
			std::wstring s((wchar_t*)(tempData.get() + position), column.parameter);
			std::wcout << "string: " << s.c_str() << std::endl;
			position += column.parameter * sizeof(wchar_t);
			continue;
		}
	}
}

void Data::InsertRow(std::unique_ptr<char[]> insertingData) {
	auto currentIndexBeforeUpdate = currentIndexAvailableForWrite;
	std::copy(insertingData.get(), insertingData.get() + rowSize, &currentBlockForWrite[currentIndexAvailableForWrite * rowSize]);
	if (currentIndexAvailableForWrite != BLOCK_SIZE / rowSize - 1) {
		*(int*)(currentBlockForWrite.get() + ((currentIndexAvailableForWrite + 1) * rowSize)) = INT_MAX;
		*(int*)(currentBlockForWrite.get() + ((currentIndexAvailableForWrite + 1) * rowSize + sizeof(int))) = INT_MAX;
		currentIndexAvailableForWrite++;
	}
	data.SaveBlock(currentBlockId, currentBlockForWrite.get());

	if (currentIndexBeforeUpdate == BLOCK_SIZE / rowSize - 1) {
		currentIndexAvailableForWrite = 0;
		currentBlockId++;
	}
}

Data::RecordBuilder Data::GetRecordBuilder() {
	return Data::RecordBuilder(this);
}

Data::RecordBuilder::RecordBuilder(Data* data) {
	this->data = data;
	this->nextColumn = data->columns.begin();
	
	this->builtData = std::unique_ptr<char[]>(new char[data->rowSize]);
	this->currentWritePtr = builtData.get();
}

Data::RecordBuilder& Data::RecordBuilder::InsertInteger(int data) {
	if (this->nextColumn->type != ColumnType::INTEGER) {
		std::cerr << "Invalid insertion" << std::endl;
		return *this;
	}
	*(int*)this->currentWritePtr = data;
	this->currentWritePtr += sizeof(int);
	this->nextColumn++;
	return *this;
}

Data::RecordBuilder& Data::RecordBuilder::InsertFloat(float data) {
	if (this->nextColumn->type != ColumnType::FLOAT) {
		std::cerr << "Invalid insertion" << std::endl;
		throw;
	}
	*(float*)this->currentWritePtr = data;
	this->currentWritePtr += sizeof(float);
	this->nextColumn++;
	return *this;
}

Data::RecordBuilder& Data::RecordBuilder::InsertCharN(std::wstring data) {
	if (this->nextColumn->type != ColumnType::CHAR) {
		std::cerr << "Invalid insertion" << std::endl;
		throw;
	}
	if (data.size() > this->nextColumn->parameter) {
		std::cerr << "Invalid insertion. Attempted to insert string that doesn't fit in column" << std::endl;
		throw;
	}
	int writeSize;
	if (data.size() < this->nextColumn->parameter) {
		writeSize = data.size() + 1;
	} else {
		writeSize = this->nextColumn->parameter;
	}
	std::copy(data.c_str(), data.c_str() + writeSize, (wchar_t*) currentWritePtr);
	this->currentWritePtr += this->nextColumn->parameter * sizeof(wchar_t);
	this->nextColumn++;
	return *this;
}

void Data::RecordBuilder::BuildAndSave() {
	if (this->nextColumn != data->columns.end()) {
		std::cerr << "Attempting to save incomplete column" << std::endl;
		throw;
	}
	data->InsertRow(std::move(builtData));
}
