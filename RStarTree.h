#pragma once

#include "RStarTreeNode.h"
#include <memory>
#include "File.h"
#include <unordered_set>

using namespace RStar;


class RStarTree
{
public:
	RStarTree(int dimensions);
	void search(int* min, int* max);
	void insertData(int* val);

private:
	void search(int* min, int* max, std::shared_ptr<RStarTreeNode> block);
	std::unique_ptr<RStarTreeNode> loadBlock(int blockId);

	void insert(int* val, std::unordered_set<int>& visitedLevels);
	void overflowTreatment(std::shared_ptr<RStarTreeNode> node, std::unordered_set<int>& visitedLevels);
	void reInsert(std::shared_ptr<RStarTreeNode> node);
	std::shared_ptr<RStarTreeNode> chooseLeaf(int* val);

	File data;
	int rootId;
	int dimensions;

	std::shared_ptr<RStarTreeNode> root;
};


