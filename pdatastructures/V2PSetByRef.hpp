/*
 * Copyright 2018
 *   Andreia Correia <andreia.veiga@unine.ch>
 *   Pedro Ramalhete <pramalhe@gmail.com>
 *   Pascal Felber <pascal.felber@unine.ch>
 *
 * This work is published under the MIT license. See LICENSE.txt
 */
#ifndef _VOLATILE_TO_PERSISTENT_SET_BY_REF_H_
#define _VOLATILE_TO_PERSISTENT_SET_BY_REF_H_



/**
 * <h1> This is a volatile wrapper to a persistent set</h1>
 *
 * We've created this for greater comodity to the user.
 * No need to place this inside a transaction.
 */
template<typename K, typename PTM, typename PSET>
class V2PSetByRef {
private:
    int    _objidx;

public:
    V2PSetByRef(int idx=0) {
        _objidx = idx;
        PTM::updateTx([&] () {
            PSET* pset = PTM::template tmNew<PSET>();
            PTM::put_object(idx, pset);
        });
    }

    ~V2PSetByRef() {
        int idx = _objidx;
        PTM::updateTx([&] () {
            PSET* pset = (PSET*)PTM::get_object(idx);
            PTM::tmDelete(pset);
            PTM::put_object(idx, nullptr);
        });
    }

    static std::string className() { return PSET::className(); }

    //
    // Set methods for running the usual tests and benchmarks
    //

    // Inserts a key only if it's not already present
    bool add(K key) {
        int idx = _objidx;
        bool ret = false;
        PTM::updateTx([&] () {
            PSET* pset = (PSET*)PTM::get_object(idx);
            ret = pset->add(key);
        });
        return ret;
    }

    // Returns true only if the key was present
    bool remove(K key) {
        int idx = _objidx;
        bool ret = false;
        PTM::updateTx([&] () {
            PSET* pset = (PSET*)PTM::get_object(idx);
            ret = pset->remove(key);
        });
        return ret;
    }

    bool contains(K key) {
        int idx = _objidx;
        bool ret = false;
        PTM::readTx([&] () {
            PSET* pset = (PSET*)PTM::get_object(idx);
            ret = pset->contains(key);
        });
        return ret;
    }

    // Used only for benchmarks
    bool addAll(K** keys, const int size) {
        for (int i = 0; i < size; i++) add(*keys[i]);
        return true;
    }
};

#endif /* _VOLATILE_TO_PERSISTENT_SET_BY_REF_H_ */
