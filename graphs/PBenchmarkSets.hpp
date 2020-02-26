/*
 * Copyright 2017-2020
 *   Andreia Correia <andreia.veiga@unine.ch>
 *   Pedro Ramalhete <pramalhe@gmail.com>
 *   Pascal Felber <pascal.felber@unine.ch>
 *
 * This work is published under the MIT license. See LICENSE.txt
 */
#ifndef _PERSISTENT_BENCHMARK_SETS_H_
#define _PERSISTENT_BENCHMARK_SETS_H_

#include <atomic>
#include <chrono>
#include <thread>
#include <string>
#include <vector>
#include <random>
#include <algorithm>
#include <iostream>

using namespace std;
using namespace chrono;

template <template <typename> class TMTYPE>
struct UserData  {
    TMTYPE<long long> seq;
    TMTYPE<int> tid;
    UserData(long long lseq, int ltid=0) {
        this->seq = lseq;
        this->tid = ltid;
    }
    UserData() {
        this->seq = -2;
        this->tid = -2;
    }
    UserData(const UserData &other) : seq(other.seq), tid(other.tid) { }

    bool operator < (const UserData& other) const {
        return seq.pload() < other.seq.pload();
    }
    bool operator == (const UserData& other) const {
        return seq.pload() == other.seq.pload() && tid.pload() == other.tid.pload();
    }
};

#ifdef NEVER
namespace std {
    template <>
    struct hash<UserData> {
        std::size_t operator()(const UserData& k) const {
            using std::size_t;
            using std::hash;
            return (hash<long long>()(k.seq.pload()));  // This hash has no collisions, which is irealistic
            /*
            long long x = k.seq;
            x ^= x >> 12; // a
            x ^= x << 25; // b
            x ^= x >> 27; // c
            return hash<long long>()(x * 2685821657736338717LL);
            */
        }
    };
}
#endif


/**
 * This is a micro-benchmark of sets, used in the CX paper
 */
template<typename K>
class PBenchmarkSets {
    bool  set_is_init_ = false;
    void* udarray_ = nullptr;
    void* set_ = nullptr;

private:
    struct Result {
        nanoseconds nsEnq = 0ns;
        nanoseconds nsDeq = 0ns;
        long long numEnq = 0;
        long long numDeq = 0;
        long long totOpsSec = 0;

        Result() { }

        Result(const Result &other) {
            nsEnq = other.nsEnq;
            nsDeq = other.nsDeq;
            numEnq = other.numEnq;
            numDeq = other.numDeq;
            totOpsSec = other.totOpsSec;
        }

        bool operator < (const Result& other) const {
            return totOpsSec < other.totOpsSec;
        }
    };

    static const long long NSEC_IN_SEC = 1000000000LL;

public:

