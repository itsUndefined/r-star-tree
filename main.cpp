#include "BTree.h"
#include <iostream>


int main() {

	BTree<int> tree;
	
	for (int i = 1; i < 10000000; i++) {
			tree.search(i);
	}

	std::cout << tree.rootId << std::endl;
	return 0; 
}
