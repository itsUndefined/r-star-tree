#include "RStarTree.h"

#include <fstream>
#include <queue>

using namespace RStar;

RStarTree::RStarTree(int dimensions): data(L"rtree.bin") {
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

void RStarTree::search(int* min, int* max) {
	std::shared_ptr<RStarTreeNode> loadedBlock = this->root;
	this->search(min, max, loadedBlock);
}

void RStarTree::search(int* min, int* max, std::shared_ptr<RStarTreeNode> block) {
	if (block->isLeaf()) {
		for (auto& point : *block) {
			for (int i = 0; i < dimensions; i++) {
				if (min[i] < point.min[i] && max[i] > point.min[i]) {
					// Print point to output
					int i = 1;
				}
			}
		}

		return;
	}

	for (auto& key : *block) {
		if (key.overlaps(min, max)) {
			this->search(min, max, this->loadBlock(key.blockPtr));
		}
	}
}

void RStarTree::insert(int* val) {
	//this->root->insert(val);
	//data.SaveBlock(this->rootId, this->root->getRawData().get());
	auto node = chooseLeaf(val);
	if (!node->isFull()) {
		node->insert(val);
		data.SaveBlock(node->getBlockId(), node->getRawData().get());
	} else {

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

std::shared_ptr<RStarTreeNode> RStarTree::chooseLeaf(int* val) {
	auto N = this->root;

	while (true) {
		if (N->isLeaf()) {
			return N;
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

		N = this->loadBlock(chosenKey->blockPtr);
		continue;
	}

}