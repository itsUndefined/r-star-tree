#include "RStarTree.h"

#include <fstream>
#include <iterator>
#include <queue>

using namespace RStar;

template<class T>
RStarTree<T>::RStarTree(int dimensions): data(L"rtree.bin") {
	auto blockCount = data.GetBlockCount();

	if (blockCount == 1) {
		char* tempBuffer = new char[BLOCK_SIZE];
		data.ReadBlock(1, tempBuffer);
		if (*(int*)tempBuffer == INT_MAX) {
			*(int*)tempBuffer = 1;
			data.SaveBlock(1, tempBuffer);
		}
	}

	this->nextBlockId = blockCount + 1;

	std::fstream metadataFile;

	metadataFile.open("rtree_metadata.bin", std::ios::out | std::ios::app);
	metadataFile.close();
	metadataFile.open("rtree_metadata.bin", std::ios::binary | std::ios::in | std::ios::out);

	metadataFile.seekp(0, std::ios::end);
	if (metadataFile.tellg() == 0) {
		this->rootId = 1;
		metadataFile.write((char*) &this->rootId, sizeof(int));
	} else {
		metadataFile.seekg(0);
		metadataFile.read((char*) &this->rootId, sizeof(int));
	}

	metadataFile.close();

	this->dimensions = dimensions;
	this->root = this->loadBlock(this->rootId);
}

template<class T>
std::vector<Key<T>> RStarTree<T>::rangeSearch(T* min, T* max) {
	std::shared_ptr<RStarTreeNode<T>> loadedBlock = this->root;
	Key<T> rangeSearch(min, max, INT_MAX, dimensions);
	return this->search(rangeSearch, loadedBlock);
}

template<class T>
std::vector<Key<T>> RStarTree<T>::search(Key<T>& rangeSearch, std::shared_ptr<RStarTreeNode<T>> block) {
	std::vector<Key<T>> list;
	if (block->isLeaf()) {
		for (auto& point : *block) {
			bool inRange = true;
			for (int i = 0; i < dimensions; i++) {
				if (rangeSearch.min[i] > point.min[i] || rangeSearch.max[i] < point.min[i]) {
					inRange = false;
				}
			}
			if (inRange) {
				list.push_back(point);
			}
		}

		return list;
	}

	for (auto& key : *block) {
		if (key.overlaps(rangeSearch)) {
			this->search(rangeSearch, this->loadBlock(key.blockPtr));
		}
	}
}

template<class T>
std::vector<Key<T>> RStarTree<T>::kNNSearch(T* point, int k) {
	std::shared_ptr<RStarTreeNode<T>> loadedBlock = this->root;
	Key<T> fromPoint(point, INT_MAX, dimensions);
	std::vector<Key<T>> kNN;
	std::priority_queue<Key<T>, std::vector<Key<T>>, std::function<bool(Key<T>&, Key<T>&)>> points([&](Key<T> &a, Key<T> &b) { return a.distanceFromEdge(fromPoint) > b.distanceFromEdge(fromPoint); });
	std::priority_queue<std::pair<Key<T>, bool>, std::vector<std::pair<Key<T>, bool>>, std::function<bool(std::pair<Key<T>, bool>&, std::pair<Key<T>, bool>&)>> pq([&](std::pair<Key<T>, bool> &a, std::pair<Key<T>, bool> &b) { return a.first.distanceFromEdge(fromPoint) > b.first.distanceFromEdge(fromPoint); });
	while (true) {
		for (auto& node : *loadedBlock) {
			pq.emplace(node, loadedBlock->isLeaf());
		}
		while (!pq.empty() && pq.top().second) {
			points.emplace(std::move(pq.top().first));
			pq.pop();
		}
		if (pq.empty()) {
			for (int i = 0; i < points.size(); i++) {
				if (i == k) {
					break;
				}
				kNN.push_back(points.top());
				points.pop();
			}
			return kNN;
		}
		loadedBlock = this->loadBlock(pq.top().first.blockPtr);
		pq.pop();
	}
}

