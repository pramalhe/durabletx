/*
 * B-tree Map (C++)
 *
 * Copyright (c) 2018 Project Nayuki. (MIT License)
 * https://www.nayuki.io/page/btree-set
 *
 * Modified by Andreia Correia
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy of
 * this software and associated documentation files (the "Software"), to deal in
 * the Software without restriction, including without limitation the rights to
 * use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of
 * the Software, and to permit persons to whom the Software is furnished to do so,
 * subject to the following conditions:
 * - The above copyright notice and this permission notice shall be included in
 *   all copies or substantial portions of the Software.
 * - The Software is provided "as is", without warranty of any kind, express or
 *   implied, including but not limited to the warranties of merchantability,
 *   fitness for a particular purpose and noninfringement. In no event shall the
 *   authors or copyright holders be liable for any claim, damages or other
 *   liability, whether in an action of contract, tort or otherwise, arising from,
 *   out of or in connection with the Software or the use or other dealings in the
 *   Software.
 */

#ifndef _TM_BTREE_MAP_PSLICE_H_
#define _TM_BTREE_MAP_PSLICE_H_

#pragma once

#include <algorithm>
#include <cassert>
#include <climits>
#include <cstddef>
#include <cstdint>
#include <memory>
#include <stdexcept>
#include <utility>
#include "ptmdb.h"
#include "slice.h"


