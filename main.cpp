#include <iostream>
#include <queue>
#include <string>

#include "Data.h"
#include "BTree.h"
#include "RStarTree.h"

#include <io.h>
#include <fcntl.h>
#include <codecvt>


//#include <mongocxx/instance.hpp>
//#include <mongocxx/client.hpp>
//#include <mongocxx/uri.hpp>


int main(int argc, char** argv) {
	_setmode(_fileno(stdout), _O_U16TEXT); // Support UTF16 in windows

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

	*/

	//Data data;
	/*
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

		if (i == 100000) {
			break;
		}

		i++;

	}

	std::wcout << L"Cursor exited" << std::endl;
	*/

	Data data;
	if (argc > 1) {
		int queryType = std::stoi(argv[1]); // 0 for range. 1 for knn
		if (queryType == 0) { // If range search
			float min[2] = { std::stof(argv[2]), std::stof(argv[3]) };
			float max[2] = { std::stof(argv[4]), std::stof(argv[5]) };
			bool useIndex;
			if (std::string(argv[6]) == "with_index") {
				useIndex = true;
			} else if(std::string(argv[6]) == "without_index") {
				useIndex = false;
			} else {
				std::wcout << "Unknown parameter: " << argv[6] << std::endl;
				exit(1);
			}
			bool printResults;
			if (std::string(argv[7]) == "print_results") {
				printResults = true;
			}
			else if (std::string(argv[7]) == "print_count") {
				printResults = false;
			}
			else {
				std::wcout << "Unknown parameter: " << argv[7] << std::endl;
				exit(1);
			}
			data.RangeSearch(min, max, useIndex, printResults);
		}

		if (queryType == 1) {
			float point[2] = { std::stof(argv[2]), std::stof(argv[3]) };
			int k = std::stoi(argv[4]);
			bool useIndex;
			if (std::string(argv[5]) == "with_index") {
				useIndex = true;
			}
			else if (std::string(argv[5]) == "without_index") {
				useIndex = false;
			}
			bool printResults;
			if (std::string(argv[6]) == "print_results") {
				printResults = true;
			}
			else if (std::string(argv[6]) == "print_count") {
				printResults = false;
			}
			data.KNNSearch(point, k, useIndex, printResults);
		}
	}


	

	/* Code snippet 3
	Data data;
	data.GetRecordBuilder()
		.InsertInteger(1)
		.InsertCharN(L"Αυτό είναι ένα σχετικά μεγάλο δοκιμαστικό string με ελληνικούς χαρακτήρες")
		.InsertFloat(0.f)
		.InsertFloat(FLT_MAX)
		.BuildAndSave();
	*/

	return 0;
}
