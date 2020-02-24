#pragma once

#include <memory>
#include <vector>
#include "Data.h"

template<class T>
struct Key {
	T key;
	int leftBlockPtr;
	bool operator<(int target) {
		return this->key < target;
	}
};

template<class T>
inline bool operator<(const Key<T>& a, const Key<T>& b) {
	return a.key < b.key;
}

template<class T>
class BTreeNode
{
public:
	BTreeNode();
	BTreeNode(int size);
	BTreeNode(Key<T> values[]);
	BTreeNode(const BTreeNode<T>& source);

	Key<T>& search(T val);
	void insert(T val);
	void loadFromArray(Key<T> values[]);
	int count() { return (int) this->data.size(); }
	std::unique_ptr<BTreeNode<T>> split();
	std::unique_ptr<Key<T>[]> getRawData();

	static constexpr int BLOCK_VALUES_COUNT = (BLOCK_SIZE / sizeof(Key<T>)) - 1;

	Key<T>& operator[](int i) {
		return this->data[i];
	}

private:
	std::vector<Key<T>> data;
};