    /**
     * When doing "updates" we execute a random removal and if the removal is successful we do an add() of the
     * same item immediately after. This keeps the size of the data structure equal to the original size (minus
     * MAX_THREADS items at most) which gives more deterministic results.
     */
    template<typename S, typename PTM>
    long long benchmark(std::string& className, const int updateRatio, const seconds testLengthSeconds, const int numRuns,
                        const int numElements, const int numThreads=1, const bool dedicated=false) {
    	int num_threads = numThreads;
    	if (dedicated) num_threads = numThreads+2;
        long long ops[num_threads][numRuns];
        long long lengthSec[numRuns];
        atomic<bool> quit = { false };
        atomic<bool> startFlag = { false };
        atomic<int> startAtZero = { false };

        className = S::className();
        std::cout << "##### " << S::className() << " #####  \n";

        if (!set_is_init_) {
            // Create all the keys in the concurrent set (2x)
            udarray_ = new K*[2*numElements];
            K** udarray = (K**)udarray_;
            for (int i = 0; i < 2*numElements; i++) udarray[i] = new K(i);
            // Randomly shuffle the 2*numElements keys to have an initial random insertion
            std::random_device rd;
            auto seed = rd();
            std::mt19937 g(seed);
            std::shuffle(udarray, udarray + 2*numElements, g);
            // Create the set
            PTM::template updateTx([&] () {
                set_ = PTM::template tmNew<S>();
            });
            S* set = (S*)set_;
            // Add numElements keys to the set, but one at a time, otherwise the transaction is too big to fit in the logs
            set->addAll(udarray, numElements);
            set_is_init_ = true;
        }
        S* set = (S*)set_;
        K** udarray = (K**)udarray_;

        // Can either be a Reader or a Writer
        auto rw_lambda = [this,&quit,&startFlag,&startAtZero,&set,&udarray,&numElements](const int updateRatio, long long *ops, const int tid) {
            long long numOps = 0;
            uint64_t seed = tid*133 + 1234567890123456781ULL;
            // don't do any warmup, no need for these PTMs
            /*
            // Execute 100k iterations as warmup and then spin wwait for all other threads
            for (uint64_t iter = 0; iter < 100*1000; iter++) {
                seed = randomLong(seed);
                auto ix = (unsigned int)(seed%numElements);
                if (set->remove(*udarray[ix])) set->add(*udarray[ix]);
            }
            */
            startAtZero.fetch_add(-1);
            // spin waiting for all other threads before starting the measurements
            // (we wait for startAtZero to be zero on the main thread).
            while (!startFlag.load()) ; // spin
            uint64_t fails = 0;
            while (!quit.load()) {
                // Select a random key out of the 2*numElements
                seed = randomLong(seed);
                auto ix = (unsigned int)(seed % (2*numElements));
                if (isRandomUpdate(seed, updateRatio)) {
                    // I'm a Writer
                    if (isRandomAdd(seed)) {
                        set->add(*udarray[ix]);
                    } else {
                        set->remove(*udarray[ix]);
                    }
                    numOps++;
                } else {
                	// I'm a Reader
                    set->contains(*udarray[ix]);
                }
                numOps++;
            }
            *ops = numOps;
        };

        for (int irun = 0; irun < numRuns; irun++) {
            startAtZero.store(num_threads);
            thread rwThreads[num_threads];
            if (dedicated) {
                rwThreads[0] = thread(rw_lambda, 1000, &ops[0][irun], 0);
                rwThreads[1] = thread(rw_lambda, 1000, &ops[1][irun], 1);
                for (int tid = 2; tid < num_threads; tid++) rwThreads[tid] = thread(rw_lambda, updateRatio, &ops[tid][irun], tid);
            } else {
                for (int tid = 0; tid < num_threads; tid++) rwThreads[tid] = thread(rw_lambda, updateRatio, &ops[tid][irun], tid);
            }
            this_thread::sleep_for(100ms);
            // Wait for startAtZero to be zero (all threads have done the 100k iteration warmup)
            while (startAtZero.load() != 0) ;
            auto startBeats = steady_clock::now();
            startFlag.store(true);
            // Sleep for testLengthSeconds seconds
            this_thread::sleep_for(testLengthSeconds);
            quit.store(true);
            auto stopBeats = steady_clock::now();
            for (int tid = 0; tid < num_threads; tid++) {
            	rwThreads[tid].join();
            }
            lengthSec[irun] = (stopBeats-startBeats).count();
            if (dedicated) {
                // We don't account for the write-only operations but we aggregate the values from the two threads and display them
                std::cout << "Mutative transactions per second = " << (ops[0][irun] + ops[1][irun])*1000000000LL/lengthSec[irun] << "\n";
                ops[0][irun] = 0;
                ops[1][irun] = 0;
            }
            quit.store(false);
            startFlag.store(false);
            // Compute ops at the end of each run
            long long agg = 0;
            for (int tid = 0; tid < num_threads; tid++) {
                agg += ops[tid][irun]*1000000000LL/lengthSec[irun];
            }
        }

        // Clear the set, one key at a time and then delete the instance
        /* For now just leak
        for (int i = 0; i < numElements; i++) set->remove(*udarray[i]);
        PTM::template updateTx([=] () {
            PTM::tmDelete(set);
        });

        for (int i = 0; i < numElements; i++) delete udarray[i];
        delete[] udarray;
        */

        // Accounting
        vector<long long> agg(numRuns);
        for (int irun = 0; irun < numRuns; irun++) {
            for (int tid = 0; tid < num_threads; tid++) {
                agg[irun] += ops[tid][irun]*1000000000LL/lengthSec[irun];
            }
        }

        // Compute the median. numRuns must be an odd number
        sort(agg.begin(),agg.end());
        auto maxops = agg[numRuns-1];
        auto minops = agg[0];
        auto medianops = agg[numRuns/2];
        auto delta = (long)(100.*(maxops-minops) / ((double)medianops));
        // Printed value is the median of the number of ops per second that all threads were able to accomplish (on average)
        std::cout << "Ops/sec = " << medianops << "      delta = " << delta << "%   min = " << minops << "   max = " << maxops << "\n";
        return medianops;
    }



    /**
     * An imprecise but fast random number generator
     */
    uint64_t randomLong(uint64_t x) {
        x ^= x >> 12; // a
        x ^= x << 25; // b
        x ^= x >> 27; // c
        return x * 2685821657736338717LL;
    }

    // Takes a random number seed, uses it to call randomLong(), updates the seed and then returns
    // whether it is an update or a read-only tx.
    // 'updateRatio' is in permil
    bool isRandomUpdate(uint64_t& x, uint64_t updateRatio) {
        if (updateRatio == 1000) return true;
        if (updateRatio == 0) return false;
        x = randomLong(x);
        const uint64_t permil = 1000;
        const uint64_t mulfactor = 1024*1024; // Just to have less rounding errors
        uint64_t randRatio = x % (permil*mulfactor);
        return (randRatio <= updateRatio*mulfactor);
    }

    // Returns whether it's an insertion in the set, with 50% probability
    bool isRandomAdd(uint64_t& x) {
        x = randomLong(x);
        const uint64_t mulfactor = 1024*1024*1024ULL;
        uint64_t rx = x % mulfactor;
        return (rx < mulfactor/2);
    }
};

#endif
