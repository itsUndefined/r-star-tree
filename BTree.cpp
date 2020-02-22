#include "BTree.h"

#include <iostream>
#include <cmath>

template<class T>
BTree<T>::BTree() {
	auto blockCount = data.GetBlockCount();
	this->nextBlockId = blockCount + 1;
	this->height = (int) floor(log2((BTreeNode<T>::BLOCK_VALUES_COUNT * blockCount) + 1) / log2(BTreeNode<T>::BLOCK_VALUES_COUNT + 1)) - 1;

	Key<T> loadedData[BTreeNode<T>::BLOCK_VALUES_COUNT + 1];
	data.ReadBlock(1, loadedData); // TODO Fetch root id from metadata
	this->root = std::shared_ptr<BTreeNode<T>>(new BTreeNode<T>(loadedData));
}

template<class T>
void BTree<T>::search(T val) {
	std::shared_ptr<BTreeNode<T>> loadedBlock = this->root;
	for (int i = 0; i < height; i++) {
		auto key = loadedBlock->search(val);
		if (key.key == val) {
			return;//return key.key; // found
		}
		loadedBlock = this->loadBlock(key.leftBlockPtr);
	}
	// not found
}

template<class T>
void BTree<T>::insert(T val) {
	//if (this->root.count() != BTreeNode<T>::BLOCK_VALUES_COUNT) { // If root not full
	//	BTreeNode<T> newRoot;
	//}
	int* path = new int[(long long)height + 1];
	path[0] = 1; // Set it to root 

	std::shared_ptr<BTreeNode<T>> loadedNode = this->root;
	for (int i = 0; i < height; i++) {
		auto key = loadedNode->search(val);
		if (key.key == val) {
			throw; // found can't insert two of the same value LOL
		}
		path[i + 1] = key.leftBlockPtr;
		loadedNode = this->loadBlock(key.leftBlockPtr);
	}

	if (loadedNode->count() != BTreeNode<T>::BLOCK_VALUES_COUNT + 1) { // If leaf not full
		loadedNode->insert(val);
	} else { // leaf full.... Splitting
		loadedNode->insert(val);
		for (int i = height; i >= 0; i--) {

			std::shared_ptr<BTreeNode<T>> parent;
			if (i == 0) { //root
				parent = std::unique_ptr<BTreeNode<T>>(new BTreeNode<T>); // TODO Set new root ID
			} else {
				parent = this->loadBlock(path[i - 1]);
			}
			

			std::unique_ptr<BTreeNode<T>> newRightBlock = loadedNode->split();
			Key<T>& forParent = (*loadedNode)[loadedNode->count() - 1];
			parent->insert(forParent.key);
			Key<T>& parentVal = parent->search(forParent.key);

			forParent.key = INT_MAX;

			parentVal.leftBlockPtr = path[height]; // old left
			(&parentVal + 1)->leftBlockPtr = nextBlockId++; // new right id

			if (parent->count() != BTreeNode<T>::BLOCK_VALUES_COUNT + 1) {
				break;
			}

			loadedNode = parent;
		}
		
	}
	void* blockRawData = loadedNode->getRawData();
	data.SaveBlock(path[height], blockRawData);

	delete blockRawData;
	delete[] path;
}

template<class T>
std::unique_ptr<BTreeNode<T>> BTree<T>::loadBlock(int blockId) {
	Key<T> loadedData[BTreeNode<T>::BLOCK_VALUES_COUNT + 1];
	data.ReadBlock(blockId, loadedData);

	return std::unique_ptr<BTreeNode<T>>(new BTreeNode<T>(loadedData));
}

// Tell the compiler for what types to compile the class.
template class BTree<int>;
