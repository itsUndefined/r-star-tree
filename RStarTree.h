#pragma once

#include "RStarTreeNode.h"
#include "InsertionNodeContext.h"
#include <memory>
#include "File.h"
#include <unordered_set>
#include <utility>

// this class creates the R*Tree according to the original paper
class RStarTree
{
public:
	RStarTree(int dimensions); // sets the dimensions of the R*Tree, exp.: RStarTree(2) creates a two dimensions R*Tree
	// perform a range search query in the R*Tree
	std::vector<RStar::Key<int>> rangeSearch(int* min, int* max);
	// perform a k-NN query in the R*Tree
	std::vector<RStar::Key<int>> kNNSearch(int* min, int* max, int k);
	// inserts data into the R*Tree
	void insertData(int* val);

private:
	std::vector<RStar::Key<int>> search(RStar::Key<int>& rangeSearch, std::shared_ptr<RStar::RStarTreeNode> block);
	// loads a specific block (parameter) from the file
	std::unique_ptr<RStar::RStarTreeNode> loadBlock(int blockId);
	
	// implements Insert Algorithm, inserts a node or data to the R*Tree
	void insert(RStar::Key<int>& val, std::unordered_set<int>& visitedLevels, int level);
	// implements OverflowTreatment Algorithm
	std::unique_ptr<RStar::Key<int>> overflowTreatment(std::shared_ptr<RStar::RStarTreeNode> node, std::unordered_set<int>& visitedLevels);
	// implements ReInsert Algorithm
	void reInsert(std::shared_ptr<RStar::RStarTreeNode> node, std::unordered_set<int>& visitedLevels);
	// implements ChooseSubtree Algorithm
	RStar::InsertionNodeContext chooseSubtree(RStar::Key<int>& val, int requiredLevel);

	File data;
	int rootId;
	int nextBlockId;
	int dimensions;

	std::shared_ptr<RStar::RStarTreeNode> root;
};


