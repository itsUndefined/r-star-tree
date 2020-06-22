#include "RStarTreeNode.h"
#include "File.h"

using namespace RStar;

RStarTreeNode::RStarTreeNode(int dimensions) {
	this->leaf = true;
	this->dimensions = dimensions;
}


RStarTreeNode::RStarTreeNode(int size, int dimensions): RStarTreeNode(dimensions) {
	this->data.resize(size);
}

RStarTreeNode::RStarTreeNode(char* diskData, int dimensions): RStarTreeNode(dimensions) {
	for (int i = 0; i < BLOCK_SIZE / Key<int>::GetKeySize(dimensions) - 1; i++) {
		int* min = (int*)(diskData + i * Key<int>::GetKeySize(dimensions));
		if (min[0] == INT_MAX) {
			break;
		}
		int* max = (int*)(diskData + i * Key<int>::GetKeySize(dimensions) + dimensions * sizeof(int));
		int leftBlockPtr = (int)*(diskData + i * Key<int>::GetKeySize(dimensions) + 2 * dimensions * sizeof(int));
		this->data.push_back(Key<int>(
			min,
			max,
			leftBlockPtr,
			dimensions
		));
	}

	/*
	auto it = std::next(values.begin(), values.size());
	std::move(values.begin(), it, std::back_inserter(this->data));
	values.erase(values.begin(), it);
	*/
}

void RStarTreeNode::insert(int* val) {
	if (!isLeaf()) {
		throw "should not have happened";
	}

	this->data.push_back(Key<int>(val, 69, this->dimensions));
}

bool RStarTreeNode::isLeaf() {
	return this->leaf;
}

std::unique_ptr<char[]> RStarTreeNode::getRawData() {
	auto blockBinaryContent = std::unique_ptr<char[]>(new char[BLOCK_SIZE]);
	int* dataPtr = (int*) blockBinaryContent.get();
	*dataPtr = isLeaf() ? 1 : 0;
	dataPtr += 1;

	for (auto it = this->data.begin(); it != this->data.end(); it++) {
		std::copy(it->min, it->min + dimensions, dataPtr);
		dataPtr += dimensions;
		if (it->max != nullptr) {
			std::copy(it->max, it->max + dimensions, dataPtr);
		}
		dataPtr += dimensions;
		*dataPtr = it->leftBlockPtr;
		dataPtr += 1;
	}
	*dataPtr = INT_MAX;
	return blockBinaryContent;
}


