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
}

void Data::PrintData() {
	std::unique_ptr<char> tempData(new char[512]);
	int position = 0;
	data.ReadBlock(1, tempData.get());

	for (auto& column : columns) {
		if (column.type == ColumnType::INTEGER) {
			int a = *(int*)(tempData.get() + position);
			std::cout << "integer: " << a << std::endl;
			position += 4;
			continue;
		}
	
		if (column.type == ColumnType::FLOAT) {
			float a = *(float*)(tempData.get() + position);
			std::cout << "float: " << a << std::endl;
			position += 4;
			continue;
		}
		
		if (column.type == ColumnType::CHAR) {
			std::string a(tempData.get() + position, column.parameter);
			std::cout << "string: " << a << std::endl;
			position += column.parameter;
			continue;
		}
	}
}

void Data::InsertRow(std::unique_ptr<char> insertingData) {
	data.SaveBlock(1, insertingData.get());
}