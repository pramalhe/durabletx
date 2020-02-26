/*
 * Copyright 2017-2020
 *   Andreia Correia <andreia.veiga@unine.ch>
 *   Pedro Ramalhete <pramalhe@gmail.com>
 *   Pascal Felber <pascal.felber@unine.ch>
 *
 * This work is published under the MIT license. See LICENSE.txt
 */
#ifndef _PERSISTENT_BENCHMARK_STACKS_H_
#define _PERSISTENT_BENCHMARK_STACKS_H_

#include <atomic>
#include <chrono>
#include <thread>
#include <string>
#include <vector>
#include <algorithm>
#include <cassert>


using namespace std;
using namespace chrono;

struct UserData  {
    long long seq;
    int tid;
    UserData(long long lseq, int ltid) {
        this->seq = lseq;
        this->tid = ltid;
    }
    UserData() {
        this->seq = -2;
        this->tid = -2;
    }
    UserData(const UserData &other) : seq(other.seq), tid(other.tid) { }

    bool operator < (const UserData& other) const {
        return seq < other.seq;
    }
};


/**
 * This is a micro-benchmark for persistent queues
 */
class PBenchmarkStacks {

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

    // Performance benchmark constants
    static const long long kNumPairsWarmup =     1000000LL;     // Each threads does 1M iterations as warmup

    // Contants for Ping-Pong performance benchmark
    static const int kPingPongBatch = 1000;            // Each thread starts by injecting 1k items in the queue


    static const long long NSEC_IN_SEC = 1000000000LL;

    const uint64_t kNumElements = 100000; // 100k elements in the queue

    int numThreads;

public:

    PBenchmarkStacks(int numThreads) {
        this->numThreads = numThreads;
    }


    /**
     * enqueue-dequeue pairs: in each iteration a thread executes an enqueue followed by a dequeue;
     * the benchmark executes 10^8 pairs partitioned evenly among all threads;
     * WARNING: If you modify this, please modify enqDeqNoTransaction() also
     */
    template<typename STACK, typename PTM>
    uint64_t pushPop(std::string& className, const long numPairs, const int numRuns) {
        nanoseconds deltas[numThreads][numRuns];
        atomic<bool> startFlag = { false };
        STACK* stack = nullptr;
        className = STACK::className();
        cout << "##### " << className << " #####  \n";

        auto pushpop_lambda = [this,&startFlag,&numPairs,&stack](nanoseconds *delta, const int tid) {
            //UserData* ud = new UserData{0,0};
            uint64_t* ud = new uint64_t(42);
            while (!startFlag.load()) {} // Spin until the startFlag is set
            // Measurement phase
            auto startBeats = steady_clock::now();
            for (long long iter = 0; iter < numPairs/numThreads; iter++) {
                PTM::updateTx([&] () {
                    stack->push(ud);
                });
                PTM::updateTx([&] () {
                    if (stack->pop() == nullptr) cout << "Error at measurement pop() iter=" << iter << "\n";
                });
            }
            auto stopBeats = steady_clock::now();
            *delta = stopBeats - startBeats;
        };

        for (int irun = 0; irun < numRuns; irun++) {
            PTM::updateTx([&] () { // It's ok to capture by reference, only the main thread is active (but it is not ok for CX-PTM)
                stack = PTM::template tmNew<STACK>();
            });
            // Fill the queue with an initial amount of nodes
            uint64_t* ud = new uint64_t(41);
            for (uint64_t ielem = 0; ielem < kNumElements; ielem++) {
                PTM::updateTx([&] () {
                    stack->push(ud);
                });
            }
            thread enqdeqThreads[numThreads];
            for (int tid = 0; tid < numThreads; tid++) enqdeqThreads[tid] = thread(pushpop_lambda, &deltas[tid][irun], tid);
            startFlag.store(true);
            // Sleep for 2 seconds just to let the threads see the startFlag
            this_thread::sleep_for(2s);
            for (int tid = 0; tid < numThreads; tid++) enqdeqThreads[tid].join();
            startFlag.store(false);
            PTM::updateTx([&] () {
                PTM::tmDelete(stack);
            });
        }

        // Sum up all the time deltas of all threads so we can find the median run
        vector<nanoseconds> agg(numRuns);
        for (int irun = 0; irun < numRuns; irun++) {
            agg[irun] = 0ns;
            for (int tid = 0; tid < numThreads; tid++) {
                agg[irun] += deltas[tid][irun];
            }
        }

        // Compute the median. numRuns should be an odd number
        sort(agg.begin(),agg.end());
        auto median = agg[numRuns/2].count()/numThreads; // Normalize back to per-thread time (mean of time for this run)

        cout << "Total Ops/sec = " << numPairs*2*NSEC_IN_SEC/median << "\n";
        return (numPairs*2*NSEC_IN_SEC/median);
    }




