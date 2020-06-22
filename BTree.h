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
	
private:
	File data;

	std::unique_ptr<BTreeNode<T>> loadBlock(int blockId);
	
	int nextBlockId;
	int rootId;
	std::shared_ptr<BTreeNode<T>> root;
};