template<class T>
void RStarTree<T>::insertData(T* val, int blockId) {
	std::unordered_set<int> visitedLevels;
	Key<T> key(val, blockId, dimensions);
	insert(key, visitedLevels, INT_MAX);
}

template<class T>
void RStarTree<T>::insert(Key<T>& val, std::unordered_set<int>& visitedLevels, int level) {

	auto result = chooseSubtree(val, level);

	auto node = result.optimalNode;
	//auto path = result.keyIndexPath;


	for (int i = result.blockPath.size() - 1; i >= -1; i--) { // -1? What about split of root?
		if (!node->isFull()) {
			node->insert(val);
			data.SaveBlock(node->getBlockId(), node->getRawData().get());
			for (int j = i; j >= 0; j--) {
				auto& parentBlock = result.blockPath[j];
				auto& parentKey = parentBlock->getKeys().at(result.keyIndexPath[j]);
				parentKey.enlargeToFit(val);
				data.SaveBlock(parentBlock->getBlockId(), parentBlock->getRawData().get());
			}
			break;
		}
		else {
			node->insert(val);
			auto parentForRightBlock = overflowTreatment(node, visitedLevels);
			if (parentForRightBlock != nullptr) {
				if (i == -1) {
					std::shared_ptr<RStarTreeNode<T>> parentBlock = std::shared_ptr<RStarTreeNode<T>>(new RStarTreeNode<T>(dimensions, false, nextBlockId));
					parentBlock->insert(*node->getBoundingBox());
					parentBlock->insert(*parentForRightBlock);
					data.SaveBlock(parentBlock->getBlockId(), parentBlock->getRawData().get());

					this->root = parentBlock;
					this->rootId = nextBlockId;

					std::fstream metadataFile;
					metadataFile.open("rtree_metadata.bin", std::ios::binary | std::ios::in | std::ios::out);

					metadataFile.seekp(0);
					metadataFile.write((char*)&this->rootId, sizeof(int));


					nextBlockId++;
				}
				else {
					auto& parentBlock = result.blockPath[i];
					auto& parentKeyOfLeftBlock = parentBlock->getKeys().at(result.keyIndexPath[i]);
					parentKeyOfLeftBlock = *node->getBoundingBox();
					val = *parentForRightBlock;
					node = parentBlock;
				}
				continue;
			} else {
				break; // What the hell happened on reinsert? Did everything get rebalanced happily?
			}
		}
	}

	

}

template<class T>
std::unique_ptr<Key<T>> RStarTree<T>::overflowTreatment(std::shared_ptr<RStarTreeNode<T>> node, std::unordered_set<int>& visitedLevels) {
	if (node->level != 0 && visitedLevels.find(node->level) == visitedLevels.end()) {
		visitedLevels.insert(node->level);
		reInsert(node, visitedLevels);
		return std::unique_ptr<Key<T>>();
	} else {
		auto rightBlock = node->split();
		data.SaveBlock(node->getBlockId(), node->getRawData().get());
		data.SaveBlock(nextBlockId, rightBlock->getRawData().get());
		auto bb = rightBlock->getBoundingBox();
		bb->blockPtr = nextBlockId++;
		return bb;
	}
}

template<class T>
void RStarTree<T>::reInsert(std::shared_ptr<RStarTreeNode<T>> node, std::unordered_set<int>& visitedLevels) {
	auto bb = node->getBoundingBox();


	std::sort(node->begin(), node->end(), [&](Key<T> a, Key<T> b) {
		return bb->distanceFromRectCenter(a) < bb->distanceFromRectCenter(b);
	});

	int M = BLOCK_SIZE / Key<T>::GetKeySize(dimensions);
	int p = 0.3 * M;

	std::vector<Key<T>> deletedNodes;
	deletedNodes.assign(node->end() - p, node->end());
	node->getKeys().erase(node->end() - p, node->end());


	data.SaveBlock(node->getBlockId(), node->getRawData().get()); // Are we certain everything will work fine with reinsert? I don't like this


	for (auto& deletedNode : deletedNodes) {
		insert(deletedNode, visitedLevels, node->level);
	}

}

