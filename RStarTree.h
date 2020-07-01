#pragma once

#include "RStarTreeNode.h"
#include "InsertionNodeContext.h"
#include <memory>
#include "File.h"
#include <unordered_set>
#include <utility>

class RStarTree
{
public:
	RStarTree(int dimensions);
	void rangeSearch(int* min, int* max);
	std::vector<RStar::Key<int>> kNNSearch(int* min, int* max, int k);
	void insertData(int* val);

private:
	void search(RStar::Key<int>& rangeSearch, std::shared_ptr<RStar::RStarTreeNode> block);
	std::unique_ptr<RStar::RStarTreeNode> loadBlock(int blockId);

	void insert(RStar::Key<int>& val, std::unordered_set<int>& visitedLevels, int level);
	std::unique_ptr<RStar::Key<int>> overflowTreatment(std::shared_ptr<RStar::RStarTreeNode> node, std::unordered_set<int>& visitedLevels);
	void reInsert(std::shared_ptr<RStar::RStarTreeNode> node, std::unordered_set<int>& visitedLevels);
	RStar::InsertionNodeContext chooseSubtree(RStar::Key<int>& val, int requiredLevel);

	File data;
	int rootId;
	int nextBlockId;
	int dimensions;

	std::shared_ptr<RStar::RStarTreeNode> root;
};


