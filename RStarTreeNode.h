#pragma once

#include <vector>
#include <memory>
#include <functional>
#include <algorithm>
#include <cmath>
#include <iostream>

// this class creates a data, that data can be a point or rectangle
namespace RStar {
	template<class T>
	class Key {
	public:
		Key() {
			min = nullptr;
			max = nullptr;
		}

		// return the the sieze of the key
		static int GetKeySize(int dimensions) {
			return sizeof(int) + dimensions * 2 * sizeof(T);
		}

		Key(T* min, T* max, int blockPtr, int size) {
			this->min = new T[size];
			this->max = new T[size];
			std::copy(min, min + size, this->min);
			std::copy(max, max + size, this->max);
			this->blockPtr = blockPtr;
			this->size = size;
		}

		Key(T* values, int blockPtr, int size) {
			this->min = new T[size];
			this->max = new T[size];
			std::copy(values, values + size, this->min);
			std::copy(values, values + size, this->max);
			this->blockPtr = blockPtr;
			this->size = size;
		}

		Key(const Key<T>& source) {
			this->min = new T[source.size];
			std::copy(source.min, source.min + source.size, this->min);
			this->max = new T[source.size];
			std::copy(source.max, source.max + source.size, this->max);
			this->blockPtr = source.blockPtr;
			this->size = source.size;
		}

		Key(Key<T>&& source) {
			this->min = source.min;
			this->max = source.max;
			this->blockPtr = source.blockPtr;
			this->size = source.size;

			source.min = nullptr;
			source.max = nullptr;
		}

		Key<T>& operator=(const Key<T>& source) {
			if (this->min != nullptr) {
				delete[] this->min;
			}
			if (this->max != nullptr) {
				delete[] this->max;
			}
			this->min = new T[source.size];
			std::copy(source.min, source.min + source.size, this->min);
			this->max = new T[source.size];
			std::copy(source.max, source.max + source.size, this->max);
			this->blockPtr = source.blockPtr;
			this->size = source.size;

			return *this;
		}

		bool operator==(const Key<T>& rhs) {
			for (int i = 0; i < this->size; i++) {
				if (this->min[i] != rhs.min[i] || this->max[i] != rhs.max[i]) {
					return false;
				}
			}

			return true;
		}

		~Key() {
			if (this->min != nullptr) {
				delete[] this->min;
			}
			if (this->max != nullptr) {
				delete[] this->max;
			}
		}

		// checks if a rectangle overlaps with another rectangle
		bool overlaps(Key<T>& key) {
			for (int i = 0; i < size; i++) {
				if (
					this->min[i] < key.min[i] && this->max[i] > key.min[i] ||
					this->min[i] < key.max[i] && this->max[i] > key.max[i]
				) {
					continue;
				} else {
					return false;
				}
			}
			return true;
		}

        // return the area that the rectangle must expand inorder to fit
		T areaEnlargementRequiredToFit(Key<T>& key) {
			T thisArea = 1;
			for (int i = 0; i < size; i++) {
				thisArea *= this->max[i] - this->min[i];
			}

			T expandedArea = 1;

			for (int i = 0; i < size; i++) {
				if (this->max[i] >= key.max[i] && this->min[i] <= key.min[i]) {
					expandedArea *= this->max[i] - this->min[i];
				} else {
					T distFromMax = std::abs(this->max[i] - key.max[i]);
					T distFromMin = std::abs(this->min[i] - key.min[i]); //TODO check some results

					expandedArea *= std::max(distFromMin, distFromMax);
				}
			}

			return expandedArea - thisArea;
		}

		// returns the expanded rectangle
		std::unique_ptr<Key<T>> getEnlargedToFit(Key<T>& key) {
			std::unique_ptr<Key<T>> enlarged(new Key<T>(this->min, this->max, this->blockPtr, this->size));

			for (int i = 0; i < size; i++) {
				if (key.min[i] < this->min[i]) {
					enlarged->min[i] = key.min[i];
				} 
				if (key.max[i] > this->max[i]) {
					enlarged->max[i] = key.max[i];
				}
			}

			return enlarged;
		}

		// expands the rectangle
		void enlargeToFit(Key<T>& key) {
			for (int i = 0; i < size; i++) {
				if (key.min[i] < this->min[i]) {
					this->min[i] = key.min[i];
				}
				if (key.max[i] > this->max[i]) {
					this->max[i] = key.max[i];
				}
			}
		}

