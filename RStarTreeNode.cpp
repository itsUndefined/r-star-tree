#include "RStarTreeNode.h"
#include "File.h"
#include <algorithm>

using namespace RStar;

RStarTreeNode::RStarTreeNode(int dimensions, int leaf, int blockId) {
	this->leaf = leaf;
	this->dimensions = dimensions;
	this->blockId = blockId;
}


RStarTreeNode::RStarTreeNode(std::vector<Key<int>>::iterator begin, std::vector<Key<int>>::iterator end, int dimensions, int leaf, int blockId): RStarTreeNode(dimensions, leaf, blockId) {
	this->data.assign(begin, end);
}

RStarTreeNode::RStarTreeNode(char* diskData, int dimensions, int leaf, int blockId): RStarTreeNode(dimensions, leaf, blockId) {
	for (int i = 0; i < BLOCK_SIZE / Key<int>::GetKeySize(dimensions); i++) {
		int* min = (int*)(diskData + i * Key<int>::GetKeySize(dimensions));
		if (min[0] == INT_MAX) {
			break;
		}
		int leftBlockPtr = *(int*)(diskData + i * Key<int>::GetKeySize(dimensions) + 2 * dimensions * sizeof(int));
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

void RStarTreeNode::insert(RStar::Key<int>& key) {
	this->data.push_back(key);
}

double RStar::RStarTreeNode::overlap(RStar::Key<int>& key) {
	double overlapArea = 0.0;
	for (auto& node : *this) {
		if (node.blockPtr == key.blockPtr) {
			continue;
		}
		overlapArea += key.intersectArea(node);
	}
	return overlapArea;
}

bool RStarTreeNode::isLeaf() {
	return this->leaf;
}

bool RStarTreeNode::isFull() {
	return this->data.size() == BLOCK_SIZE / Key<int>::GetKeySize(dimensions);
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
	if (!isFull()) {
		*dataPtr = INT_MAX;
	}
	return blockBinaryContent;
}

std::unique_ptr<RStarTreeNode> RStarTreeNode::split() {
	int M = BLOCK_SIZE / Key<int>::GetKeySize(dimensions);
	int m = 0.4 * M;

	int axis = chooseSplitAxis();
	int index = chooseSplitIndex(axis);
	if (index < M - 2 * m + 2) {
		std::sort(this->data.begin(), this->data.end(), [&](Key<int> a, Key<int> b) { return a.min[axis] < b.min[axis]; });
	} else {
		std::sort(this->data.begin(), this->data.end(), [&](Key<int> a, Key<int> b) { return a.max[axis] < b.max[axis]; });
		index -= M - 2 * m + 2;
	}

	std::unique_ptr<RStarTreeNode> rightBlock = std::unique_ptr<RStarTreeNode>(new RStarTreeNode(this->data.begin() + m + index + 1, this->data.end(), dimensions, leaf, 69));
	this->data.erase(this->data.begin(), this->data.begin() + m + index + 1);

	return rightBlock;
}

int RStarTreeNode::chooseSplitAxis() {
	double minSum = std::numeric_limits<double>::max();
	int axis = 0;

	int M = BLOCK_SIZE / Key<int>::GetKeySize(dimensions);
	int m = 0.4 * M;

	for (int i = 0; i < dimensions; i++) {

		double marginSum = 0;

		std::sort(this->data.begin(), this->data.end(), [&](Key<int>& a, Key<int>& b) {
			return a.min[i] < b.min[i]; 
		});
		for (int k = 0; k < M - 2 * m + 2; k++) {
			auto bbFirstGroup = getBoundingBox(0, m + k);
			auto bbSecondGroup = getBoundingBox(m + k + 1, M + 1);
			marginSum += bbFirstGroup->marginValue() + bbSecondGroup->marginValue();
		}

		std::sort(this->data.begin(), this->data.end(), [&](Key<int> a, Key<int> b) { return a.max[i] < b.max[i]; });
		for (int k = 0; k < M - 2 * m + 2; k++) {
			auto bbFirstGroup = getBoundingBox(0, m + k);
			auto bbSecondGroup = getBoundingBox(m + k + 1, M + 1);
			marginSum += bbFirstGroup->marginValue() + bbSecondGroup->marginValue();
		}

		if (marginSum < minSum) {
			minSum = marginSum;
			axis = i;
		}
	}

	return axis;
}

int RStarTreeNode::chooseSplitIndex(int axis) {

	int M = BLOCK_SIZE / Key<int>::GetKeySize(dimensions) - 1;
	int m = 0.4 * M;

	double minOverlap = std::numeric_limits<double>::max();
	int index = 0;

	std::sort(this->data.begin(), this->data.end(), [&](Key<int> a, Key<int> b) { return a.min[axis] < b.min[axis]; });
	for (int k = 0; k < M - 2 * m + 2; k++) {
		auto bbFirstGroup = getBoundingBox(0, m + k);
		auto bbSecondGroup = getBoundingBox(m + k + 1, M + 2);
		auto overlap = bbFirstGroup->intersectArea(*bbSecondGroup);
		if (overlap < minOverlap) {
			minOverlap = overlap;
			index = k;
		}
	}

	std::sort(this->data.begin(), this->data.end(), [&](Key<int> a, Key<int> b) { return a.max[axis] < b.max[axis]; });
	for (int k = 0; k < M - 2 * m + 2; k++) {
		auto bbFirstGroup = getBoundingBox(0, m + k);
		auto bbSecondGroup = getBoundingBox(m + k + 1, M + 2);
		auto overlap = bbFirstGroup->intersectArea(*bbSecondGroup);
		if (overlap < minOverlap) {
			minOverlap = overlap;
			index = k + (M - 2 * m + 2); // TODO check 
		}
	}

	return index;
}