#include "RStarTreeNode.h"
#include "File.h"
#include <algorithm>

using namespace RStar;

template<class T>
RStarTreeNode<T>::RStarTreeNode(int dimensions, int leaf, int blockId) {
	this->leaf = leaf;
	this->dimensions = dimensions;
	this->blockId = blockId;
	this->data.reserve(BLOCK_SIZE / Key<T>::GetKeySize(dimensions) + 1);
}

template<class T>
RStarTreeNode<T>::RStarTreeNode(typename std::vector<Key<T>>::iterator begin, typename std::vector<Key<T>>::iterator end, int dimensions, int leaf, int blockId): RStarTreeNode(dimensions, leaf, blockId) {
	this->data.assign(begin, end);
}

template<class T>
RStarTreeNode<T>::RStarTreeNode(char* diskData, int dimensions, int leaf, int blockId): RStarTreeNode(dimensions, leaf, blockId) {
	for (int i = 0; i < BLOCK_SIZE / Key<T>::GetKeySize(dimensions); i++) {
		T* min = (T*)(diskData + i * Key<T>::GetKeySize(dimensions));
		if (*(int*)min == INT_MAX) {
			break;
		}
		int leftBlockPtr = *(int*)(diskData + i * Key<T>::GetKeySize(dimensions) + 2 * dimensions * sizeof(T));
		if (!leaf) {
			T* max = (T*)(diskData + i * Key<T>::GetKeySize(dimensions) + dimensions * sizeof(T));
			this->data.emplace_back(
				min,
				max,
				leftBlockPtr,
				dimensions
			);
		} else {
			this->data.emplace_back(
				min,
				leftBlockPtr,
				dimensions
			);
		}
	}

	/*
	auto it = std::next(values.begin(), values.size());
	std::move(values.begin(), it, std::back_inserter(this->data));
	values.erase(values.begin(), it);
	*/
}

template<class T>
void RStarTreeNode<T>::insert(RStar::Key<T>& key) {
	this->data.push_back(key);
}

template<class T>
T RStarTreeNode<T>::overlap(RStar::Key<T>& key) {
	T overlapArea = 0;
	for (auto& node : *this) {
		if (node.blockPtr == key.blockPtr) {
			continue;
		}
		overlapArea += key.intersectArea(node);
	}
	return overlapArea;
}

template<class T>
bool RStarTreeNode<T>::isLeaf() {
	return this->leaf;
}

template<class T>
bool RStarTreeNode<T>::isFull() {
	return this->data.size() == BLOCK_SIZE / Key<T>::GetKeySize(dimensions);
}

template<class T>
std::unique_ptr<Key<T>> RStarTreeNode<T>::getBoundingBox() {
	T* min = new T[dimensions];
	T* max = new T[dimensions];
	std::fill_n(min, dimensions, INT_MAX);
	std::fill_n(max, dimensions, INT_MIN);

	for (auto& key : data) {
		for (int i = 0; i < dimensions; i++) {
			if (key.min[i] < min[i]) {
				min[i] = key.min[i];
			} 
			if (key.max[i] > max[i]) {
				max[i] = key.max[i];
			}
		}
	}

	return std::unique_ptr<Key<T>>(new Key<T>(min, max, blockId, dimensions));
}

template<class T>
std::unique_ptr<Key<T>> RStarTreeNode<T>::getBoundingBox(int start, int end) {
	T* min = new T[dimensions];
	T* max = new T[dimensions];
	std::fill_n(min, dimensions, INT_MAX);
	std::fill_n(max, dimensions, INT_MIN);

	if (start == end) {
		return std::unique_ptr<Key<T>>();
	}

	for (int i = start; i < end; i++) {
		for (int j = 0; j < dimensions; j++) {
			if (this->data[i].min[j] < min[j]) {
				min[j] = this->data[i].min[j];
			}
			if (this->data[i].max[j] > max[j]) {
				max[j] = this->data[i].max[j];
			}
		}
	}

	return std::unique_ptr<Key<T>>(new Key<T>(min, max, blockId, dimensions));
}

template<class T>
std::unique_ptr<char[]> RStarTreeNode<T>::getRawData() {
	auto blockBinaryContent = std::unique_ptr<char[]>(new char[BLOCK_SIZE]);
	int* dataPtr = (int*) blockBinaryContent.get();
	*dataPtr = isLeaf() ? 1 : 0;
	dataPtr += 1;

	for (auto it = this->data.begin(); it != this->data.end(); it++) {
		std::copy(it->min, it->min + dimensions, (T*)dataPtr);
		dataPtr = (int*)((T*)dataPtr + dimensions);
		std::copy(it->max, it->max + dimensions, (T*)dataPtr);
		dataPtr = (int*)((T*)dataPtr + dimensions);
		*dataPtr = it->blockPtr;
		dataPtr += 1;
	}
	if (!isFull()) {
		*dataPtr = INT_MAX;
	}
	return blockBinaryContent;
}