		// return the intersect area between two rectangles
		T intersectArea(Key<T>& key) {
			T intersectingArea = 1;
			for (int i = 0; i < size; i++) {
				T intersectEdge = std::min(this->max[i], key.max[i]) - std::max(this->min[i], key.min[i]);
				if (intersectEdge <= 0) {
					return 0;
				}
				intersectingArea *= intersectEdge;
			}
			return intersectingArea;
		}

		// return the destance from the center of the rectangle
		double distanceFromRectCenter(Key<T>& key) {
			double distanceSqr = 0;
			for (int i = 0; i < size; i++) {
				double thisCenter = (this->max[i] + this->min[i]) / 2.0;
				double center = (key.max[i] + key.min[i]) / 2.0;

				distanceSqr += std::pow(center - thisCenter, 2);
			}
			return std::sqrt(distanceSqr);
		}

		// return the margin value of the rectangle
		T marginValue() {
			T sum = 0;
			for (int i = 0; i < size; i++) {
				sum += this->max[i] - this->min[i];
			}
			return sum;
		}

		// return the distance from the rectangle edge
		double minEdgeDistanceFromPoint(Key<T>& point) const {
			double distance = 0;
			for (int i = 0; i < size; i++) {
				if (point.min[i] > this->min[i] && point.min[i] < this->max[i]) {
					continue;
				}
				
				distance += std::min(std::abs(this->min[i] - point.min[i]), std::abs(this->max[i] - point.min[i]));
			}
			return distance;
		}

		double maxEdgeDistanceFromPoint(Key<T>& point) const {
			double distance = 0;
			for (int i = 0; i < size; i++) {
				distance += std::max(std::abs(this->min[i] - point.min[i]), std::abs(this->max[i] - point.min[i]));
			}
			return distance;
		}

		// return the area value of the rectangle
		T areaValue() {
			T val = 1;
			for (int i = 0; i < size; i++) {
				val *= this->max[i] - this->min[i];
			}
			return val;
		}

		int blockPtr;
		T* min;
		T* max;
		//bool <(Key target) {
			//return this->key < target;
		//	return true;
		//}

	private:
		int size;
	};

	
	// the class creates a node for the data structure of R*Tree
	template<class T>
	class RStarTreeNode
	{
	public:
		RStarTreeNode(int dimensions, int leaf, int blockId);
		RStarTreeNode(
			std::move_iterator<typename std::vector<Key<T>>::iterator> begin,
			std::move_iterator<typename std::vector<Key<T>>::iterator> end,
			int dimensions, int leaf, int blockId
		);
		RStarTreeNode(char* diskData, int dimensions, int leaf, int blockId);

		// insert a data or node thge the node
		void insert(Key<T>& key);
		// implements Split Algorithm
		std::unique_ptr<RStarTreeNode> split();
		// checks if a rectangle overlap another rectangle
		T overlap(Key<T>& key);
		// checks if a node is a leaf
		bool isLeaf();
		// checks if a node is full
		bool isFull();
		// returns the bounding rectangle of rectangles
		std::unique_ptr<Key<T>> getBoundingBox();
		// returns the bounding rectangle of rectangles starting from a specific entry and ending to another entry
		std::unique_ptr<Key<T>> getBoundingBox(int start, int end);
		// returns the block id
		int getBlockId() { return blockId; }
		void setBlockId(int blockId) { this->blockId = blockId; }
		

		//Used for forEach functionality
		typename std::vector<Key<T>>::iterator begin() { return data.begin(); }
		typename std::vector<Key<T>>::const_iterator begin() const { return data.begin(); }
		typename std::vector<Key<T>>::iterator end() { return data.end(); }
		typename std::vector<Key<T>>::const_iterator end() const { return data.end(); }

		std::vector<Key<T>>& getKeys() { return data; };

		std::unique_ptr<char[]> getRawData();

		

		int level = 0;
	private:
		// implements ChooseSplitAxis Algorithm
		int chooseSplitAxis();
		// implements ChooseSplitIndex Algorithm
		int chooseSplitIndex(int axis);
		
		bool leaf;
		int dimensions;
		int blockId;
		std::vector<Key<T>> data;
	};
}



