// Copyright (c) 2011 The LevelDB Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file. See the AUTHORS file for names of contributors.
//
// Slice is a simple structure containing a pointer into some external
// storage and a size.  The user of a Slice must ensure that the slice
// is not used after the corresponding external storage has been
// deallocated.
//
// Multiple threads can invoke const methods on a Slice without
// external synchronization, but if any of the threads may call a
// non-const method, all threads accessing the same Slice must use
// external synchronization.

#ifndef _RDB_INCLUDE_SLICE_H_
#define _RDB_INCLUDE_SLICE_H_

#include <cassert>
#include <cstddef>
#include <cstring>
#include <string>
#include "ptmdb.h"

namespace ptmdb {

class Slice {
 public:
  // Create an empty slice.
  Slice() : data_(""), size_(0) { }

  ~Slice() { if (isreplica) delete[] data_; }

  // Create a slice that refers to d[0,n-1].
  Slice(const char* d, size_t n) : data_(d), size_(n) { }

  // Create a slice that refers to the contents of "s"
  Slice(const std::string& s) : data_(s.data()), size_(s.size()) { }

  // Create a slice that refers to s[0,strlen(s)-1]
  Slice(const char* s) : data_(s), size_(strlen(s)) { }

  // Copy constructor
  Slice(const Slice& other){
    size_ = other.size_;
    data_ = new char[size_+1];
    std::memcpy((void*)data_, (void*)other.data_, size_+1);
    isreplica = true; // Tell the destructor to cleanup data_
    //printf("Slice copy constructor size=%ld\n", size_);
  }

  // Return a pointer to the beginning of the referenced data
  const char* data() const { return data_; }

  // Return the length (in bytes) of the referenced data
  size_t size() const { return size_; }

  // Return true iff the length of the referenced data is zero
  bool empty() const { return size_ == 0; }

  // Return the ith byte in the referenced data.
  // REQUIRES: n < size()
  char operator[](size_t n) const {
    assert(n < size());
    return data_[n];
  }

  // Change this slice to refer to an empty array
  void clear() { data_ = ""; size_ = 0; }

  // Drop the first "n" bytes from this slice.
  void remove_prefix(size_t n) {
    assert(n <= size());
    data_ += n;
    size_ -= n;
  }

  // Return a string that contains the copy of the referenced data.
  std::string ToString() const { return std::string(data_, size_); }

  // Three-way comparison.  Returns value:
  //   <  0 iff "*this" <  "b",
  //   == 0 iff "*this" == "b",
  //   >  0 iff "*this" >  "b"
  int compare(const Slice& b) const;

  // Return true iff "x" is a prefix of "*this"
  bool starts_with(const Slice& x) const {
    return ((size_ >= x.size_) &&
            (std::memcmp(data_, x.data_, x.size_) == 0));
  }

  uint64_t toHash() const {
      uint64_t h = 5381;
      for (size_t i = 0; i < size_; i++) {
          h = ((h << 5) + h) + data_[i]; /* hash * 33 + data_[i] */
      }
      return h;
  }

private:
  const char* data_ {nullptr};
  size_t size_ {0};
  bool isreplica {false};
};

inline bool operator==(const Slice& x, const Slice& y) {
  return ((x.size() == y.size()) &&
          (std::memcmp(x.data(), y.data(), x.size()) == 0));
}

inline bool operator!=(const Slice& x, const Slice& y) {
  return !(x == y);
}

inline int Slice::compare(const Slice& b) const {
  const size_t min_len = (size_ < b.size_) ? size_ : b.size_;
  int r = std::memcmp(data_, b.data_, min_len);
  if (r == 0) {
    if (size_ < b.size_) r = -1;
    else if (size_ > b.size_) r = +1;
  }
  return r;
};


// We need this for persistency because the contents have to be copied into persistent memory
class PSlice : public PTM_BASE {
public:
	PSlice() { }
	
