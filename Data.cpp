#include "Data.h"
#include <fstream>
#include <iostream>
#include <queue>
#include <chrono>

Data::Data() : data(L"data.bin"), index(2) {
	std::fstream metadataFile;
	metadataFile.open("data_metadata.bin", std::ios::binary | std::ios::in | std::ios::out);
	
	char columnCount; 
	metadataFile.read(&columnCount, 1);

	for (int i = 0; i < columnCount; i++) {
		ColumnType columnType = ColumnType::INTEGER;
		metadataFile.read((char*) &columnType, 1);

		if (columnType == ColumnType::CHAR) {
			unsigned int charLength = 0;
			metadataFile.read((char*)&charLength, 1);

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

void Data::PrintData(const char* block, int index) {
	int position = index * rowSize;

	for (auto& column : columns) {
		if (column.type == ColumnType::INTEGER) {
			int i = *(int*)(block + position);
			std::wcout << "integer: " << i << std::endl;
			position += sizeof(int);
			continue;
		}
	
		if (column.type == ColumnType::FLOAT) {
			float f = *(float*)(block + position);
			std::wcout << "float: " << f << std::endl;
			position += sizeof(float);
			continue;
		}
		
		if (column.type == ColumnType::CHAR) {
			std::wstring s((wchar_t*)(block + position), column.parameter);
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
	index.insertData((float*)(insertingData.get() + rowSize - 8), currentBlockId);

	if (currentIndexBeforeUpdate == BLOCK_SIZE / rowSize - 1) {
		currentIndexAvailableForWrite = 0;
		currentBlockId++;
	}
}

void Data::RangeSearch(float* min, float* max, bool withIndex) {
	
	char* dataOut = new char[BLOCK_SIZE];

	if (withIndex) {
		auto start = std::chrono::high_resolution_clock::now();
		auto keys = index.rangeSearch(min, max);
		
		for (auto& key : keys) {

			data.ReadBlock(key.blockPtr, dataOut);

			for (int i = 0; i < BLOCK_SIZE; i += rowSize) {
				if (*(int*)(dataOut + i) == INT_MAX) {
					break;
				}

				float x = *(float*)(dataOut + i + rowSize - 2 * sizeof(float));
				float y = *(float*)(dataOut + i + rowSize - sizeof(float));
				if (key.min[0] == x && key.min[1] == y) {
					//PrintData(dataOut, i / rowSize);
					break;
				}
			}

		}

		auto stop = std::chrono::high_resolution_clock::now();
		auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(stop - start);
		std::wcout << duration.count() << L" milliseconds using R* index" << std::endl;
	}

	if (!withIndex) {
		auto start = std::chrono::high_resolution_clock::now();
		for (int blockId = 1; blockId <= currentBlockId; blockId++) {
			data.ReadBlock(blockId, dataOut);
			for (int i = 0; i < BLOCK_SIZE; i += rowSize) {
				if (*(int*)(dataOut + i) == INT_MAX) {
					break;
				}
				float x = *(float*)(dataOut + i + rowSize - 2 * sizeof(float));
				float y = *(float*)(dataOut + i + rowSize - sizeof(float));
				if (min[0] < x && x < max[0] && min[1] < y && y < max[1]) {
					//PrintData(dataOut, i / rowSize);
				}
			}
		}
		auto stop = std::chrono::high_resolution_clock::now();
		auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(stop - start);
		std::wcout << duration.count() << L" milliseconds using full table scan" << std::endl;
	}

	delete[] dataOut;
}

void Data::KNNSearch(float* point, int k, bool withIndex) {
	char* dataOut = new char[BLOCK_SIZE];

	if (withIndex) {
		auto start = std::chrono::high_resolution_clock::now();
		auto keys = index.kNNSearch(point, k);

		while (!keys.empty()) {
			auto key = keys.top();
			keys.pop();

			data.ReadBlock(key.blockPtr, dataOut);

			for (int i = 0; i < BLOCK_SIZE; i += rowSize) {
				if (*(int*)(dataOut + i) == INT_MAX) {
					break;
				}
				float x = *(float*)(dataOut + i + rowSize - 2 * sizeof(float));
				float y = *(float*)(dataOut + i + rowSize - sizeof(float));
				if (key.min[0] == x && key.min[1] == y) {
					//PrintData(dataOut, i / rowSize);
					break;
				}
			}
		}

		auto stop = std::chrono::high_resolution_clock::now();
		auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(stop - start);
		std::wcout << duration.count() << L" milliseconds using R* tree" << std::endl;
	}

	if (!withIndex) {
		RStar::Key<float> fromPoint(point, INT_MAX, 2);

		typedef std::pair<RStar::Key<float>, std::unique_ptr<char[]>> Pair;

		std::priority_queue<Pair, std::vector<Pair>, std::function<bool(Pair&, Pair&)>> pq([&](Pair& a, Pair& b) { return a.first.distanceFromEdge(fromPoint) < b.first.distanceFromEdge(fromPoint); });

		auto start = std::chrono::high_resolution_clock::now();
		for (int blockId = 1; blockId <= currentBlockId; blockId++) {
			data.ReadBlock(blockId, dataOut);
			for (int i = 0; i < BLOCK_SIZE; i += rowSize) {

				if (*(int*)(dataOut + i) == INT_MAX) {
					break;
				}

				float dims[2];
				dims[0] = *(float*)(dataOut + i + rowSize - 2 * sizeof(float));
				dims[1] = *(float*)(dataOut + i + rowSize - sizeof(float));


				RStar::Key<float> currentKey(dims, INT_MAX, 2);
				
				if (pq.size() == k) {
					auto& farKey = pq.top().first;
					if (farKey.distanceFromEdge(fromPoint) > currentKey.distanceFromEdge(fromPoint)) {
						pq.pop();
						std::unique_ptr<char[]> rowData(new char[rowSize]);
						std::copy(dataOut + i, dataOut + i + rowSize, rowData.get());
						pq.emplace(std::move(currentKey), std::move(rowData));
					}
				} else {
					std::unique_ptr<char[]> rowData(new char[rowSize]);
					std::copy(dataOut + i, dataOut + i + rowSize, rowData.get());
					pq.emplace(std::move(currentKey), std::move(rowData));
				}

			}
		}

		while (!pq.empty()) {
			//PrintData(pq.top().second.get(), 0);
			pq.pop();
		}
		auto stop = std::chrono::high_resolution_clock::now();
		auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(stop - start);
		std::wcout << duration.count() << L" milliseconds using full table scan" << std::endl;
	}

	delete[] dataOut;
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
	size_t writeSize;
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
