#include "RStarTree.h"

#include <fstream>
#include <queue>
#include <iterator>

using namespace RStar;

RStarTree::RStarTree(int dimensions): data(L"rtree.bin") {
	auto blockCount = data.GetBlockCount();

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

void RStarTree::rangeSearch(int* min, int* max) {
	std::shared_ptr<RStarTreeNode> loadedBlock = this->root;
	Key<int> rangeSearch(min, max, INT_MAX, dimensions);
	this->search(rangeSearch, loadedBlock);
}

void RStarTree::search(Key<int>& rangeSearch, std::shared_ptr<RStarTreeNode> block) {
	if (block->isLeaf()) {
		for (auto& point : *block) {
			for (int i = 0; i < dimensions; i++) {
				if (rangeSearch.min[i] < point.min[i] && rangeSearch.max[i] > point.min[i]) {
					// Print point to output
					int i = 1;
				}
			}
		}

		return;
	}

	for (auto& key : *block) {
		if (key.overlaps(rangeSearch)) {
			this->search(rangeSearch, this->loadBlock(key.blockPtr));
		}
	}
}

void RStarTree::insertData(int* val) {
	std::unordered_set<int> visitedLevels;
	Key<int> key(val, INT_MAX, dimensions);
	insert(key, visitedLevels, INT_MAX);
}

void RStarTree::insert(Key<int>& val, std::unordered_set<int>& visitedLevels, int level) {
	//this->root->insert(val);
	//data.SaveBlock(this->rootId, this->root->getRawData().get());



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
				auto& parentBlock = result.blockPath[i];
				auto& parentKeyOfLeftBlock = parentBlock->getKeys().at(result.keyIndexPath[i]);
				parentKeyOfLeftBlock = *node->getBoundingBox();

				val = *parentForRightBlock;
				node = parentBlock;
				continue;
			} else {
				break; // What the hell happened on reinsert? Did everything get rebalanced happily?
			}
		}
	}

	

}

std::unique_ptr<Key<int>> RStarTree::overflowTreatment(std::shared_ptr<RStarTreeNode> node, std::unordered_set<int>& visitedLevels) {
	if (node->level != 0 && visitedLevels.find(node->level) == visitedLevels.end()) {
		visitedLevels.insert(node->level);
		reInsert(node, visitedLevels);
		return std::unique_ptr<Key<int>>();
	} else {
		auto rightBlock = node->split();
		data.SaveBlock(node->getBlockId(), node->getRawData().get());
		data.SaveBlock(rightBlock->getBlockId(), rightBlock->getRawData().get());
		auto bb = rightBlock->getBoundingBox();
		bb->blockPtr = nextBlockId++;
		return bb;
	}
}

void RStarTree::reInsert(std::shared_ptr<RStarTreeNode> node, std::unordered_set<int>& visitedLevels) {
	auto bb = node->getBoundingBox();


	std::sort(node->begin(), node->end(), [&](Key<int> a, Key<int> b) {
		return bb->distanceFromRectCenter(a) < bb->distanceFromRectCenter(b);
	});

	int M = BLOCK_SIZE / Key<int>::GetKeySize(dimensions) - 1;
	int p = 0.3 * M;

	std::vector<Key<int>> deletedNodes;
	deletedNodes.assign(node->end() - p, node->end());
	node->getKeys().erase(node->end() - p, node->end());


	data.SaveBlock(node->getBlockId(), node->getRawData().get()); // Are we certain everything will work fine with reinsert? I don't like this


	for (auto& deletedNode : deletedNodes) {
		insert(deletedNode, visitedLevels, node->level);
	}

}

std::unique_ptr<RStarTreeNode> RStarTree::loadBlock(int blockId) {
	char* loadedData = new char[BLOCK_SIZE];
	data.ReadBlock(blockId, loadedData);

	bool leaf = *(int*)loadedData == 1;
	
	auto ptr = std::unique_ptr<RStarTreeNode>(new RStarTreeNode(loadedData + 4, dimensions, leaf, blockId));
	delete[] loadedData;
	return ptr;
}

InsertionNodeContext RStarTree::chooseSubtree(Key<int>& val, int requiredLevel) {

	std::vector<std::shared_ptr<RStarTreeNode>> blockPath;
	std::vector<int> keyIndexPath;

	auto N = this->root;

	int level = 0;

	while (true) {

		if (N->isLeaf()) {
			N->level = level;
			return InsertionNodeContext(N, blockPath, keyIndexPath);
		}

		auto child = loadBlock(N->begin()->blockPtr);

		int minimum = INT_MAX;
		Key<int>* chosenKey = nullptr;

		if (child->isLeaf()) {
			std::priority_queue<std::pair<int, Key<int>*>, std::vector<std::pair<int, Key<int>*>>, std::greater<std::pair<int, Key<int>*>>> pq;
			for (auto& node : *N) {
				pq.push(std::make_pair(node.areaEnlargementRequiredToFit(val), &node));
			}

			for (int i = 0; i < 32; i++) {
				auto& A = *pq.top().second;
				pq.pop();
				double beforeEnlargementOverlap = N->overlap(A);
				auto enlargedKey = A.getEnlargedToFit(val);
				double afterEnlargementOverlap = N->overlap(*enlargedKey);
				auto overlapEnlargement = afterEnlargementOverlap - beforeEnlargementOverlap;
				if (overlapEnlargement == minimum) {
					throw "We NEED to resolve TIES!!!!";
				}
				if (overlapEnlargement < minimum) {
					minimum = overlapEnlargement;
					chosenKey = &A;
				}
			}
		}

		if (!child->isLeaf()) {
			for (auto& node : *N) {
				auto areaEnlargmentRequired = node.areaEnlargementRequiredToFit(val);
				if (areaEnlargmentRequired == minimum) {
					throw "We NEED to resolve TIES!!!!";
				}
				if (areaEnlargmentRequired < minimum) {
					minimum = areaEnlargmentRequired;
					chosenKey = &node;
				}
			}
		}
		blockPath.push_back(N);
		keyIndexPath.push_back(std::distance(N->begin(), std::find(N->begin(), N->end(), *chosenKey)));
		N = this->loadBlock(chosenKey->blockPtr);
		level++;
		if (requiredLevel == level) {
			return InsertionNodeContext(N, blockPath, keyIndexPath);;
		}
		continue;
	}

}