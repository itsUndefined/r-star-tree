#include "BTree.h"

#include <iostream>
#include <cmath>

template<class T>
BTree<T>::BTree() {
	this->rootId = 1; //Store in metadata

	auto blockCount = data.GetBlockCount();
	this->nextBlockId = blockCount + 1;
	//this->height = (int) ceil(log2((BTreeNode<T>::BLOCK_VALUES_COUNT * blockCount) + 1) / log2(BTreeNode<T>::BLOCK_VALUES_COUNT + 1)) - 1;

	Key<T> loadedData[BTreeNode<T>::BLOCK_VALUES_COUNT + 1];
	data.ReadBlock(this->rootId, loadedData);
	this->root = std::shared_ptr<BTreeNode<T>>(new BTreeNode<T>(loadedData));
}


template<class T>
void BTree<T>::search(T val) { // Calling search on a completely empty BTree will crash the program
	std::shared_ptr<BTreeNode<T>> loadedBlock = this->root;
	while (true) {
		auto key = loadedBlock->search(val);
		if (key.key == val) {
			return;//return key.key; // found
		}
		if (key.leftBlockPtr == INT_MAX) {
			std::cout << "DAMAGE";
			return;//not found
		}
		loadedBlock = this->loadBlock(key.leftBlockPtr);
	}
}

template<class T>
void BTree<T>::insert(T val) {
	std::vector<int> path;
	path.push_back(this->rootId);

	std::shared_ptr<BTreeNode<T>> loadedNode = this->root;

	while (true) {
		auto key = loadedNode->search(val);
		if (key.key == val) {
			throw; // found can't insert two of the same value LOL
		}
		if (key.leftBlockPtr == INT_MAX) {
			break;
		}
		path.push_back(key.leftBlockPtr);
		loadedNode = this->loadBlock(key.leftBlockPtr);
	}

	if (loadedNode->count() != BTreeNode<T>::BLOCK_VALUES_COUNT + 1) { // If leaf not full
		loadedNode->insert(val);
		data.SaveBlock(path.back(), loadedNode->getRawData().get());
	} else { // leaf full.... Splitting
		loadedNode->insert(val);
		for (int i = path.size() - 1; i >= 0; i--) {
			std::shared_ptr<BTreeNode<T>> parent;
			if (i == 0) { //root
				parent = std::unique_ptr<BTreeNode<T>>(new BTreeNode<T>);
				this->rootId = nextBlockId++;
				this->root = parent;
			} else if(i == 1) {
				parent = this->root;
			} else {
				parent = this->loadBlock(path[i - 1]);
			}
			
			std::unique_ptr<BTreeNode<T>> newRightBlock = loadedNode->split();
			Key<T>& forParent = (*loadedNode)[loadedNode->count() - 1];
			parent->insert(forParent.key);
			Key<T>& parentVal = parent->search(forParent.key);

			forParent.key = INT_MAX;

			int newRightBlockId = nextBlockId++;

			parentVal.leftBlockPtr = path[i]; // old left? maybe?
			(&parentVal + 1)->leftBlockPtr = newRightBlockId;

			data.SaveBlock(path[i], loadedNode->getRawData().get());
			data.SaveBlock(newRightBlockId, newRightBlock->getRawData().get());
			if (i == 0) {
				data.SaveBlock(this->rootId, parent->getRawData().get());
				break;
			}

			if (parent->count() != BTreeNode<T>::BLOCK_VALUES_COUNT + 2) {
				data.SaveBlock(path[i - 1], parent->getRawData().get());
				break;
			}

			loadedNode = parent;
		}
		
	}
}

template<class T>
std::unique_ptr<BTreeNode<T>> BTree<T>::loadBlock(int blockId) {
	Key<T> loadedData[BTreeNode<T>::BLOCK_VALUES_COUNT + 1];
	data.ReadBlock(blockId, loadedData);

	return std::unique_ptr<BTreeNode<T>>(new BTreeNode<T>(loadedData));
}

// Tell the compiler for what types to compile the class.
template class BTree<int>;
