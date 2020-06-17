#pragma once

struct Rectangular
{
	//TODO: min-max coordinates of current rectangular
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

