#include "Data.h"
#include <fstream>

Data::Data() : data(L"data.bin") {
	std::fstream metadataFile;
	metadataFile.open("data_metadata.bin", std::ios::binary | std::ios::in | std::ios::out);
	
	char columnCount; 
	metadataFile.read(&columnCount, 1);

	for (int i = 0; i < columnCount; i++) {
		ColumnType columnType;
		metadataFile.read((char*) &columnType, 1);

		if (columnType == ColumnType::CHAR) {
			int charLength;
			metadataFile.read((char*)&charLength, 2);
		}

		
	}
	metadataFile.close();
	
}