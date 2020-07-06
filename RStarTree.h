#pragma once

#include "RStarTreeNode.h"
#include "InsertionNodeContext.h"
#include <memory>
#include "File.h"
#include "MemoryBuffer.h"
#include <unordered_set>
#include <utility>
#include <queue>

// this class creates the R*Tree according to the original paper
template<class T>
class RStarTree
{
public:
	RStarTree(int dimensions); // sets the dimensions of the R*Tree, exp.: RStarTree(2) creates a two dimensions R*Tree
	// perform a range search query in the R*Tree
	std::vector<RStar::Key<T>> rangeSearch(T* min, T* max);
	// perform a k-NN query in the R*Tree
	std::priority_queue<RStar::Key<T>, std::vector<RStar::Key<T>>, std::function<bool(RStar::Key<T>&, RStar::Key<T>&)>> kNNSearch(T* min, int k);
	// inserts data into the R*Tree
	void insertData(T* val, int blockId);


private:
	std::vector<RStar::Key<T>> search(RStar::Key<T>& rangeSearch, std::shared_ptr<RStar::RStarTreeNode<T>> block);
	// loads a specific block (parameter) from the file
	std::shared_ptr<RStar::RStarTreeNode<T>> loadBlock(int blockId);
	// implements Insert Algorithm, inserts a node or data to the R*Tree
	void insert(RStar::Key<T>& val, std::unordered_set<int>& visitedLevels, int level);
	// implements OverflowTreatment Algorithm
	std::unique_ptr<RStar::Key<T>> overflowTreatment(std::shared_ptr<RStar::RStarTreeNode<T>> node, std::unordered_set<int>& visitedLevels);
	// implements ReInsert Algorithm
	void reInsert(std::shared_ptr<RStar::RStarTreeNode<T>> node, std::unordered_set<int>& visitedLevels);
	// implements ChooseSubtree Algorithm
	RStar::InsertionNodeContext<T> chooseSubtree(RStar::Key<T>& val, int requiredLevel);

	File data;
	MemoryBuffer<T> buffer;

	int rootId;
	int nextBlockId;
	int dimensions;

	std::shared_ptr<RStar::RStarTreeNode<T>> root;
};


