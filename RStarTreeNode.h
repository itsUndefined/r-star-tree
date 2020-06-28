#pragma once

#include <vector>
#include <memory>
#include <functional>
#include <algorithm>
#include <cmath>

namespace RStar {
	template<class T>
	class Key {
	public:
		Key() {
			min = nullptr;
			max = nullptr;
			blockPtr = 666;
		}

		static int GetKeySize(int dimensions) {
			return sizeof(int) + dimensions * 2 * sizeof(int);
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

		/*
		Key(Key<T>&& source) {
			this->min = source.min;
			this->max = source.max;
			this->leftBlockPtr = source.leftBlockPtr;
			this->size = source.size;

			source.min = nullptr;
			source.max = nullptr;
		}
		*/

		Key<T>& operator=(const Key<T>& source) {
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

		double areaEnlargementRequiredToFit(Key<T>& key) {
			double thisArea = 1;
			for (int i = 0; i < size; i++) {
				thisArea *= this->max[i] - this->min[i];
			}

			double expandedArea = 1;

			for (int i = 0; i < size; i++) {
				if (this->max[i] >= key.max[i] && this->min[i] <= key.min[i]) {
					expandedArea *= this->max[i] - this->min[i];
				} else {
					int distFromMax = std::abs(this->max[i] - key.max[i]);
					int distFromMin = std::abs(this->min[i] - key.min[i]); //TODO check some results

					expandedArea *= std::max(distFromMin, distFromMax);
				}
			}

			return expandedArea - thisArea;
		}

		std::unique_ptr<Key<T>> getEnlargedToFit(Key<T>& key) {
			std::unique_ptr<Key<T>> enlarged(new Key<T>(this->min, this->max, this->blockPtr, this->size));

			for (int i = 0; i < size; i++) {
				if (key.min[i] < this->min[i]) {
					enlarged->min[i] = key.min[i];
				} else if (key.max[i] > this->max[i]) {
					enlarged->max[i] = key.max[i];
				}
			}

			return enlarged;
		}

		void enlargeToFit(Key<T>& key) {
			for (int i = 0; i < size; i++) {
				if (key.min[i] < this->min[i]) {
					this->min[i] = key.min[i];
				}
				else if (key.max[i] > this->max[i]) {
					this->max[i] = key.max[i];
				}
			}
		}

		double intersectArea(Key<T>& key) {
			double intersectingArea = 1;
			for (int i = 0; i < size; i++) {
				auto intersectEdge = std::min(this->max[i], key.max[i]) - std::max(this->min[i], key.min[i]);
				if (intersectEdge <= 0) {
					return 0;
				}
				intersectingArea *= intersectEdge;
			}
			return intersectingArea;
		}

		double distanceFromRectCenter(Key<T>& key) {
			double distanceSqr = 0;
			for (int i = 0; i < size; i++) {
				T thisCenter = (this->max[i] + this->min[i]) / 2.0;
				T center = (key.max[i] + key.min[i]) / 2.0;

				distanceSqr += std::pow(center - thisCenter, 2);
			}
			return std::sqrt(distanceSqr);
		}

		double marginValue() {
			double sum = 0;
			for (int i = 0; i < size; i++) {
				sum += this->max[i] - this->min[i];
			}
			return sum;
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

	

	class RStarTreeNode
	{
	public:
		RStarTreeNode(int dimensions, int leaf, int blockId);
		RStarTreeNode(std::vector<Key<int>>::iterator begin, std::vector<Key<int>>::iterator end, int dimensions, int leaf, int blockId);
		RStarTreeNode(char* diskData, int dimensions, int leaf, int blockId);
		void insert(Key<int>& key);
		std::unique_ptr<RStarTreeNode> split();
		double overlap(Key<int>& key);
		bool isLeaf();
		bool isFull();
		std::unique_ptr<Key<int>> getBoundingBox();
		std::unique_ptr<Key<int>> getBoundingBox(int start, int end);
		int getBlockId() { return blockId; }
		

		//Used for forEach functionality
		std::vector<Key<int>>::iterator begin() { return data.begin(); }
		std::vector<Key<int>>::const_iterator begin() const { return data.begin(); }
		std::vector<Key<int>>::iterator end() { return data.end(); }
		std::vector<Key<int>>::const_iterator end() const { return data.end(); }

		std::vector<Key<int>>& getKeys() { return data; };

		std::unique_ptr<char[]> getRawData();

		

		int level = 0;
	private:
		int chooseSplitAxis();
		int chooseSplitIndex(int axis);
		
		bool leaf;
		int dimensions;
		int blockId;
		std::vector<Key<int>> data;
	};
}



