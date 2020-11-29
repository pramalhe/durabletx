#include <vector>
#include <string>
#include <cstring>
#include <thread>
#include <atomic>
#include <random>
#include <algorithm>
#include <iostream>
#include <chrono>
#include <fstream>
#include <cassert>

#ifdef USE_PMEMKV
#include <libpmemkv.hpp>
using namespace pmem::kv;
#elif defined USE_ROCKSDB
//#include "rocksdb/rocksdb_namespace.h"
#define ROCKSDB_NAMESPACE rocksdb
#include "rocksdb/db.h"
#include "rocksdb/slice.h"
#include "rocksdb/options.h"
#else
#include "db.h"
#include "db_impl.h"
#include "options.h"
#include "status.h"
#include "ptmdb.h"
#endif

#define MAX_KEY_SIZE  256

using namespace std;
using namespace chrono;
#ifdef USE_PMEMKV
using namespace pmem::kv;
#elif defined USE_ROCKSDB
using namespace ROCKSDB_NAMESPACE;
#else // PTMDB
using namespace ptmdb;
#endif

enum class Operation {
  INSERT,
  READ,
  SCAN,
  UPDATE,
  READMODIFYWRITE
};

atomic<bool> startFlag = { false };
atomic<int>  startAtZero = { 0 };


class YCSBPTM {
public:
    int valueSize_ {0};
    char **keys_ {nullptr};
    char **values_ {nullptr};   // This isn't really useful seen as the value is always the same
    vector<string> **traces_ {nullptr};
    char *valueBuffer_ {nullptr};
#ifdef USE_PMEMKV
    db* db_ {nullptr};
#else
    DB* db_ {nullptr};
#endif
    const char* one_ring = "Three Rings for the Elven-kings under the sky, Seven for the Dwarf-lords in their halls of stone, Nine for Mortal Men doomed to die, One for the Dark Lord on his dark throne. In the Land of Mordor where the Shadows lie. One Ring to rule them all, One Ring to find them, One Ring to bring them all, and in the darkness bind them, In the Land of Mordor where the Shadows lie.";
    uint64_t operations_ {0};

    YCSBPTM(int nThreads, string& workloadPrefix, int valueSize=100) {
        valueSize_ = valueSize;
        keys_ = (char **)malloc(sizeof(char *) * nThreads);
        values_ = (char **)malloc(sizeof(char *) * nThreads);
        valueBuffer_ = (char *)malloc(valueSize_+1);
        memcpy(valueBuffer_, one_ring, valueSize_);
        valueBuffer_[valueSize_] = 0;

        // Open/create DB file in PM
#ifdef USE_PMEMKV
        config cfg;
        status s = cfg.put_path("/mnt/pmem0/pmemkv_pool");
        assert(s == status::OK);
        db_ = new db();
        assert(db_ != nullptr);
        s = db_->open("cmap", std::move(cfg));
        assert(s == status::OK);
#elif defined USE_ROCKSDB
        Options options{};
        options.compression = ROCKSDB_NAMESPACE::kNoCompression;
        options.create_if_missing = true;
        Status s = DB::Open(options, "/mnt/pmem0/rocksdb", &db_);  // The path is used only by rocksdb
        assert(s.ok());
#else
        Options options{};
        Status s = DB::Open(options, "/tmp/notneeded", &db_);
        assert(s.ok());
#endif

        // Populate database with the load data
        string workloadPath = workloadPrefix + "-load-" + to_string(nThreads) + ".";
        char *value = (char *)malloc(valueSize+1);
        std::memcpy(value, valueBuffer_, valueSize);
        value[valueSize] = 0;
        for (unsigned int thread = 0; thread < nThreads; thread++) {
            string op, key;
            string loadFilename = workloadPath + to_string(thread);
            printf("Loading keys from [%s] in DB...\n", loadFilename.c_str());
            ifstream infile(loadFilename);
            while (infile >> op >> key) {
                //printf("Put()  key=[%s]  value=[%s]\n", key.c_str(), value);
#ifdef USE_PMEMKV
                db_->put(key.c_str(), value);
#else
                Slice k {key.c_str()};
                Slice v {value};
                db_->Put(WriteOptions{}, k, v);
#endif
            }
        }
        free(value);

        // Populate traces/operations
        operations_ = 0;
        traces_ = (vector<string> **)malloc(sizeof(vector<string> *) * nThreads);
        for (unsigned int thread = 0; thread < nThreads; thread++) {
            traces_[thread] = new vector<string>();
            string workloadPath = workloadPrefix;
            workloadPath += "-run-" + to_string(nThreads) + ".";
            workloadPath += to_string(thread);
            printf("Loading operations from [%s] into memory...\n", workloadPath.c_str());
            string temp1, temp2;
            ifstream infile(workloadPath);
            while (infile >> temp1 >> temp2) {
                //printf("run for [%s]\n", (temp1 + " " + temp2).c_str()); // TODO: printf in PRONTO here
                traces_[thread]->push_back(temp1 + " " + temp2);
                operations_++;
            }
        }
        printf("Read %ld operations\n", operations_);
    }

