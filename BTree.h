#pragma once

#include <memory>

#include "File.h"
#include "BTreeNode.h"

template<class T> 
class BTree
{
public:
	BTree();
	void search(T val);
	void insert(T val);
	int rootId;
private:
	File data;

	std::unique_ptr<BTreeNode<T>> loadBlock(int blockId);
	
	int nextBlockId;
	std::shared_ptr<BTreeNode<T>> root;
};