    // Creates a copy of an existing Slice
    PSlice(const Slice& sl) {
        size_ = sl.size();
        data_ = (char*)PTM_MALLOC(size_+1);
        uint8_t* _addr = (uint8_t*)data_.pload();
        size_t _size = size_.pload();
#ifdef USE_CXREDO
        uint64_t offset = cxredo::tlocal.tl_cx_size;
        PTM_LOG(_addr,(uint8_t*)sl.data(),_size);
        std::memcpy(_addr+offset, sl.data(), _size);
        *reinterpret_cast<char*>(_addr+_size+offset) = 0;
        PTM_FLUSH((uint8_t*)_addr, _size+1);
#elif defined USE_CXREDOTIMED
        uint64_t offset = cxredotimed::tlocal.tl_cx_size;
        PTM_LOG(_addr,(uint8_t*)sl.data(),_size);
        std::memcpy(_addr+offset, sl.data(), _size);
        *reinterpret_cast<char*>(_addr+_size+offset) = 0;
        PTM_FLUSH((uint8_t*)_addr, _size+1);
#elif defined USE_REDOTIMEDHASH
        uint64_t offset = redotimedhash::tlocal.tl_cx_size;
        PTM_LOG(_addr,(uint8_t*)sl.data(),_size);
        std::memcpy(_addr+offset, sl.data(), _size);
        *reinterpret_cast<char*>(_addr+_size+offset) = 0;
        PTM_FLUSH((uint8_t*)_addr, _size+1);
#else
        PTM_MEMCPY(_addr, sl.data(), _size+1);
#endif
    }

    // Creates a copy of an existing PSlice (Copy Constructor). Not being used
    PSlice(const PSlice& psl) {
        size_ = psl.size();
        data_ = (char*)PTM_MALLOC(size_+1);
        uint8_t* _addr = (uint8_t*)data_.pload();
        size_t _size = size_.pload();
#ifdef USE_CXREDO
        uint64_t offset = cxredo::tlocal.tl_cx_size;
        PTM_LOG(_addr,(uint8_t*)psl.data(),_size);
        std::memcpy(_addr+offset, psl.data(), _size+1);
        PTM_FLUSH((uint8_t*)_addr, _size+1);
#elif defined USE_CXREDOTIMED
        uint64_t offset = cxredotimed::tlocal.tl_cx_size;
        PTM_LOG(_addr,(uint8_t*)psl.data(),_size);
        std::memcpy(_addr+offset, psl.data(), _size+1);
        PTM_FLUSH((uint8_t*)_addr, _size+1);
#elif defined USE_REDOTIMEDHASH
        uint64_t offset = redotimedhash::tlocal.tl_cx_size;
        PTM_LOG(_addr,(uint8_t*)psl.data(),_size);
        std::memcpy(_addr+offset, psl.data(), _size+1);
        PTM_FLUSH((uint8_t*)_addr, _size+1);
#else
        PTM_MEMCPY(_addr, psl.data(), _size+1);
#endif
    }

    ~PSlice() {
        PTM_FREE(data_.pload());
    }

    // Return a pointer to the beginning of the referenced data
    const char* data() const {
#ifdef USE_CXREDO
        uint64_t offset = cxredo::tlocal.tl_cx_size;
#elif defined USE_CXREDOTIMED
        uint64_t offset = cxredotimed::tlocal.tl_cx_size;
#else
        uint64_t offset = 0;
#endif
    	return data_.pload()+offset;
    }

    // Return the length (in bytes) of the referenced data
    size_t size() const { return size_.pload(); }

    int compare(const Slice& other) {
        return PTM_STRCMP(data(), other.data(), size());
    }

    bool operator == (const Slice& other) const {
        return (size() == other.size() && PTM_MEMCMP(data(), other.data(), size()) == 0);
    }

    bool operator == (const PSlice& other) const {
        return (size() == other.size() && PTM_MEMCMP(data(), other.data(), size()) == 0);
    }

    // Comparison operator
    bool operator < (const Slice& other) {
        return (PTM_MEMCMP(data(), other.data(), size()) < 0);
    }

