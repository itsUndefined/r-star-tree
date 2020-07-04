#include <iostream>
#include <queue>

#include "Data.h"
#include "BTree.h"
#include "RStarTree.h"

#include <io.h>
#include <fcntl.h>


int main() {
	_setmode(_fileno(stdout), _O_U16TEXT);
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

	//int min[2] = { 0, 0 };
	//int max[2] = { 2, 2 };

	//RStar::Key<int> a(min, max, 1, 2);

	//int point[2] = { 5, 1 };

	//auto result = a.areaEnlargementRequiredToFit(point);

	//auto result1 = *a.getEnlargedToFit(point);


	/*
	pugi::xml_document doc;
	pugi::xml_parse_result result = doc.load_file("map.osm");
	if (!result) {
		return -1;
	}

	auto nodes = doc.child(L"osm").children(L"node");

	for (auto& node : nodes) {
		for (auto& tag : node.children(L"tag")) {
			std::wstring key(tag.attribute(L"k").as_string());
			if (key == L"name") {
				std::wcout << tag.attribute(L"v").as_string() << std::endl;
			}
		}
		//std::cout << node.attribute("id").as_int() << std::endl;
		//std::cout << node.attribute("lat").as_double() << std::endl;
		//std::cout << node.attribute("lon").as_double() << std::endl;
		//return 0;
	}

	
	return 0;
	*/

	/*
	Data data;
	data.PrintData();


	data.GetRecordBuilder()
		.InsertInteger(16)
		.InsertFloat(0.3f)
		.InsertFloat(0.4f)
		.InsertCharN(L"dddd")
		.BuildAndSave();

	*/


	RStarTree<int> magicBoy(2);
	int input[2] = { 3, 3 };
	//magicBoy.insertData(input);
	
	
	
	/*
	for (int i = 1; i < 6; i++) {
		int input[2] = { i, i };
		magicBoy.insertData(input);
	}
	*/




	return 0;
}