template<class T>
std::unique_ptr<RStarTreeNode<T>> RStarTreeNode<T>::split() {
	int M = BLOCK_SIZE / Key<T>::GetKeySize(dimensions);
	int m = 0.4 * M;

	int axis = chooseSplitAxis();
	int index = chooseSplitIndex(axis);
	if (index < M - 2 * m + 2) {
		std::sort(this->data.begin(), this->data.end(), [&](Key<T> a, Key<T> b) { return a.min[axis] < b.min[axis]; });
	} else {
		std::sort(this->data.begin(), this->data.end(), [&](Key<T> a, Key<T> b) { return a.max[axis] < b.max[axis]; });
		index -= M - 2 * m + 2;
	}

	std::unique_ptr<RStarTreeNode<T>> rightBlock = std::unique_ptr<RStarTreeNode<T>>(new RStarTreeNode<T>(this->data.begin() + m + index, this->data.end(), dimensions, leaf, INT_MAX));
	this->data.erase(this->data.begin() + m + index, this->data.end());

	return rightBlock;
}

template<class T>
int RStarTreeNode<T>::chooseSplitAxis() {
	T minSum = std::numeric_limits<T>::max();
	int axis = 0;

	int M = BLOCK_SIZE / Key<T>::GetKeySize(dimensions);
	int m = 0.4 * M;

	for (int i = 0; i < dimensions; i++) {

		T marginSum = 0;

		std::sort(this->data.begin(), this->data.end(), [&](Key<T>& a, Key<T>& b) {
			return a.min[i] < b.min[i]; 
		});
		for (int k = 0; k < M - 2 * m + 2; k++) {
			auto bbFirstGroup = getBoundingBox(0, m + k);
			auto bbSecondGroup = getBoundingBox(m + k, M + 1);
			if (!bbFirstGroup || !bbSecondGroup) {
				continue;
			}
			marginSum += bbFirstGroup->marginValue() + bbSecondGroup->marginValue();
		}

		std::sort(this->data.begin(), this->data.end(), [&](Key<T>& a, Key<T>& b) { return a.max[i] < b.max[i]; });
		for (int k = 0; k < M - 2 * m + 2; k++) {
			auto bbFirstGroup = getBoundingBox(0, m + k);
			auto bbSecondGroup = getBoundingBox(m + k, M + 1);
			if (!bbFirstGroup || !bbSecondGroup) {
				continue;
			}
			marginSum += bbFirstGroup->marginValue() + bbSecondGroup->marginValue();
		}

		if (marginSum < minSum) {
			minSum = marginSum;
			axis = i;
		}
	}

	return axis;
}

template<class T>
int RStarTreeNode<T>::chooseSplitIndex(int axis) {

	int M = BLOCK_SIZE / Key<T>::GetKeySize(dimensions);
	int m = 0.4 * M;

	T minOverlap = std::numeric_limits<T>::max();
	T minArea = std::numeric_limits<T>::max();
	int index = 0;

	std::sort(this->data.begin(), this->data.end(), [&](Key<T> a, Key<T> b) { return a.min[axis] < b.min[axis]; });
	for (int k = 0; k < M - 2 * m + 2; k++) {
		auto bbFirstGroup = getBoundingBox(0, m + k);
		auto bbSecondGroup = getBoundingBox(m + k, M + 1);
		if (!bbFirstGroup || !bbSecondGroup) {
			continue;
		}
		auto overlap = bbFirstGroup->intersectArea(*bbSecondGroup);
		T area = bbFirstGroup->areaValue() + bbSecondGroup->areaValue();

		if (overlap == minOverlap && area < minArea) {
			minArea = area;
			minOverlap = overlap;
			index = k;
		}

		if (overlap < minOverlap) {
			minArea = area;
			minOverlap = overlap;
			index = k;
		}
	}

	std::sort(this->data.begin(), this->data.end(), [&](Key<T> a, Key<T> b) { return a.max[axis] < b.max[axis]; });
	for (int k = 0; k < M - 2 * m + 2; k++) {
		auto bbFirstGroup = getBoundingBox(0, m + k);
		auto bbSecondGroup = getBoundingBox(m + k, M + 1);
		if (!bbFirstGroup || !bbSecondGroup) {
			continue;
		}
		auto overlap = bbFirstGroup->intersectArea(*bbSecondGroup);
		T area = bbFirstGroup->areaValue() + bbSecondGroup->areaValue();

		if (overlap == minOverlap && area < minArea) {
			minArea = area;
			minOverlap = overlap;
			index = k + (M - 2 * m + 2); // TODO check
		}

		if (overlap < minOverlap) {
			minArea = area;
			minOverlap = overlap;
			index = k + (M - 2 * m + 2);
		}
	}
	return index;
}

// Tell the compiler for what types to compile the class.
template class RStarTreeNode<int>;
template class RStarTreeNode<float>;
template class RStarTreeNode<double>;