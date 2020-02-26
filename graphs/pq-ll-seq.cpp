#include <iostream>
#include <fstream>
#include <cstring>
#include "pdatastructures/psequential/PSeqLinkedListQueue.hpp"
#include "ptms/ptm.h"

#ifdef USE_ROM_LOG_FC
#include "ptms/romuluslog/RomLogFC.hpp"
#define DATA_FILE "data/pq-ll-enq-deq-seq-romlogfc.txt"
#elif defined USE_ROM_LOG_2F_FC
#include "ptms/romuluslog/RomLog2FFC.hpp"
#define DATA_FILE "data/pq-ll-enq-deq-seq-romlog2ffc.txt"
#elif defined USE_ROM_LOG_TS_FC
#include "ptms/romuluslog/RomLogTSFC.hpp"
#define DATA_FILE "data/pq-ll-enq-deq-seq-romlogtsfc.txt"
#endif

#ifndef DATA_FILE
#define DATA_FILE "data/pq-ll-enq-deq-seq-" PTM_FILEXT ".txt"
#endif

#include "PBenchmarkQueues.hpp"


int main(void) {
    const std::string dataFilename { DATA_FILE };
    vector<int> threadList = { 1 };
    const int numPairs = 10*1000*1000;                  // Number of pairs of items to enqueue-dequeue. 1M for the paper
    const int numRuns = 1;                              // 5 runs for the paper
    uint64_t results = 0;
    std::string cName;

    PBenchmarkQueues bench(1);
    std::cout << "\n----- Sequential Persistent Queues (Linked-Lists)   numPairs=" << numPairs << "   runs=" << numRuns << " -----\n";
#ifdef USE_ROM_LOG_FC
    results = bench.enqDeqSequential<PSeqLinkedListQueue<uint64_t,romlogfc::RomLog,romlogfc::persist>,romlogfc::RomLog>                (cName, numPairs, numRuns);
#elif defined USE_ROM_LOG_2F_FC
    results = bench.enqDeqSequential<PSeqLinkedListQueue<uint64_t,romlog2ffc::RomLog,romlog2ffc::persist>,romlog2ffc::RomLog>          (cName, numPairs, numRuns);
#elif defined USE_ROM_LOG_TS_FC
    results = bench.enqDeqSequential<PSeqLinkedListQueue<uint64_t,romlogtsfc::RomLog,romlogtsfc::persist>,romlogtsfc::RomLog>          (cName, numPairs, numRuns);
#else
    results = bench.enqDeqSequential<PSeqLinkedListQueue<uint64_t,PTM_CLASS,PTM_TYPE>, PTM_CLASS>(cName, numPairs, numRuns);
#endif

    // Export tab-separated values to a file to be imported in gnuplot or excel
    ofstream dataFile;
    dataFile.open(dataFilename);
    // Printf class name
    dataFile << cName << "\t";
    dataFile << "\n";
    dataFile << results << "\t";
    dataFile << "\n";
    dataFile.close();
    std::cout << "\nSuccessfuly saved results in " << dataFilename << "\n";

    return 0;
}