    /*
     * WARNING: If you modify this, please modify enqDeq() also
     */
    template<typename Q, typename PTM>
    uint64_t pushPopSequential(std::string& className, const long numPairs, const int numRuns) {
        nanoseconds deltas[numThreads][numRuns];
        atomic<bool> startFlag = { false };
        Q* queue = nullptr;
        className = Q::className();
        cout << "##### " << className << " #####  \n";

        auto enqdeq_lambda = [this,&startFlag,&numPairs,&queue](nanoseconds *delta, const int tid) {
            //UserData* ud = new UserData{0,0};
            uint64_t* ud = new uint64_t(42);
            while (!startFlag.load()) {} // Spin until the startFlag is set
            // Warmup phase
            for (long long iter = 0; iter < numPairs/(numThreads*10); iter++) { // Do 1/10 iterations as warmup
                PTM::updateTxSeq([&] () {
                    queue->enqueue(*ud, tid);
                    if (queue->dequeue(tid) == queue->EMPTY) cout << "Error at warmup dequeueing iter=" << iter << "\n";
                });
            }
            // Measurement phase
            auto startBeats = steady_clock::now();
            for (long long iter = 0; iter < numPairs/numThreads; iter++) {
                PTM::updateTxSeq([&] () {
                    queue->enqueue(*ud, tid);
                    if (queue->dequeue(tid) == queue->EMPTY) cout << "Error at measurement dequeueing iter=" << iter << "\n";
                });
            }
            auto stopBeats = steady_clock::now();
            *delta = stopBeats - startBeats;
        };

        for (int irun = 0; irun < numRuns; irun++) {
            PTM::updateTxSeq([&] () { // It's ok to capture by reference, only the main thread is active (but it is not ok for CX-PTM)
                queue = PTM::template tmNew<Q>();
            });
            // Fill the queue with an initial amount of nodes
            uint64_t* ud = new uint64_t(47);
            for (uint64_t ielem = 0; ielem < kNumElements; ielem++) queue->enqueue(*ud);
            thread enqdeqThreads[numThreads];
            for (int tid = 0; tid < numThreads; tid++) enqdeqThreads[tid] = thread(enqdeq_lambda, &deltas[tid][irun], tid);
            startFlag.store(true);
            // Sleep for 2 seconds just to let the threads see the startFlag
            this_thread::sleep_for(2s);
            for (int tid = 0; tid < numThreads; tid++) enqdeqThreads[tid].join();
            startFlag.store(false);
            PTM::updateTxSeq([&] () {
                PTM::tmDelete(queue);
            });
        }

        // Sum up all the time deltas of all threads so we can find the median run
        vector<nanoseconds> agg(numRuns);
        for (int irun = 0; irun < numRuns; irun++) {
            agg[irun] = 0ns;
            for (int tid = 0; tid < numThreads; tid++) {
                agg[irun] += deltas[tid][irun];
            }
        }

        // Compute the median. numRuns should be an odd number
        sort(agg.begin(),agg.end());
        auto median = agg[numRuns/2].count()/numThreads; // Normalize back to per-thread time (mean of time for this run)

        cout << "Total Ops/sec = " << numPairs*2*NSEC_IN_SEC/median << "\n";
        return (numPairs*2*NSEC_IN_SEC/median);
    }

};

#endif
