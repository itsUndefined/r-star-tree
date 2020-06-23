#include <iostream>
#include <queue>

#include "Data.h"
#include "BTree.h"
#include "RStarTree.h"


int main() {

	/*BTree<int> tree;
	
	for (int i = 1; i < 100000; i++) {
			tree.insert(i);
	}

	std::cout << tree.rootId << std::endl;*/

	//Data data;

	//data.InsertRow(std::unique_ptr<char>(new char[512]{ 'h', 'e', 'l', 'l', 'o', '\0', 'p', 10, 0, 0, 0 }));
	//data.PrintData();

	//RStarTree tree(1);

	//tree.search(new int[1]{ 0 }, new int[1]{ 20 });

	int min[2] = { 0, 0 };
	int max[2] = { 2, 2 };

	RStar::Key<int> a(min, max, 1, 2);

	int point[2] = { 5, 1 };

	auto result = a.areaEnlargementRequiredToFit(point);

	auto result1 = *a.getEnlargedToFit(point);

	return 0;
}
