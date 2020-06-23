#pragma once

#include <vector>
#include <memory>
#include <functional>
#include <algorithm>

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
			this->max = nullptr;
			std::copy(values, values + size, this->min);
			this->blockPtr = blockPtr;
			this->size = size;
		}

		Key(const Key<T>& source) {
			if (source.max != nullptr) {
				this->max = new T[source.size];
				std::copy(source.max, source.max + source.size, this->max);
			} else {
				this->max = nullptr;
			}

			this->min = new T[source.size];
			std::copy(source.min, source.min + source.size, this->min);
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

		Key<T>& operator=(const Key<T>& source) {
			int b = 1;
			return *this;
		}
		*/


		~Key() {
			if (this->min != nullptr) {
				delete[] this->min;
			}
			if (this->max != nullptr) {
				delete[] this->max;
			}
		}

		bool overlaps(T* min, T* max) {
			for (int i = 0; i < size; i++) {
				if (
					this->min[i] < min[i] && this->max[i] > min[i] ||
					this->min[i] < max[i] && this->max[i] > max[i]
				) {
					continue;
				} else {
					return false;
				}
			}
			return true;
		}

		double areaEnlargementRequiredToFit(T* point) {
			double thisArea = 1;
			for (int i = 0; i < size; i++) {
				thisArea *= this->max[i] - this->min[i];
			}

			double expandedArea = 1;

			for (int i = 0; i < size; i++) {
				if (this->max[i] >= point[i] && this->min[i] <= point[i]) {
					expandedArea *= this->max[i] - this->min[i];
				} else {
					int distFromMax = std::abs(this->max[i] - point[i]);
					int distFromMin = std::abs(this->min[i] - point[i]);

					expandedArea *= std::max(distFromMin, distFromMax);
				}
			}

			return expandedArea - thisArea;

		}

		std::unique_ptr<Key<T>> getEnlargedToFit(T* point) {
			std::unique_ptr<Key<T>> enlarged(new Key<T>(this->min, this->max, this->blockPtr, this->size));

			for (int i = 0; i < size; i++) {
				if (point[i] < this->min[i]) {
					enlarged->min[i] = point[i];
				} else if (point[i] > this->max[i]) {
					enlarged->max[i] = point[i];
				}
			}

			return enlarged;
		}

		double intersectArea(T* min, T* max) {
			double intersectingArea = 1;
			for (int i = 0; i < size; i++) {
				intersectingArea *= std::min(this->max[i], max[i]) - std::max(this->min[i], min[i]);
			}
			return intersectingArea;
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
		RStarTreeNode(int size, int dimensions, int leaf, int blockId);
		RStarTreeNode(char* diskData, int dimensions, int leaf, int blockId);
		void insert(int* val);
		double overlap(Key<int>& key);
		bool isLeaf();
		bool isFull();
		bool getBlockId() { return blockId; }

		//Used for forEach functionality
		std::vector<Key<int>>::iterator begin() { return data.begin(); }
		std::vector<Key<int>>::const_iterator begin() const { return data.begin(); }
		std::vector<Key<int>>::iterator end() { return data.end(); }
		std::vector<Key<int>>::const_iterator end() const { return data.end(); }

		std::unique_ptr<char[]> getRawData();
	private:
		bool leaf;
		int dimensions;
		int blockId;
		std::vector<Key<int>> data;
	};
}



