#include <iostream>
#include <fstream>
#include <cstring>
#include "PBenchmarkQueues.hpp"
#include "pdatastructures/queues/TMLinkedListQueueWF.hpp"
#include "pdatastructures/queues/TMLinkedListQueueByRef.hpp"
#include "ptms/ptm.h"

// Macros suck, but we can't have multiple TMs at the same time (too much memory)
#ifdef USE_CXPTM
#include "ptms/cxptm/CXPTM.hpp"
#define DATA_FILE "data/pq-ll-cxptm.txt"
#elif defined USE_CXREDO
#include "ptms/cxredo/CXRedo.hpp"
#define DATA_FILE "data/pq-ll-cxredo.txt"
#elif defined USE_CXREDOTIMED
#include "ptms/cxredotimed/CXRedoTimed.hpp"
#define DATA_FILE "data/pq-ll-cxredotimed.txt"
#elif defined USE_ROMLR
#include "ptms/romuluslr/RomulusLR.hpp"
#define DATA_FILE "data/pq-ll-romlr.txt"
#elif defined USE_ROMLOG
#include "ptms/romuluslog/RomulusLog.hpp"
#define DATA_FILE "data/pq-ll-romlog.txt"
#elif defined USE_ROM_LOG_FC
#include "ptms/romuluslog/RomLogFC.hpp"
#define DATA_FILE "data/pq-ll-romlogfc.txt"
#elif defined USE_OFWF
#include "ptms/onefile/OneFilePTMWF.hpp"
#define DATA_FILE "data/pq-ll-ofwf.txt"
#elif defined USE_FRIEDMAN
#include "ptms/pmdk/PMDKTM.hpp"
#include "pdatastructures/pqueues/PFriedmanQueue.hpp"
#define DATA_FILE "data/pq-ll-friedman.txt"
#endif

#ifndef DATA_FILE
#define DATA_FILE "data/pq-ll-" PTM_FILEXT ".txt"
#endif


#define MILLION  1000000LL

