#pragma once

struct Node;

struct Rectangular
{
	Node* child;
};

struct Node
{
	bool isLeaf;
	Rectangular rectangular;
};

// the nodes in a block, it will be an array
struct ListNode
{
	Node* node;
	ListNode* next;
};

class RStarTree
{
public:
	RStarTree();
	void search();
	void insert();
};

