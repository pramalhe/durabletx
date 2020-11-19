// Copyright (c) 2011 The LevelDB Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file. See the AUTHORS file for names of contributors.
//
// An iterator yields a sequence of key/value pairs from a source.
// The following class defines the interface.  Multiple implementations
// are provided by this library.  In particular, iterators are provided
// to access the contents of a Table or a DB.
//
// Multiple threads can invoke const methods on an Iterator without
// external synchronization, but if any of the threads may call a
// non-const method, all threads accessing the same Iterator must use
// external synchronization.

#ifndef _TM_BTREE_MAP_ITERATOR_H_
#define _TM_BTREE_MAP_ITERATOR_H_

#include "slice.h"
#include "status.h"
#include "iterator.h"
#include "TMBTreeMap.hpp"

namespace ptmdb {


class TMBTreeMapIterator : public Iterator {
public:
    uint8_t           pad_a[64];
	TMBTreeMap*       db_btree = nullptr;
	// The state of the iterator is kept in the 'bucket' and 'node'
	TMBTreeMap::Node* node = nullptr;
	long              it = -1;
	uint8_t           pad_b[64];

	TMBTreeMapIterator(TMBTreeMap* db_btree) : db_btree{db_btree} {
	}

	~TMBTreeMapIterator() {
	}

	// An iterator is either positioned at a key/value pair, or
	// not valid.  This method returns true iff the iterator is valid.
	bool Valid() const{
		if(it==-1) return false;
		return true;
	}

	// Position at the first key in the source.  The iterator is Valid()
	// after this call iff the source is not empty.
	void SeekToFirst(){
		TMBTreeMap::Node* n = db_btree->root.pload();
		if(n->isLeaf()){
			it = -1;
			return;
		}

		while(!n->isLeaf()){
			n = n->children[0].pload();
		}
		node = n;
		it = 0;

	}

	// Position at the last key in the source.  The iterator is
	// Valid() after this call iff the source is not empty.
	void SeekToLast(){
		TMBTreeMap::Node* n = db_btree->root.pload();
		if(n->isLeaf()){
			it = -1;
			return;
		}

		while(!n->isLeaf()){
			n = n->children[n->length.pload()].pload();
		}
		node = n;
		it = n->length.pload()-1;

	}

	// Position at the first key in the source that is at or past target.
	// The iterator is Valid() after this call iff the source contains
	// an entry that comes at or past target.
	void Seek(const Slice& target){
		node = db_btree->root.pload();

		TMBTreeMap::Node* snode = node;
		while(!snode->isLeaf()){
			TMBTreeMap::SearchResult sr = snode->search(target);
			if (sr.first){
				node = snode;
				it = sr.second;
				return;
			}
			else{// Internal node
				snode = snode->children[sr.second];
			}

		}

		TMBTreeMap::SearchResult sr = snode->search(target);
		if(sr.first || sr.second<snode->length){
			node = snode;
			it = sr.second;
			return;
		}
		node = snode;
		while(true){
			TMBTreeMap::Node* p = node->parent.pload();
			if(p==nullptr) {
				it=-1;
				return;
			}
			int32_t index = node->childIndex.pload();
			node = p;
			if(index != p->length){
				it = index;
				return;
			}
		}

	}

	// Moves to the next entry in the source.  After this call, Valid() is
	// true iff the iterator was not positioned at the last entry in the source.
	// REQUIRES: Valid()
	void Next(){

		if(it == -1) return;

		if(node->isLeaf()){
			it++;
			if(it<node->length) {
				return;
			}
			while(true){
				TMBTreeMap::Node* p = node->parent.pload();
				if(p==nullptr) {
					it=-1;
					return;
				}
				int32_t index = node->childIndex.pload();
				node = p;
				if(index < p->length){
					it = index;
					return;
				}
			}
		}

		node = node->children[it+1];
		while(!node->isLeaf()){
			node = node->children[0];
		}
		it = 0;
	}

	// Moves to the previous entry in the source.  After this call, Valid() is
	// true iff the iterator was not positioned at the first entry in source.
	// REQUIRES: Valid()
	void Prev(){
		if(it == -1) return;

		if(node->isLeaf()){
			it--;
			if(it>=0) {
				return;
			}
			while(true){
				TMBTreeMap::Node* p = node->parent.pload();
				if(p==nullptr) {
					it=-1;
					return;
				}
				int32_t index = node->childIndex.pload();
				node = p;
				if(index-1 >= 0){
					it = index-1;
					return;
				}

			}
		}

		node = node->children[it];
		while(!node->isLeaf()){
			node = node->children[node->length];
		}
		it = node->length-1;
	}

	// Return the key for the current entry.  The underlying storage for
	// the returned slice is valid only until the next modification of
	// the iterator.
	// REQUIRES: Valid()
	Slice key() const{
		if(it==-1) return Slice();
		return node->keys[it].data();


	}

	// Return the value for the current entry.  The underlying storage for
	// the returned slice is valid only until the next modification of
	// the iterator.
	// REQUIRES: Valid()
	Slice value() const{
		if(it==-1) return Slice();
		return node->vals[it].data();
	}

	// If an error has occurred, return it.  Else return an ok status.
	Status status() const{
		return Status{};
	}

	// Clients are allowed to register function/arg1/arg2 triples that
	// will be invoked when this iterator is destroyed.
	//
	// Note that unlike all of the preceding methods, this method is
	// not abstract and therefore clients should not override it.
	//typedef void (*CleanupFunction)(void* arg1, void* arg2);
	void RegisterCleanup(CleanupFunction function, void* arg1, void* arg2){

	}

	// No copying allowed
	TMBTreeMapIterator(const TMBTreeMapIterator&);
	void operator=(const TMBTreeMapIterator&);
};

}  // namespace ptmdb

#endif  // _TM_BTREE_MAP_ITERATOR_H_
