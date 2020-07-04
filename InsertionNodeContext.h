#pragma once

#include <vector>

#include "RStarTreeNode.h"

namespace RStar {

	template<class T>
	class InsertionNodeContext
	{
	public:

		InsertionNodeContext(std::shared_ptr<RStarTreeNode<T>>& optimalNode, std::vector<std::shared_ptr<RStarTreeNode<T>>>& blockPath, std::vector<int>& keyIndexPath) {
			this->optimalNode = optimalNode; // TODO Move instead of assign for better performance
			this->blockPath = blockPath;
			this->keyIndexPath = keyIndexPath;
		}

		std::shared_ptr<RStarTreeNode<T>> optimalNode;
		std::vector<std::shared_ptr<RStarTreeNode<T>>> blockPath;
		std::vector<int> keyIndexPath;
	};
}
