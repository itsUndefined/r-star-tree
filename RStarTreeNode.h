#pragma once

#include <vector>
#include <memory>
#include <functional>

namespace RStar {
	template<class T>
	class Key {
	public:
		Key() {
			min = nullptr;
			max = nullptr;
			leftBlockPtr = 666;
		}

		static int GetKeySize(int dimensions) {
			return sizeof(int) + dimensions * 2 * sizeof(int);
		}

		Key(T* min, T* max, int leftBlockPtr, int size) {
			this->min = new T[size];
			this->max = new T[size];
			std::copy(min, min + size, this->min);
			std::copy(max, max + size, this->max);
			this->leftBlockPtr = leftBlockPtr;
			this->size = size;
		}

		Key(T* values, int leftBlockPtr, int size) {
			this->min = new T[size];
			this->max = nullptr;
			std::copy(values, values + size, this->min);
			this->leftBlockPtr = leftBlockPtr;
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
			this->leftBlockPtr = source.leftBlockPtr;
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

		int leftBlockPtr;
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
		RStarTreeNode(int dimensions);
		RStarTreeNode(int size, int dimensions);
		RStarTreeNode(char* diskData, int dimensions);
		void insert(int* val);
		bool isLeaf();

		//Used for forEach functionality
		std::vector<Key<int>>::iterator begin() { return data.begin(); }
		std::vector<Key<int>>::const_iterator begin() const { return data.begin(); }
		std::vector<Key<int>>::iterator end() { return data.end(); }
		std::vector<Key<int>>::const_iterator end() const { return data.end(); }

		std::unique_ptr<char[]> getRawData();
	private:
		bool leaf;
		int dimensions;
		std::vector<Key<int>> data;
	};
}



