#include "RStarTreeNode.h"
#include "File.h"
#include <algorithm>

using namespace RStar;

RStarTreeNode::RStarTreeNode(int dimensions, int leaf, int blockId) {
	this->leaf = leaf;
	this->dimensions = dimensions;
	this->blockId = blockId;
}


RStarTreeNode::RStarTreeNode(int size, int dimensions, int leaf, int blockId): RStarTreeNode(dimensions, leaf, blockId) {
	this->data.resize(size);
}

RStarTreeNode::RStarTreeNode(char* diskData, int dimensions, int leaf, int blockId): RStarTreeNode(dimensions, leaf, blockId) {
	for (int i = 0; i < BLOCK_SIZE / Key<int>::GetKeySize(dimensions) - 1; i++) {
		int* min = (int*)(diskData + i * Key<int>::GetKeySize(dimensions));
		if (min[0] == INT_MAX) {
			break;
		}
		int leftBlockPtr = (int)*(diskData + i * Key<int>::GetKeySize(dimensions) + 2 * dimensions * sizeof(int));
		if (!leaf) {
			int* max = (int*)(diskData + i * Key<int>::GetKeySize(dimensions) + dimensions * sizeof(int));
			this->data.push_back(Key<int>(
				min,
				max,
				leftBlockPtr,
				dimensions
			));
		} else {
			this->data.push_back(Key<int>(
				min,
				leftBlockPtr,
				dimensions
			));
		}
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

double RStar::RStarTreeNode::overlap(Key<int>& key) {
	double overlapArea = 0.0;
	for (auto& node : *this) {
		if (node.blockPtr == key.blockPtr) {
			continue;
		}
		overlapArea += key.intersectArea(node.min, node.max);
	}
	return overlapArea;
}

bool RStarTreeNode::isLeaf() {
	return this->leaf;
}

bool RStarTreeNode::isFull() {
	return this->data.size() == BLOCK_SIZE / Key<int>::GetKeySize(dimensions) - 1;
}

std::unique_ptr<Key<int>> RStarTreeNode::getBoundingBox() {
	int* min = new int[dimensions];
	int* max = new int[dimensions];
	std::fill_n(min, dimensions, INT_MAX);
	std::fill_n(max, dimensions, INT_MIN);

	for (auto& key : data) {
		for (int i = 0; i < dimensions; i++) {
			if (key.min[i] < min[i]) {
				min[i] = key.min[i];
			} else if (key.max[i] > max[i]) {
				max[i] = key.max[i];
			}
		}
	}

	return std::unique_ptr<Key<int>>(new Key<int>(min, max, blockId, dimensions));
}

std::unique_ptr<Key<int>> RStarTreeNode::getBoundingBox(int start, int end) {
	int* min = new int[dimensions];
	int* max = new int[dimensions];
	std::fill_n(min, dimensions, INT_MAX);
	std::fill_n(max, dimensions, INT_MIN);

	for (int i = start; i < end; i++) {
		for (int j = 0; j < dimensions; j++) {
			if (this->data[i].min[j] < min[j]) {
				min[j] = this->data[i].min[j];
			}
			else if (this->data[i].max[j] > max[j]) {
				max[j] = this->data[i].max[j];
			}
		}
	}

	return std::unique_ptr<Key<int>>(new Key<int>(min, max, blockId, dimensions));
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
		*dataPtr = it->blockPtr;
		dataPtr += 1;
	}
	*dataPtr = INT_MAX;
	return blockBinaryContent;
}

int RStarTreeNode::chooseSplitAxis() {
	double* min = new double[dimensions];
	for (int i = 0; i < dimensions; i++) {
		std::sort(this->data.begin(), this->data.end(), [&](Key<int> a, Key<int> b) { a.min[i] < b.min[i]; });
		auto bbLowerFirstGroup = getBoundingBox(0, this->data.size() - 2 * 0.4 + 2);
		auto bbLowerSecondGroup = getBoundingBox(this->data.size() - 2 * 0.4 + 2, this->data.size());
		double bbLowerMargin = bbLowerFirstGroup->marginValue() + bbLowerSecondGroup->marginValue();
		std::sort(this->data.begin(), this->data.end(), [&](Key<int> a, Key<int> b) { a.max[i] > b.max[i]; });
		auto bbUpperFirstGroup = getBoundingBox(0, this->data.size() - 2 * 0.4 + 2);
		auto bbUpperSecondGroup = getBoundingBox(this->data.size() - 2 * 0.4 + 2, this->data.size());
		double bbUpperMargin = bbUpperFirstGroup->marginValue() + bbUpperSecondGroup->marginValue();
		if (bbLowerMargin < bbUpperMargin) {
			min[i] = bbLowerMargin;
		} else {
			min[i] = bbUpperMargin;
		}
	}
	double minVal = min[0];
	int axis = 0;
	for (int i = 1; i < dimensions; i++) {
		if (min[i] < minVal) {
			minVal = min[i];
			axis = i;
		}
	}
	return axis;
}