int main(void) {
    const std::string dataFilename { DATA_FILE };
    vector<int> threadList = { 1, 2, 4, 8, 10, 16, 24, 32, 40 };     // For Castor
    const int numRuns = 1;                                           // Number of runs
#ifdef USE_FRIEDMAN
    const long numPairs = 1*MILLION;                                 // FHMP leaks so we can't do as many iterations
#else
    const long numPairs = 10*MILLION;                                // 10M is fast enough on the laptop, but on AWS we can use 100M
#endif
    uint64_t results[threadList.size()];
    std::string cName;
    // Reset results
    std::memset(results, 0, sizeof(uint64_t)*threadList.size());

    // Enq-Deq Throughput benchmarks
    for (int it = 0; it < threadList.size(); it++) {
        int nThreads = threadList[it];
        PBenchmarkQueues bench(nThreads);
        std::cout << "\n----- pq-ll (enqueue-dequeue)   threads=" << nThreads << "   pairs=" << numPairs/MILLION << "M   runs=" << numRuns << " -----\n";
#ifdef USE_CXPTM
            results[it] = bench.enqDeq<TMLinkedListQueue<uint64_t,cx::CX,cx::persist>,                              cx::CX>                  (cName, numPairs, numRuns);
#elif defined USE_CXREDO
            results[it] = bench.enqDeq<TMLinkedListQueue<uint64_t,cxredo::CXRedo,cxredo::persist>,                  cxredo::CXRedo>          (cName, numPairs, numRuns);
#elif defined USE_CXREDOTIMED
            results[it] = bench.enqDeq<TMLinkedListQueue<uint64_t,cxredotimed::CXRedoTimed,cxredotimed::persist>,   cxredotimed::CXRedoTimed>(cName, numPairs, numRuns);
#elif defined USE_ROMLR
            results[it] = bench.enqDeq<TMLinkedListQueueByRef<uint64_t,romuluslr::RomulusLR,romuluslr::persist>,    romuluslr::RomulusLR>    (cName, numPairs, numRuns);
#elif defined USE_ROMLOG
            results[it] = bench.enqDeq<TMLinkedListQueueByRef<uint64_t,romuluslog::RomulusLog,romuluslog::persist>, romuluslog::RomulusLog>  (cName, numPairs, numRuns);
#elif defined USE_ROM_LOG_FC
            results[it] = bench.enqDeq<TMLinkedListQueueByRef<uint64_t,romlogfc::RomLog,romlogfc::persist>,         romlogfc::RomLog>        (cName, numPairs, numRuns);
#elif defined USE_OFWF
            results[it] = bench.enqDeq<TMLinkedListQueue<uint64_t,pofwf::OneFileWF,pofwf::tmtype>,                  pofwf::OneFileWF>        (cName, numPairs, numRuns);
#elif defined USE_DUALZONE_2F_FC
            results[it] = bench.enqDeq<TMLinkedListQueueByRef<uint64_t,dualzone2ffc::DualZone,dualzone2ffc::persist>, dualzone2ffc::DualZone>(cName, numPairs, numRuns);
#elif defined USE_DZ_V1
            results[it] = bench.enqDeq<TMLinkedListQueueByRef<uint64_t,dzv1::DualZone,dzv1::persist>,               dzv1::DualZone>          (cName, numPairs, numRuns);
#elif defined USE_DZ_V2
            results[it] = bench.enqDeq<TMLinkedListQueueByRef<uint64_t,dzv2::DualZone,dzv2::persist>,               dzv2::DualZone>          (cName, numPairs, numRuns);
#elif defined USE_DZ_V3
            results[it] = bench.enqDeq<TMLinkedListQueueByRef<uint64_t,dzv3::DualZone,dzv3::persist>,               dzv3::DualZone>          (cName, numPairs, numRuns);
#elif defined USE_DZ_V4
            results[it] = bench.enqDeq<TMLinkedListQueueByRef<uint64_t,dzv4::DualZone,dzv4::persist>,               dzv4::DualZone>          (cName, numPairs, numRuns);
#elif defined USE_DZ_TL2
            results[it] = bench.enqDeq<TMLinkedListQueueByRef<uint64_t,dztl2::DualZone,dztl2::persist>,             dztl2::DualZone>         (cName, numPairs, numRuns);
#elif defined USE_REDO_LOG_FC
            results[it] = bench.enqDeq<TMLinkedListQueueByRef<uint64_t,redologfc::RedoLog,redologfc::persist>,      redologfc::RedoLog>      (cName, numPairs, numRuns);
#elif defined USE_REDO_LOG_TL2
            results[it] = bench.enqDeq<TMLinkedListQueueByRef<uint64_t,redologtl2::RedoLog,redologtl2::persist>,    redologtl2::RedoLog>     (cName, numPairs, numRuns);
#elif defined USE_FRIEDMAN
            results[it] = bench.enqDeq<PFriedmanQueue<uint64_t>,                                                    pmdk::PMDKTM>            (cName, numPairs, numRuns);
#else
            results[it] = bench.enqDeq<TMLinkedListQueueByRef<uint64_t,PTM_CLASS,PTM_TYPE>, PTM_CLASS>    (cName, numPairs, numRuns);
#endif
    }

    // Export tab-separated values to a file to be imported in gnuplot or excel
    ofstream dataFile;
    dataFile.open(dataFilename);
    dataFile << "Threads\t";
    // Printf class names for each column
    dataFile << cName << "\t";
    dataFile << "\n";
    for (int it = 0; it < threadList.size(); it++) {
        dataFile << threadList[it] << "\t";
        dataFile << results[it] << "\t";
        dataFile << "\n";
    }
    dataFile.close();
    std::cout << "\nSuccessfuly saved results in " << dataFilename << "\n";

    return 0;
}