    ~YCSBPTM() {
        free(keys_);
        free(values_);
        free(traces_);
        //delete db_;
        //db_->Close();
    }

    // Nothing to do since RocksDB is MT-safe
    void init(unsigned int mytid) {
        keys_[mytid] = (char *)malloc(MAX_KEY_SIZE);
        values_[mytid] = (char *)malloc(valueSize_+1);
        std::memcpy(values_[mytid], valueBuffer_, valueSize_);
        values_[mytid][valueSize_] = 0;
    }

    void cleanup(unsigned int mytid) {
        free(keys_[mytid]);
        free(values_[mytid]);
    }

    /*
    int totalKeys() {
        int totalKeys = 0;
        Iterator *it = db->NewIterator(ReadOptions());
        for (it->SeekToFirst(); it->Valid(); it->Next()) {
            if (it->value().ToString().length() == cfg->valueSize - 1) {
                totalKeys++;
            }
        }
        assert(it->status().ok());
        delete it;
        return totalKeys;
    }
    */
};

// This must be static to be passed to thread()
void worker(YCSBPTM* ycsb, int mytid) {
#ifdef USE_PMEMKV
#elif defined ROCKSDB
    WriteOptions writeOptions {};
    ReadOptions readOptions {};
    writeOptions.sync = true;
#else
    WriteOptions writeOptions {};
    ReadOptions readOptions {};
#endif
    unsigned int i = 0;
    char *key = ycsb->keys_[mytid];
    char *value = ycsb->values_[mytid];

    startAtZero.fetch_add(-1);
    // spin waiting for all other threads before starting the measurements
    // (we wait for startAtZero to be zero on the main thread).
    while (!startFlag.load()) ; // spin

    for (i = 0; i < ycsb->traces_[mytid]->size(); i++) {
        string t = ycsb->traces_[mytid]->at(i);
        //printf("t = %s\n", t.c_str());
        string tag = t.substr(0, 3);
        if (tag == "Add" || tag == "Upd") {
            memcpy(value, ycsb->valueBuffer_, ycsb->valueSize_);
            if (tag == "Add")
                strcpy(key, t.c_str() + 4);
            else // Update
                strcpy(key, t.c_str() + 7);
#ifdef USE_PMEMKV
            status s = ycsb->db_->put(key, value);
            if (s == status::OK) i++;
#else
            Slice k {key};
            Slice v {value};
            Status status = ycsb->db_->Put(writeOptions, k, v);
            if (status.ok()) i++;
#endif

        } else if (tag == "Rea") {
            string t_value;
            strcpy(key, t.c_str() + 5);
#ifdef USE_PMEMKV
            status s = ycsb->db_->get(key, &t_value);
            if (s == status::OK) i++;
#else
            Status status = ycsb->db_->Get(readOptions, key, &t_value);
            if (status.ok()) i++;
#endif

        }
    }
}


//}; // end namespace ptmdb


// run with this:
// bin/ycsb_trinvrfc 1 ~/pronto-v1.1/benchmark/workloads/1m/a
//
int main(int argc, char* argv[]) {

    if (argc <= 2) {
        printf("ERROR: too few arguments. Use like this:\n");
        printf("bin/ycsb [nThreads] [workloadFolder]\n");
        return 0;
    }
    int nThreads = atoi(argv[1]);
    string workloadPrefix{argv[2]};
    printf("threads=%d  workloadPrefix=[%s]\n", nThreads, argv[2]);
    startAtZero.store(nThreads);

    // Load phase
    YCSBPTM ycsb {nThreads, workloadPrefix};

    // Run phase
    printf("Starting up %d threads for 'Run phase'\n", nThreads);
    thread workers[nThreads];
    for (int tid = 0; tid < nThreads; tid++) ycsb.init(tid);
    for (int tid = 0; tid < nThreads; tid++) workers[tid] = thread(worker, &ycsb, tid);
    // Rendez-vous
    this_thread::sleep_for(100ms);
    // Wait for startAtZero to be zero
    while (startAtZero.load() != 0) ;
    auto startBeats = steady_clock::now();
    startFlag.store(true);
    this_thread::sleep_for(1ms);
    for (int tid = 0; tid < nThreads; tid++) workers[tid].join();
    auto stopBeats = steady_clock::now();
    for (int tid = 0; tid < nThreads; tid++) ycsb.cleanup(tid);

    // Show results
    uint64_t lengthSec = (stopBeats-startBeats).count();
    printf("%d %lld\n", nThreads, ycsb.operations_*1000000000LL/lengthSec);
}
