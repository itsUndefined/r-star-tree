#include "RStarTree.h"

#include <fstream>

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
				}
			}
		}

		return;
	}

	for (auto& key : *block) {
		if (key.overlaps(min, max)) {
			this->search(min, max, this->loadBlock(key.leftBlockPtr));
		}
	}
}

void RStarTree::insert(int* val) {
	this->root->insert(val);
	data.SaveBlock(this->rootId, this->root->getRawData().get());
}

std::unique_ptr<RStarTreeNode> RStarTree::loadBlock(int blockId) {
	char* loadedData = new char[BLOCK_SIZE];
	data.ReadBlock(blockId, loadedData);

	if (loadedData[0] == 0) { // Not Leaf
		
	} else { // Leaf

	}
	
	auto ptr =  std::unique_ptr<RStarTreeNode>(new RStarTreeNode(loadedData + 4, dimensions));
	delete[] loadedData;
	return ptr;
}