    // Assignment operator
    PSlice& operator=(const Slice& psl) {
        printf("PSlice assignment\n");
        // Delete the old data (if there was something)
        PTM_FREE(data_.pload());
        size_ = psl.size();
        data_ = (char*)PTM_MALLOC(size_+1);
        uint8_t* _addr = (uint8_t*)data_.pload();
        size_t _size = size_.pload();
#ifdef USE_CXREDO
        uint64_t offset = cxredo::tlocal.tl_cx_size;
        PTM_LOG(_addr,(uint8_t*)psl.data(),_size);
        std::memcpy(_addr+offset, psl.data(), _size+1);
        PTM_FLUSH((uint8_t*)_addr, _size+1);
#elif defined USE_CXREDOTIMED
        uint64_t offset = cxredotimed::tlocal.tl_cx_size;
        PTM_LOG(_addr,(uint8_t*)psl.data(),_size);
        std::memcpy(_addr+offset, psl.data(), _size+1);
        PTM_FLUSH((uint8_t*)_addr, _size+1);
#elif defined USE_REDOTIMEDHASH
        uint64_t offset = redotimedhash::tlocal.tl_cx_size;
        PTM_LOG(_addr,(uint8_t*)psl.data(),_size);
        std::memcpy(_addr+offset, psl.data(), _size+1);
        PTM_FLUSH((uint8_t*)_addr, _size+1);
#else
        PTM_MEMCPY(_addr, psl.data(), _size+1);
#endif
        return *this;
    }

    uint64_t toHash() const {
#ifdef USE_CXREDO
        uint64_t offset = cxredo::tlocal.tl_cx_size;
#elif defined USE_CXREDOTIMED
        uint64_t offset = cxredotimed::tlocal.tl_cx_size;
#elif defined USE_REDOTIMEDHASH
        uint64_t offset = redotimedhash::tlocal.tl_cx_size;
#else
        uint64_t offset = 0;
#endif
        uint64_t h = 5381;
        PTM_TYPE<char>* data = (PTM_TYPE<char>*)(data_.pload()+offset);
        size_t size = size_.pload();
        for (size_t i = 0; i < size; i++) {
            h = ((h << 5) + h) + data[i]; /* hash * 33 + data_[i] */ // pload() on every data[i]
        }
        return h;
    }

private:
    PTM_TYPE<char*> data_ {nullptr};
    PTM_TYPE<size_t> size_ {0};
};



// This is a variant of PSlice meant to be used ONLY in the BTreeMap
// The assignment operator does NOT make a copy
class PBTSlice {
public:
    PBTSlice() { }

    ~PBTSlice() {
        //printf("PBTSlice destructor of size=%d\n", size_.pload());
        if (size_ != 0) {
            PTM_FREE(data_.pload());
            size_ = 0;
        }
    }

    // Assignment operator
    void operator=(const Slice& sl) {
        //printf("PBTSlice assignment of const Slice\n");
        int32_t size = sl.size();
        char* addr = data_;
        // If different size, delete the old string and make a new one, otherwise re-use
        if (size != size_) {
            //printf("PBTSlice %d != %d => deleting old data\n", size, size_.pload());
            // Delete the old data (if there was something)
            if (size_ != 0) PTM_FREE(data_.pload());
            addr = (char*)PTM_MALLOC(size+1);
            data_ = addr;
            size_ = size;
        }
        PTM_MEMCPY(addr, sl.data(), size+1);
    }

    // Assignment operator from a PBTSlice does NOT delete the old data
    void operator=(PBTSlice& psl) {
        //printf("PBTSlice assignment PBTSlice\n");
        size_ = psl.size_;
        data_ = psl.data_;
        psl.size_ = 0;
    }

    // Return a pointer to the beginning of the referenced data
    inline const char* data() const {
        return data_.pload();
    }

    // Return the length (in bytes) of the referenced data
    inline size_t size() const {
        return size_.pload();
    }

    inline int compare(const Slice& other) {
        return PTM_STRCMP(data_.pload(), other.data(), size_.pload());
    }

private:
    PTM_TYPE<char*>   data_ {nullptr};
    PTM_TYPE<int32_t> size_ {0};
};

}  // namespace ptmdb


namespace std {
    template <>
    struct hash<ptmdb::Slice> {
        std::size_t operator()(const ptmdb::Slice& k) const {
            using std::size_t;
            using std::hash;
            return (hash<uint64_t>()(k.toHash()));
        }
    };

    template <>
    struct hash<ptmdb::PSlice> {
        std::size_t operator()(const ptmdb::PSlice& k) const {
            using std::size_t;
            using std::hash;
            return (hash<uint64_t>()(k.toHash()));
        }
    };
}

#endif
