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
	auto it = std::lower_bound(this->data.begin(), this->data.end(), val);

	if (it == data.end()) {
		if (this->data.size() > 0) {
			this->data.back().key = val; // Check that the logic is right
		} else {
			this->data.push_back({ val, INT_MAX });
		}
		this->data.push_back({ INT_MAX, INT_MAX });
		return;
	}

	this->data.insert(it, { val, INT_MAX });

}

template<class T>
void BTreeNode<T>::loadFromArray(Key<T> values[]) {
	int i;
	for (i = 0; i < BLOCK_VALUES_COUNT; i++) {
		if (values[i].key == INT_MAX) {
			break;
		}
	}
	this->data.assign(values, values + i + 1);
	/*for (int i = 0; i < BLOCK_VALUES_COUNT + 1; i++) {
		this->data.push_back(values[i]); // TODO PERFORMANCE IMPLICATIONS?
		if (values[i].key == INT_MAX) {
			return;
		}
	}*/
	//this->data.assign(values, values + BLOCK_VALUES_COUNT);
	
}

template<class T>
std::unique_ptr<BTreeNode<T>> BTreeNode<T>::split() {
	std::unique_ptr<BTreeNode<T>> rightBlock = std::unique_ptr<BTreeNode<T>>(new BTreeNode<T>((BLOCK_VALUES_COUNT / 2) + 1));
	Key<T>* middleValPtr = this->data.data() + (BLOCK_VALUES_COUNT / 2) + 1;

	for (int i = 0; i <= BLOCK_VALUES_COUNT / 2; i++) {
		(*rightBlock)[i] = middleValPtr[i];
	}

	//(*rightBlock.get())[BLOCK_VALUES_COUNT / 2] = { INT_MAX, INT_MAX };
	//rightBlock[BLOCK_VALUES_COUNT / 2].leftBlockPtr = this->rightBlockPtr;

	this->data.resize((BLOCK_VALUES_COUNT / 2) + 1);

	return rightBlock;
}

template<class T>
std::unique_ptr<Key<T>[]> BTreeNode<T>::getRawData() {
	auto result = std::unique_ptr<Key<T>[]>(new Key<T>[BLOCK_VALUES_COUNT + 1]);
	std::copy(this->data.begin(), this->data.end(), result.get());
	return result;
}

// Tell the compiler for what types to compile the class.
template class BTreeNode<int>;
