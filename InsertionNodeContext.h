#pragma once

#include <vector>

#include "RStarTreeNode.h"

namespace RStar {

	template<class T>
	class InsertionNodeContext
	{
	public:

		InsertionNodeContext(std::shared_ptr<RStarTreeNode<T>>& optimalNode, std::vector<std::shared_ptr<RStarTreeNode<T>>>&& blockPath, std::vector<int>& keyIndexPath) {
			this->optimalNode = optimalNode;
			this->blockPath = std::move(blockPath);
			this->keyIndexPath = keyIndexPath;
		}

		std::shared_ptr<RStarTreeNode<T>> optimalNode;
		std::vector<std::shared_ptr<RStarTreeNode<T>>> blockPath;
		std::vector<int> keyIndexPath;
	};
}
