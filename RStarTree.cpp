#include "RStarTree.h"

#include <fstream>
#include <iterator>

using namespace RStar;

template<class T>
RStarTree<T>::RStarTree(int dimensions): data(L"rtree.bin"), buffer(&data, 32, dimensions) {
	auto blockCount = data.GetBlockCount();

	if (blockCount == 1) {
		char* tempBuffer = new char[BLOCK_SIZE];
		data.ReadBlock(1, tempBuffer);
		if (*(int*)tempBuffer == INT_MAX) {
			*(int*)tempBuffer = 1;
			data.SaveBlock(1, tempBuffer);
		}
		delete[] tempBuffer;
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
std::priority_queue<Key<T>,	std::vector<Key<T>>, std::function<bool(Key<T>&, Key<T>&)>> RStarTree<T>::kNNSearch(T* point, int k) {
	std::shared_ptr<RStarTreeNode<T>> loadedBlock = this->root;
	std::shared_ptr<Key<T>> fromPoint(new Key<T>(point, INT_MAX, dimensions));
	std::priority_queue<
		Key<T>,
		std::vector<Key<T>>,
		std::function<bool(Key<T>&, Key<T>&)>
	> kNN([fromPoint](Key<T> &a, Key<T> &b) { return a.minEdgeDistanceFromPoint(*fromPoint) < b.minEdgeDistanceFromPoint(*fromPoint); });
	std::priority_queue<std::pair<Key<T>, bool>, std::vector<std::pair<Key<T>, bool>>, std::function<bool(std::pair<Key<T>, bool>&, std::pair<Key<T>, bool>&)>> pq([fromPoint](std::pair<Key<T>, bool> &a, std::pair<Key<T>, bool> &b) { return a.first.minEdgeDistanceFromPoint(*fromPoint) > b.first.minEdgeDistanceFromPoint(*fromPoint); });
	while (true) {
		for (auto& node : *loadedBlock) {
			pq.emplace(node, loadedBlock->isLeaf());
		}
		while (!pq.empty()) {
			if (pq.top().second) {
				if (kNN.size() < k) {
					kNN.emplace(std::move(pq.top().first));
					pq.pop();
				}
				else {
					if (pq.top().first.minEdgeDistanceFromPoint(*fromPoint) < kNN.top().minEdgeDistanceFromPoint(*fromPoint)) {
						kNN.pop();
						kNN.emplace(std::move(pq.top().first));
					}
					pq.pop();
				}
			}
			else {
				if (!kNN.empty()) {
					if (pq.top().first.minEdgeDistanceFromPoint(*fromPoint) < kNN.top().minEdgeDistanceFromPoint(*fromPoint)) {
						loadedBlock = this->loadBlock(pq.top().first.blockPtr);
						pq.pop();
						break;
					}
					else {
						pq.pop();
					}
				}
				else {
					loadedBlock = this->loadBlock(pq.top().first.blockPtr);
					pq.pop();
					break;
				}
			}
		}
		if (pq.empty()) {
			return kNN;
		}
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

	static int count = 0;
	count++;


	for (int i = result.blockPath.size() - 1; i >= -1; i--) {
		if (!node->isFull()) {
			node->insert(val);
			buffer.WriteNode(node->getBlockId(), node);
			for (int j = i; j >= 0; j--) {
				auto& parentBlock = result.blockPath[j];
				auto& parentKey = parentBlock->getKeys().at(result.keyIndexPath[j]);
				parentKey.enlargeToFit(val);
				buffer.WriteNode(parentBlock->getBlockId(), parentBlock);
			}
			break;
		}
		else {
			node->insert(val);
			auto parentForRightBlock = overflowTreatment(node, visitedLevels);
			if (parentForRightBlock != nullptr) {
				if (i == -1) {
					std::shared_ptr<RStarTreeNode<T>> parentBlock = std::shared_ptr<RStarTreeNode<T>>(
						new RStarTreeNode<T>(dimensions, false, nextBlockId)
					);
					parentBlock->insert(*node->getBoundingBox());
					parentBlock->insert(*parentForRightBlock);
					buffer.WriteNode(parentBlock->getBlockId(), parentBlock);

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
		std::shared_ptr<RStarTreeNode<T>> rightBlock = node->split();
		buffer.WriteNode(node->getBlockId(), node);
		buffer.WriteNode(nextBlockId, rightBlock);
		auto bb = rightBlock->getBoundingBox();
		bb->blockPtr = nextBlockId++;
		return bb;
	}
}

template<class T>
void RStarTree<T>::reInsert(std::shared_ptr<RStarTreeNode<T>> node, std::unordered_set<int>& visitedLevels) {
	auto bb = node->getBoundingBox();


	std::sort(node->begin(), node->end(), [&](Key<T>& a, Key<T>& b) {
		return bb->distanceFromRectCenter(a) < bb->distanceFromRectCenter(b);
	});

	int M = BLOCK_SIZE / Key<T>::GetKeySize(dimensions);
	int p = 0.3 * M;

	std::vector<Key<T>> deletedNodes;
	deletedNodes.assign(std::make_move_iterator(node->end() - p), std::make_move_iterator(node->end()));
	node->getKeys().erase(node->end() - p, node->end());


	buffer.WriteNode(node->getBlockId(), node); // Are we certain everything will work fine with reinsert? I don't like this


	for (auto& deletedNode : deletedNodes) {
		insert(deletedNode, visitedLevels, node->level);
	}

}

template<class T>
std::shared_ptr<RStarTreeNode<T>> RStarTree<T>::loadBlock(int blockId) {
	return buffer.ReadNode(blockId);
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
			return InsertionNodeContext<T>(N, std::move(blockPath), keyIndexPath);
		}

		auto child = loadBlock(N->begin()->blockPtr);

		T minOverlap = std::numeric_limits<T>::max();
		T minAreaEnlargmement = std::numeric_limits<T>::max();
		T minArea;
		Key<T>* chosenKey = nullptr;

		if (child->isLeaf()) {
			typedef std::pair<T, Key<T>*> Pair;

			std::priority_queue<
				Pair,
				std::vector<Pair>,
				std::function<bool(Pair&, Pair&)>
			> pq([&](Pair& a, Pair& b) { return a.first > b.first; });

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
			return InsertionNodeContext<T>(N, std::move(blockPath), keyIndexPath);;
		}
		continue;
	}

}

// Tell the compiler for what types to compile the class.
template class RStarTree<int>;
template class RStarTree<float>;
template class RStarTree<double>;