template<class T>
std::unique_ptr<RStarTreeNode<T>> RStarTree<T>::loadBlock(int blockId) {
	char* loadedData = new char[BLOCK_SIZE];
	data.ReadBlock(blockId, loadedData);

	bool leaf = *(int*)loadedData == 1;
	
	auto ptr = std::unique_ptr<RStarTreeNode<T>>(new RStarTreeNode<T>(loadedData + 4, dimensions, leaf, blockId));
	delete[] loadedData;
	return ptr;
}

template<class T>
InsertionNodeContext<T> RStarTree<T>::chooseSubtree(Key<T>& val, int requiredLevel) {

	std::vector<std::shared_ptr<RStarTreeNode<T>>> blockPath;
	std::vector<int> keyIndexPath;

	auto N = this->root;

	int level = 0;

	while (true) {

		if (N->isLeaf()) {
			N->level = level;
			return InsertionNodeContext<T>(N, blockPath, keyIndexPath);
		}

		auto child = loadBlock(N->begin()->blockPtr);

		T minOverlap = std::numeric_limits<T>::max();
		T minAreaEnlargmement = std::numeric_limits<T>::max();
		T minArea;
		Key<T>* chosenKey = nullptr;

		if (child->isLeaf()) {
			std::priority_queue<std::pair<T, Key<T>*>, std::vector<std::pair<T, Key<T>*>>, std::greater<std::pair<T, Key<T>*>>> pq; // TODO pointers? Safe?
			for (auto& node : *N) {
				pq.push(std::make_pair(node.areaEnlargementRequiredToFit(val), &node));
			}

			for (int i = 0; i < 32; i++) {
				auto& A = *pq.top().second;
				pq.pop();
				T beforeEnlargementOverlap = N->overlap(A);
				auto enlargedKey = A.getEnlargedToFit(val);
				T afterEnlargementOverlap = N->overlap(*enlargedKey);
				T overlapEnlargement = afterEnlargementOverlap - beforeEnlargementOverlap;
				T areaEnlargmement = A.areaEnlargementRequiredToFit(val);
				T area = A.areaValue();
				if (overlapEnlargement == minOverlap) {
					if (areaEnlargmement < minAreaEnlargmement) {
						minArea = area;
						minAreaEnlargmement = areaEnlargmement;
						minOverlap = overlapEnlargement;
						chosenKey = &A;
					}
					else if (areaEnlargmement == minAreaEnlargmement && area < minArea) {
						minArea = area;
						minAreaEnlargmement = areaEnlargmement;
						minOverlap = overlapEnlargement;
						chosenKey = &A;
					}
				}
				if (overlapEnlargement < minOverlap) {
					minArea = area;
					minAreaEnlargmement = areaEnlargmement;
					minOverlap = overlapEnlargement;
					chosenKey = &A;
				}
				if (pq.size() == 0) {
					break;
				}
			}
		}

		if (!child->isLeaf()) {
			for (auto& node : *N) {
				auto areaEnlargmentRequired = node.areaEnlargementRequiredToFit(val);
				T area = node.areaValue();
				if (areaEnlargmentRequired == minOverlap && area < minArea) {
					minArea = area;
					minAreaEnlargmement = areaEnlargmentRequired;
					chosenKey = &node;
				}
				if (areaEnlargmentRequired < minAreaEnlargmement) {
					minArea = area;
					minAreaEnlargmement = areaEnlargmentRequired;
					chosenKey = &node;
				}
			}
		}
		blockPath.push_back(N);
		auto end = std::find(N->begin(), N->end(), *chosenKey);
		auto distance = std::distance(N->begin(), end);
		keyIndexPath.push_back(distance);
		N = this->loadBlock(chosenKey->blockPtr);
		level++;
		if (requiredLevel == level) {
			return InsertionNodeContext<T>(N, blockPath, keyIndexPath);;
		}
		continue;
	}

}

// Tell the compiler for what types to compile the class.
template class RStarTree<int>;
template class RStarTree<float>;
template class RStarTree<double>;
