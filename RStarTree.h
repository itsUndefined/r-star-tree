#pragma once

#include <memory>


#include "File.h"
#include "RStarTreeNode.h"

using namespace RStar;


class RStarTree
{
public:
	RStarTree(int dimensions);
	void search(int* min, int* max);
	void insert(int* val);

private:
	void search(int* min, int* max, std::shared_ptr<RStarTreeNode> block);
	std::unique_ptr<RStarTreeNode> loadBlock(int blockId);

	File data;
	int rootId;
	int dimensions;

	std::shared_ptr<RStarTreeNode> root;
};


