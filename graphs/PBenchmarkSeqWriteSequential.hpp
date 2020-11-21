/*
 * Copyright 2017-2019
 *   Andreia Correia <andreia.veiga@unine.ch>
 *   Pedro Ramalhete <pramalhe@gmail.com>
 *   Pascal Felber <pascal.felber@unine.ch>
 *
 * This work is published under the MIT license. See LICENSE.txt
 */
#ifndef _PERSISTENT_BENCHMARK_SeqWrite_SEQUENTIAL_H_
#define _PERSISTENT_BENCHMARK_SeqWrite_SEQUENTIAL_H_

#include <atomic>
#include <chrono>
#include <thread>
#include <string>
#include <vector>
#include <algorithm>
#include <cassert>
#include <iostream>
#include <typeinfo>

static const int arraySize=1000*1000;

using namespace std;
using namespace chrono;


/**
 * This is a micro-benchmark with integer swaps (SPS) for PTMs
 * Same as PBenchmarkSPS.hpp but for sequential code only
 */
class PBenchmarkSeqWriteSequential {

private:
    int numThreads;

public:
    PBenchmarkSeqWriteSequential(int numThreads) {
        this->numThreads = numThreads;
    }


    /*
     * An array of integers that gets randomly permutated.
     */
    template<typename PTM, template<typename> class PERSIST>
    uint64_t SeqWriteInteger(std::string& className, const seconds testLengthSeconds, const long numWritesPerTx, const int numRuns) {
        long long ops[numThreads][numRuns];
        long long lengthSec[numRuns];
        atomic<bool> startFlag = { false };
        atomic<bool> quit = { false };

        // Create the array of integers and initialize it, saving it in root pointer 0
        int larraySize = arraySize;
        PTM::updateTxSeq([larraySize] () {
            //PTM::pfree( PTM::template get_object<PERSIST<uint64_t>>(0) ); // TODO: re-enable this after we add the clear of objects as a transaction in CX
            PTM::put_object(0, PTM::pmalloc( larraySize*sizeof(PERSIST<uint64_t>*) ));
        });

        auto func = [this,&startFlag,&quit,&numWritesPerTx](long long *ops, const int tid) {
            uint64_t seed = (tid*1024)+tid+1234567890123456781ULL;
            int larraySize = arraySize;
            // Spin until the startFlag is set
            while (!startFlag.load()) {}
            // Do transactions until the quit flag is set
            long long tcount = 0;
            while (!quit.load()) {
                // Everything has to be captured by value, or get/put in root pointers
                PTM::updateTxSeq([&] () {
                    PERSIST<uint64_t>* parray = PTM::template get_object<PERSIST<uint64_t>>(0);
                    seed = randomLong(seed);
                    auto ia = seed%arraySize;
                    for (int i = 0; i < numWritesPerTx; i++) {
                        parray[ia] = tcount;
                        ia = (ia + i)%arraySize;
                    }
                });
                ++tcount;
            }
            *ops = tcount;
        };
        for (int irun = 0; irun < numRuns; irun++) {
            if (irun == 0) {
                className = PTM::className();
                cout << "##### " << PTM::className() << " #####  \n";
            }
            thread enqdeqThreads[numThreads];
            for (int tid = 0; tid < numThreads; tid++) enqdeqThreads[tid] = thread(func, &ops[tid][irun], tid);
            auto startBeats = steady_clock::now();
            startFlag.store(true);
            // Sleep for 20 seconds
            this_thread::sleep_for(testLengthSeconds);
            quit.store(true);
            auto stopBeats = steady_clock::now();
            for (int tid = 0; tid < numThreads; tid++) enqdeqThreads[tid].join();
            lengthSec[irun] = (stopBeats-startBeats).count();
            startFlag.store(false);
            quit.store(false);
        }

        PTM::updateTxSeq([&] () {
            PTM::pfree( PTM::template get_object<PERSIST<uint64_t>>(0) );
            PTM::template put_object<void>(0, nullptr);
        });

        // Accounting
        vector<long long> agg(numRuns);
        for (int irun = 0; irun < numRuns; irun++) {
        	for(int i=0;i<numThreads;i++){
        		agg[irun] += ops[i][irun]*1000000000LL/lengthSec[irun];
        	}
        }
        // Compute the median. numRuns should be an odd number
        sort(agg.begin(),agg.end());
        auto maxops = agg[numRuns-1];
        auto minops = agg[0];
        auto medianops = agg[numRuns/2];
        auto delta = (long)(100.*(maxops-minops) / ((double)medianops));
        // Printed value is the median of the number of ops per second that all threads were able to accomplish (on average)
        std::cout << "Writes/sec = " << medianops*numWritesPerTx << "     delta = " << delta*numWritesPerTx << "%   min = " << minops*numWritesPerTx << "   max = " << maxops*numWritesPerTx << "\n";
        return medianops*numWritesPerTx;
    }


    /**
     * An imprecise but fast random number generator
     */
    static uint64_t randomLong(uint64_t x) {
        x ^= x >> 12; // a
        x ^= x << 25; // b
        x ^= x >> 27; // c
        return x * 2685821657736338717LL;
    }
};

#endif
