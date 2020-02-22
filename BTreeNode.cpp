#include "BTreeNode.h"

#include <algorithm>
#include <iostream>

template<class T>
BTreeNode<T>::BTreeNode() {
}

template<class T>
BTreeNode<T>::BTreeNode(int size) {
	this->data.resize(size);
}

template<class T>
BTreeNode<T>::BTreeNode(Key<T> values[]) {
	this->loadFromArray(values);
}

template<class T>
BTreeNode<T>::BTreeNode(const BTreeNode<T>& source) {
	std::cout << "CALLED WHEN IT SHOULDNT!";
}

template<class T> Key<T>& BTreeNode<T>::search(T val) {
	auto it = std::lower_bound(this->data.begin(), this->data.end(), val);
	
	return *it;
}

template<class T>
void BTreeNode<T>::insert(T val) {
	auto it = data.begin();
	for (; it != data.end(); it++) {
		if (it->key > val) {
			break;
		}
	}

	if (it == data.end()) {
		if (this->data.size() > 0) {
			this->data.back().key = val; // Check that the logic is right
		} else {
			this->data.push_back({ val, NULL });
		}
		this->data.push_back({ INT_MAX, NULL });
		return;
	}

	this->data.insert(it, { val, NULL });

}

template<class T>
void BTreeNode<T>::loadFromArray(Key<T> values[]) {
	for (int i = 0; i < BLOCK_VALUES_COUNT + 1; i++) {
		if (values[i].key == INT_MAX && values[i].leftBlockPtr == INT_MAX) {
			return;
		}
		this->data.push_back(values[i]); // TODO PERFORMANCE IMPLICATIONS?
	}
	//this->data.assign(values, values + BLOCK_VALUES_COUNT);
	
}

template<class T>
std::unique_ptr<BTreeNode<T>> BTreeNode<T>::split() {
	std::unique_ptr<BTreeNode<T>> rightBlock = std::unique_ptr<BTreeNode<T>>(new BTreeNode<T>((BLOCK_VALUES_COUNT / 2) + 1));
	Key<T>* middleValPtr = this->data.data() + (BLOCK_VALUES_COUNT / 2) + 1;

	for (int i = 0; i < BLOCK_VALUES_COUNT / 2; i++) {
		(*rightBlock)[i] = middleValPtr[i];
	}
	//rightBlock[BLOCK_VALUES_COUNT / 2].leftBlockPtr = this->rightBlockPtr;

	this->data.resize((BLOCK_VALUES_COUNT / 2) + 1);

	return rightBlock;
}

template<class T>
void* BTreeNode<T>::getRawData() {
	int* result = new int[BLOCK_SIZE / sizeof(int)];
	memcpy(result, this->data.data(), this->data.size() * sizeof(Key<T>));
	return result;
}

// Tell the compiler for what types to compile the class.
template class BTreeNode<int>;
