#pragma once

#include <unordered_map>
#include <memory>
#include <list>

#include "File.h"
#include "RStarTreeNode.h"

template <class T>
class MemoryBuffer
{
public:
	MemoryBuffer(File* data, int size, int dimensions) {
		this->data = data;
		this->dimensions = dimensions;
		this->size = size;
		buffer.reserve(size);
	}

	std::shared_ptr<RStar::RStarTreeNode<T>> ReadNode(int blockId) {
		auto bufferedNode = buffer.find(blockId);
		if (bufferedNode == buffer.end()) {

			if (dq.size() == size) {
				auto old = dq.back();
				dq.pop_back();
				buffer.erase(old->getBlockId());
			}

			char* loadedData = new char[BLOCK_SIZE];
			data->ReadBlock(blockId, loadedData);

			bool leaf = *(int*)loadedData == 1;

			std::shared_ptr<RStar::RStarTreeNode<T>> const ptr = std::shared_ptr<RStar::RStarTreeNode<T>>(new RStar::RStarTreeNode<T>(loadedData + 4, dimensions, leaf, blockId));
			delete[] loadedData;
			dq.push_front(ptr);
			buffer[blockId] = dq.begin();
			return ptr;
		}



		dq.push_front(*bufferedNode->second);
		dq.erase(buffer[blockId]);
		buffer[blockId] = dq.begin();
		return *buffer[blockId];
	};

	void WriteNode(int blockId, const std::shared_ptr<RStar::RStarTreeNode<T>> node) {
		auto bufferedNode = buffer.find(blockId);
		node->setBlockId(blockId);
		if (bufferedNode == buffer.end()) {

			if (dq.size() == size) {
				auto old = dq.back();
				dq.pop_back();
				buffer.erase(old->getBlockId());
			}

			dq.push_front(node);
			buffer[blockId] = dq.begin();
			data->SaveBlock(blockId, node->getRawData().get());
			return;
		}
		
		dq.push_front(*bufferedNode->second);
		dq.erase(buffer[node->getBlockId()]);
		buffer[blockId] = dq.begin();
		data->SaveBlock(blockId, node->getRawData().get());
	};

private:
	File* data;
	std::list<std::shared_ptr<RStar::RStarTreeNode<T>>> dq;
	std::unordered_map<int, typename std::list<std::shared_ptr<RStar::RStarTreeNode<T>>>::iterator> buffer;
	int dimensions;
	int size;
};

