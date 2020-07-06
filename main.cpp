#include <iostream>
#include <queue>

#include "Data.h"
#include "BTree.h"
#include "RStarTree.h"

#include <io.h>
#include <fcntl.h>
#include <codecvt>

//#include <mongocxx/instance.hpp>
//#include <mongocxx/client.hpp>
//#include <mongocxx/uri.hpp>

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
	data.GetRecordBuilder()
		.InsertInteger(16)
		.InsertFloat(0.3f)
		.InsertFloat(0.4f)
		.InsertCharN(L"dddd")
		.BuildAndSave();
		*/

	/*
	mongocxx::instance instance{}; // This should be done only once.
	mongocxx::client client{ mongocxx::uri{"mongodb://localhost:27017"} };

	mongocxx::collection nodes = client["openstreetmaps"]["nodes"];

	mongocxx::options::find options;
	options.no_cursor_timeout(true);

	mongocxx::cursor cursor = nodes.find({}, options);
	
	std::wstring_convert<std::codecvt_utf8_utf16<wchar_t>> convert;


	int i = 0;

	Data data;
	
	for(auto& record : cursor) {
		auto node = record.find("node")->get_document().view();
		std::string id = node.find("@id")->get_utf8().value.to_string();
		std::string lat = node.find("@lat")->get_utf8().value.to_string();
		std::string lon = node.find("@lon")->get_utf8().value.to_string();
		int fId = std::stoi(id);
		float fLat = std::stof(lat);
		float fLon = std::stof(lon);
		boost::string_view name;
		if (node.find("tag") != node.end()) {
			if (node.find("tag")->type() == bsoncxx::type::k_array) {
				for (auto& tag : node.find("tag")->get_array().value) {
					if (tag.get_document().view().find("@k")->get_utf8().value.to_string() == "name") {
						name = tag.get_document().view().find("@v")->get_utf8().value;
						break;
					}
				}
			} else {
				auto tag = *node.find("tag");
				if (tag.get_document().view().find("@k")->get_utf8().value.to_string() == "name") {
					name = tag.get_document().view().find("@v")->get_utf8().value;
				}
			}
			
		}
		

		std::wstring fName;

		if (name.length()) {
			fName = convert.from_bytes(name.begin());
		}

	
		data.GetRecordBuilder()
			.InsertInteger(fId)
			.InsertCharN(fName)
			.InsertFloat(fLat)
			.InsertFloat(fLon)
			.BuildAndSave();


		if (i % 200000 == 0) {
			std::wcout << (i / 1173000000.0) * 100 << L"% complete with " << i << L"elements" << std::endl;
		}

		if (i == 20000000) {
			break;
		}

		i++;

	}

	std::wcout << L"Cursor exited" << std::endl;
	*/



	//RStarTree<int> magicBoy(2);
	//int input[2] = { 3, 3 };
	//magicBoy.insertData(input);
	
	
	
	/*
	for (int i = 1; i < 6; i++) {
		int input[2] = { i, i };
		magicBoy.insertData(input);
	}
	*/


	Data data;
	float min[2] = { 59.0, 30 };
	//float max[2] = { 60.0, 31 };
	data.KNNSearch(min, 10, true);
	data.KNNSearch(min, 10, false);





	return 0;
}