namespace ptmdb {


/* If V is larger than size_t then better make it a pointer to the object, otherwise it's going to be slow */
class TMBTreeMap {

private: static const int MAXKEYS { 16 };    // Must be (at least) degree*2 - 1

public: class Node;  // Forward declaration

/*---- Fields ----*/

public: PTM_TYPE<Node*>   root {nullptr};
public: PTM_TYPE<int32_t> minKeys;  // At least 1, equal to degree-1
public: PTM_TYPE<int32_t> maxKeys;  // At least 3, odd number, equal to minKeys*2+1



/*---- Constructors ----*/

// The degree is the minimum number of children each non-root internal node must have.
public: explicit TMBTreeMap(std::int32_t degree=8) :
									minKeys(degree - 1),
									maxKeys(degree <= UINT32_MAX / 2 ? degree * 2 - 1 : 0) {  // Avoid overflow
	if (degree < 2)
		throw std::domain_error("Degree must be at least 2");
	if (degree > UINT32_MAX / 2)  // In other words, need maxChildren <= UINT32_MAX
		throw std::domain_error("Degree too large");
	clear();
}

~TMBTreeMap() {
	if (root == nullptr) return;
	Node* first = seekFirst(root);
	deleteAll(first);
}


/*---- Methods ----*/

public: void deleteAll(Node* node) {
	int it=-1;
	TMBTreeMap::Node* p =nullptr;
	while(true){
		p = node->parent.pload();
		int32_t index = node->childIndex.pload();
		PTM_DELETE(node);
		if(p==nullptr) return;

		it = index+1;
		int32_t len = p->length;
		while(it <= len){
			node = p->children[it];
			if(!node->isLeaf()) {
				node = node->children[0];
				while(!node->isLeaf()){
					node = node->children[0];
				}
				it = 0;
				break;
			}
			PTM_DELETE(node);
			it++;
		}
		if(it <= len) {
			continue;
		}
		node = p;
	}
}

public: Node* seekFirst(Node* r) {
	if(r->isLeaf()){
		return r;
	}

	while(!r->isLeaf()){
		r = r->children[0].pload();
	}
	return r;
}

// Starting from root, add all keys in a set and all values in another set and make sure they are unique
// This is used for debugging only.
/*
public: void assertIfNotASet(Node* node, std::set<PBTSlice*>& keyset, std::set<PBTSlice*>& valset) {
    if (!node->isLeaf()) {
        for(int i = 0; i < node->length.pload()+1; i++) {
            assertIfNotASet(node->children[i], keyset, valset);
        }
    }
    for (int i = 0; i < node->length.pload(); i++) {
        auto resultKey = keyset.insert(node->keys[i].pload());
        assert (resultKey.second);
        auto resultVal = valset.insert(node->vals[i].pload());
        assert (resultVal.second);
    }
}
*/

public: void clear() {
	if (root != nullptr) {
		deleteAll(root);
	}
	root = PTM_NEW<Node>(maxKeys, true, nullptr);
}


using SearchResult = std::pair<bool,std::int32_t>;
using KeyVal = std::pair<PBTSlice*,PBTSlice*>;


/*---- Helper class: B-tree node ----*/

public: class Node final {

	/*-- Fields --*/

public: PTM_TYPE<int32_t>   childIndex;
public: PTM_TYPE<int32_t>   length {0};
// Size is in the range [0, maxKeys] for root node, [minKeys, maxKeys] for all other nodes.
public: PBTSlice  keys[MAXKEYS];
public: PBTSlice  vals[MAXKEYS];
// If leaf then size is 0, otherwise if internal node then size always equals keys.size()+1.
public: PTM_TYPE<Node*>     children[MAXKEYS+1];
public: PTM_TYPE<Node*>     parent;
/*-- Constructor --*/

// Note: Once created, a node's structure never changes between a leaf and internal node.
public: Node(std::uint32_t maxKeys, bool leaf, Node* parent) {
	assert(maxKeys >= 3 && maxKeys % 2 == 1);
	assert(maxKeys <= MAXKEYS);
	//printf("Creating Node = %p\n", this);
	for (int i = 0; i < maxKeys+1; i++) children[i] = nullptr;
	this->parent.pstore(parent);
}

public: ~Node() {
    //printf("Destroying Node = %p with length=%d\n", this, length.pload());
}

/*-- Methods for getting info --*/

public: void print() {
    printf("keys=[");
    for (int i = 0; i < length; i++) {
        printf("%p, ", &keys[i]);
    }
    printf("]\n");
    printf("vals=[");
    for (int i = 0; i < length; i++) {
        printf("%p, ", &vals[i]);
    }
    printf("]\n");
}

public: int getSize(){
	int size = 0;
	for(int i=0; i<length.pload();i++){
		size += children[i].pload()->getSize();
	}
	return size;
}

public: bool getLength() const {
	return length;
}

public: bool isLeaf() const {
	return (children[0]==nullptr);
}

// Searches this node's keys vector and returns (true, i) if obj equals keys[i],
// otherwise returns (false, i) if children[i] should be explored. For simplicity,
// the implementation uses linear search. It's possible to replace it with binary search for speed.
public: SearchResult search(const Slice &val) const {
	if (length == 0) return SearchResult(false, 0);
	std::int32_t max = length;
	std::int32_t min = 0;
	while (true) {
		std::int32_t i = min + (max-min)/2;
		PBTSlice* elem = (PBTSlice*)&keys[i];
		int cmp = elem->compare(val);
		if (cmp == 0) {
			assert(i < length);
			return SearchResult(true, i);  // Key found
		} else if (cmp < 0){
			if (i == length-1 || max-min<=1) return SearchResult(false, i+1);
			min = i;
		}else{  // val < elem
			if (i == 0 || max-min<=1) return SearchResult(false, i);
			max = i;
		}
	}
}

/*-- Methods for insertion --*/

// For the child node at the given index, this moves the right half of keys and children to a new node,
// and adds the middle key and new child to this node. The left half of child's data is not moved.
public: void splitChild(std::size_t minKeys, std::int32_t maxKeys, std::size_t index) {
	assert(!this->isLeaf() && index <= this->length && this->length < maxKeys);
	Node* left = this->children[index];
	Node* right = PTM_NEW<Node>(maxKeys,left->isLeaf(),this);

	// Handle keys
	int j=0;
	for(int i=minKeys + 1;i<left->length;i++){
		right->keys[j] = left->keys[i];
		right->vals[j] = left->vals[i];
		j++;
	}

	//add right node to this
	for(int i=length+1; i>=index+2;i--){
		this->children[i] = this->children[i-1];
		this->children[i].pload()->childIndex.pstore(i);
	}
	this->children[index+1] = right;
	right->parent = this;
	right->childIndex.pstore(index+1);
	for(int i=length; i>=index+1;i--){
		this->keys[i] = this->keys[i-1];
		this->vals[i] = this->vals[i-1];
	}
	this->keys[index] = left->keys[minKeys];
	this->vals[index] = left->vals[minKeys];
	this->length = this->length+1;

	if(!left->isLeaf()){
		j=0;
		for(int i= minKeys + 1;i<left->length+1;i++){
			right->children[j] = left->children[i];
			right->children[j].pload()->parent = right;
			right->children[j].pload()->childIndex = j;
			j++;
		}
	}

	right->length = left->length-minKeys-1;
	left->length = minKeys;
}


/*-- Methods for removal --*/

// Performs modifications to ensure that this node's child at the given index has at least
// minKeys+1 keys in preparation for a single removal. The child may gain a key and subchild
// from its sibling, or it may be merged with a sibling, or nothing needs to be done.
// A reference to the appropriate child is returned, which is helpful if the old child no longer exists.
public: Node* ensureChildRemove(std::int32_t minKeys, std::uint32_t index) {
	// Preliminaries
	assert(!this->isLeaf() && index < this->length+1);
	Node* child = this->children[index];
	if (child->length > minKeys)  // Already satisfies the condition
		return child;
	assert(child->length == minKeys);

	// Get siblings
	Node* left = index >= 1 ? this->children[index - 1].pload() : nullptr;
	Node* right = index < this->length ? this->children[index + 1].pload() : nullptr;
	bool internal = !child->isLeaf();
	assert(left != nullptr || right != nullptr);  // At least one sibling exists because degree >= 2
	assert(left  == nullptr || left ->isLeaf() != internal);  // Sibling must be same type (internal/leaf) as child
	assert(right == nullptr || right->isLeaf() != internal);  // Sibling must be same type (internal/leaf) as child

	if (left != nullptr && left->length > minKeys) {  // Steal rightmost item from left sibling
		if (internal) {
			for(int i=child->length+1;i>=1;i--){
				child->children[i] = child->children[i-1];
				child->children[i].pload()->childIndex.pstore(i);
			}
			child->children[0] = left->children[left->length];
			child->children[0].pload()->parent = child;
			child->children[0].pload()->childIndex = 0;
		}
		for(int i=child->length;i>=1;i--){
			child->keys[i] = child->keys[i-1];
			child->vals[i] = child->vals[i-1];
		}
		child->keys[0] = this->keys[index - 1];
		child->vals[0] = this->vals[index - 1];
		this->keys[index-1] = left->keys[left->length-1];
		this->vals[index-1] = left->vals[left->length-1];
		left->length = left->length-1;
		child->length = child->length+1;
		return child;
	} else if (right != nullptr && right->length > minKeys) {  // Steal leftmost item from right sibling
		if (internal) {
			child->children[child->length+1] = right->children[0];
			child->children[child->length+1].pload()->parent = child;
			child->children[child->length+1].pload()->childIndex.pstore(child->length+1);
			for(int i=0;i<right->length;i++){
				right->children[i] = right->children[i+1];
				right->children[i].pload()->childIndex.pstore(i);
			}
		}
		child->keys[child->length] = this->keys[index];
		child->vals[child->length] = this->vals[index];
		KeyVal ret = right->removeKey(0);
		//PTM_DELETE(this->keys[index].pload());
		//PTM_DELETE(this->vals[index].pload());
		this->keys[index] = *ret.first;
		this->vals[index] = *ret.second;
		child->length = child->length+1;
		return child;
	} else if (left != nullptr) {  // Merge child into left sibling
		this->mergeChildren(minKeys, index - 1);
		return left;  // This is the only case where the return value is different
	} else if (right != nullptr) {  // Merge right sibling into child
		this->mergeChildren(minKeys, index);
		return child;
	} else
		throw std::logic_error("Impossible condition");
}


// Merges the child node at index+1 into the child node at index,
// assuming the current node is not empty and both children have minKeys.
public: void mergeChildren(std::int32_t minKeys, std::uint32_t index) {
	assert(!this->isLeaf() && index < this->length);
	Node* left  = this->children[index];
	Node* right = this->children[index+1];
	assert(left->length == minKeys && right->length == minKeys);
	//std::cout<<"Passou\n";
	if (!left->isLeaf()){
		for(int i=0;i<right->length+1;i++){
			left->children[left->length+1+i] = right->children[i];
			left->children[left->length+1+i].pload()->parent = left;
			left->children[left->length+1+i].pload()->childIndex.pstore(left->length+1+i);
		}
	}
	left->keys[left->length] = this->keys[index];
	left->vals[left->length] = this->vals[index];
	for(int i=0;i<right->length;i++){
		left->keys[left->length+1+i] = right->keys[i];
		left->vals[left->length+1+i] = right->vals[i];
	}
	left->length = left->length + right->length+1;
	// remove key(index)
	for(int i=index;i<length-1;i++){
		this->keys[i] = this->keys[i+1];
		this->vals[i] = this->vals[i+1];
	}
	// remove children (index+1)
	children[index+1].pload()->length = 0; // TODO: do we really need this or will cause leaks?
	PTM_DELETE(children[index+1].pload());
	for(int i=index+1;i<length;i++){
		this->children[i] = this->children[i+1];
		this->children[i].pload()->childIndex.pstore(i);
	}
	this->children[length] = nullptr;
	length = length-1;
}


// Removes and returns the minimum key among the whole subtree rooted at this node.
// Requires this node to be preprocessed to have at least minKeys+1 keys.
public: KeyVal removeMin(std::int32_t minKeys) {
	for (Node* node = this; ; ) {
		assert(node->length > minKeys);
		if (node->isLeaf()){
			KeyVal ret = KeyVal(&node->keys[0],&node->vals[0]);
			//PTM_DELETE(node->keys[0]->pload());
			//PTM_DELETE(node->vals[0]->pload());
			for(int i=0;i<node->length-1;i++){
				node->keys[i] = node->keys[i+1];
				node->vals[i] = node->vals[i+1];
			}
			node->length = node->length-1;
			return ret;
		}else{
			node = node->ensureChildRemove(minKeys, 0);
		}

	}
}


// Removes and returns the maximum key among the whole subtree rooted at this node.
// Requires this node to be preprocessed to have at least minKeys+1 keys.
public: KeyVal removeMax(std::int32_t minKeys) {
	for (Node *node = this; ; ) {
		assert(node->length > minKeys);
		if (node->isLeaf()){
			node->length = node->length-1;
            //PTM_DELETE(node->keys[node->length]->pload());
            //PTM_DELETE(node->vals[node->length]->pload());
			return KeyVal(&node->keys[node->length], &node->vals[node->length]);
		}else{
			node = node->ensureChildRemove(minKeys, node->length);
		}
	}
}


// Removes and returns this node's key at the given index.
public: KeyVal removeKey(std::uint32_t index) {
	KeyVal ret = KeyVal(&keys[index], &vals[index]);
	for(int i=index;i < this->length-1;i++){
		this->keys[i] = this->keys[i+1];
		this->vals[i] = this->vals[i+1];
	}
	length = length-1;
	return ret;
}
};

static std::string className() { return PTM_NAME()+ "-BTreeMap" ; }


/*
 * Adds a node with a key if the key is not present, otherwise replaces the value.
 * Returns true if there was no mapping for the key, false if there was already a value and it was replaced.
 */
bool innerPut(const Slice& key, const Slice& val) {
	// Special preprocessing to split root node
	if (root.pload()->length.pload() == maxKeys) {
		Node* child = root;
		root = PTM_NEW<Node>(maxKeys, false,nullptr);  // Increment tree height
		child->parent = root;
		root.pload()->children[0] = child;
		child->childIndex.pstore(0);
		root.pload()->splitChild(minKeys, maxKeys, 0);
	}

	// Walk down the tree
	Node* node = root;
	//node->print();
	while (true) {
		// Search for index in current node
		assert(node->length < maxKeys);
		assert(node == root || node->length >= minKeys);

		SearchResult sr = node->search(key);
		if (sr.first) {
		    // Replace val
			node->vals[sr.second] = val;
			//node->print();
			return false;  // Key already exists in tree
		}
		std::int32_t index = sr.second;
		if (node->isLeaf()) {  // Simple insertion into leaf
			for(int i=node->length;i>=index+1;i--){
				node->keys[i] = node->keys[i-1];
				node->vals[i] = node->vals[i-1];
			}
			node->keys[index] = key;
			node->vals[index] = val;
			node->length = node->length+1;
			//node->print();
			return true;  // Successfully inserted

		} else {  // Handle internal node
			Node* child = node->children[index];
			if (child->length == maxKeys) {  // Split child node
				node->splitChild(minKeys, maxKeys, index);
				PBTSlice* middleKey = &node->keys[index];
				int cmp = middleKey->compare(key);
				if (cmp == 0) {
					node->vals[index] = val;
					//node->print();
					return false;  // Key already exists in tree
				} else if (cmp < 0) {
					child = node->children[index + 1];
				}
			}
			node = child;
		}
	}
}


/*
 * Removes a key and its mapping.
 * Returns returns true if a matching key was found
 */
bool innerRemove(const Slice& key) {
	// Walk down the tree
	bool found;
	std::int32_t index;
	{
		SearchResult sr = root.pload()->search(key);
		found = sr.first;
		index = sr.second;
	}
	Node* node = root;
	while (true) {
		assert(node->length <= maxKeys);
		assert(node == root || node->length > minKeys);
		if (node->isLeaf()) {
			if (found) {  // Simple removal from leaf
			    // TODO: implement a "reset()" ?
		        //PTM_DELETE(node->keys[index].pload());
		        //PTM_DELETE(node->vals[index].pload());
				node->removeKey(index);
				return 1;
			} else
				return 0;

		} else {  // Internal node
			if (found) {  // Key is stored at current node
				Node* left  = node->children[index + 0];
				Node* right = node->children[index + 1];
				assert(left != nullptr && right != nullptr);
				if (left->length > minKeys) {  // Replace key with predecessor
					KeyVal ret = left->removeMax(minKeys);
					node->keys[index] = *ret.first;
					node->vals[index] = *ret.second;
					return true;
				} else if (right->length > minKeys) {  // Replace key with successor
					KeyVal ret = right->removeMin(minKeys);
					node->keys[index] = *ret.first;
					node->vals[index] = *ret.second;
					return true;
				} else {  // Merge key and right node into left node, then recurse
					node->mergeChildren(minKeys, index);
					if (node == root && root.pload()->length==0) {
						assert(root.pload()->length+1 == 1);
						Node* next = root.pload()->children[0];
						PTM_DELETE(root.pload());
						root = next; //TODO:delete root
						//root->children[0] = nullptr;
					}
					node = left;
					index = minKeys;  // Index known due to merging; no need to search
				}
			} else {  // Key might be found in some child
				Node* child = node->ensureChildRemove(minKeys, index);
				if (node == root && root.pload()->length==0) {
					assert(root.pload()->length +1 == 1);
					Node* next = root.pload()->children[0];
					PTM_DELETE(root.pload());
					root = next; //TODO:delete root
					//root->children[0] = nullptr;
				}
				node = child;
				SearchResult sr = node->search(key);
				found = sr.first;
				index = sr.second;
			}
		}
	}
}


/*
 * Returns true if key is present. Saves a copy of 'value' in 'oldValue' if 'saveOldValue' is set.
 */
bool innerGet(const Slice& key, std::string* oldValue) {
	// Walk down the tree
	Node* node = root.pload();
	while (true) {
		SearchResult sr = node->search(key);
		if (sr.first) {
			// Found a matching key: make a copy of the corresponding value
			oldValue->assign(node->vals[sr.second].data(), node->vals[sr.second].size());
			return true;
		} else if (node->isLeaf()) {
			return false;
		} else { // Internal node
			node = node->children[sr.second];
		}
	}
}

};
}
#endif   // _TM_BTREE_MAP_PSLICE_H_
