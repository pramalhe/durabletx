#include <iostream>
#include <fstream>
#include <cstring>
#include "pdatastructures/psequential/PSeqHashMap.hpp"
#include "PBenchmarkSets.hpp"
#include "ptms/ptm.h"

#ifdef USE_ROM_LOG_FC
#include "ptms/romuluslog/RomLogFC.hpp"
#define DATA_FILE "data/pset-hash-1m-seq-romlogfc.txt"
#elif defined USE_ROM_LOG_2F_FC
#include "ptms/romuluslog/RomLog2FFC.hpp"
#define DATA_FILE "data/pset-hash-1m-seq-romlog2ffc.txt"
#elif defined USE_ROM_LOG_TS_FC
#include "ptms/romuluslog/RomLogTSFC.hpp"
#define DATA_FILE "data/pset-hash-1m-seq-romlogtsfc.txt"
#endif

#ifndef DATA_FILE
#define DATA_FILE "data/pset-hash-1m-seq-" PTM_FILEXT ".txt"
#endif

int main(int argc, char* argv[]) {
    const std::string dataFilename { DATA_FILE };
    vector<int> threadList = { 1 };                              // For castor
    vector<int> ratioList = { 1000, 100, 10 };                   // Permil ratio: 100%, 10%, 1%
    const int numKeys = 1000*1000;                           // Number of keys in the set
    const int numRuns = 1;                                       // 5 runs for the paper
    // Read the number of seconds from the command line or use 20 seconds as default
    long secs = (argc >= 2) ? atoi(argv[1]) : 20;
    seconds testLength {secs};
    const int EMAX_CLASS = 10;
    uint64_t results[EMAX_CLASS][ratioList.size()];
    std::string cNames[EMAX_CLASS];
    int maxClass = 0;
    // Reset results
    std::memset(results, 0, sizeof(uint64_t)*EMAX_CLASS*ratioList.size());

    double totalHours = (double)ratioList.size()*threadList.size()*testLength.count()*numRuns/(60.*60.);
    std::cout << "This benchmark is going to take " << totalHours << " hours to complete\n";

    PBenchmarkSets<uint64_t> bench {};
    for (unsigned ir = 0; ir < ratioList.size(); ir++) {
        auto ratio = ratioList[ir];
        for (unsigned it = 0; it < threadList.size(); it++) {
            auto nThreads = threadList[it];
            int ic = 0;
            std::cout << "\n----- Persistent Sets (Hash Map)   numKeys=" << numKeys << "   ratio=" << ratio/10. << "%   threads=" << nThreads << "   runs=" << numRuns << "   length=" << testLength.count() << "s -----\n";
#ifdef USE_ROM_LOG_FC
            results[ic][ir] = bench.benchmark<PSeqHashMap<uint64_t,uint64_t,romlogfc::RomLog,romlogfc::persist>,         romlogfc::RomLog>         (cNames[ic], ratio, testLength, numRuns, numKeys);
            ic++;
#elif defined USE_ROM_LOG_2F_FC
            results[ic][ir] = bench.benchmark<PSeqHashMap<uint64_t,uint64_t,romlog2ffc::RomLog,romlog2ffc::persist>,     romlog2ffc::RomLog>       (cNames[ic], ratio, testLength, numRuns, numKeys);
            ic++;
#elif defined USE_ROM_LOG_TS_FC
            results[ic][ir] = bench.benchmark<PSeqHashMap<uint64_t,uint64_t,romlogtsfc::RomLog,romlogtsfc::persist>,     romlogtsfc::RomLog>       (cNames[ic], ratio, testLength, numRuns, numKeys);
            ic++;
#else
            results[ic][ir] = bench.benchmark<PSeqHashMap<uint64_t,uint64_t,PTM_CLASS,PTM_TYPE>, PTM_CLASS>(cNames[ic], ratio, testLength, numRuns, numKeys);
            ic++;
#endif
            maxClass = ic;
        }
    }

    // Export tab-separated values to a file to be imported in gnuplot or excel
    ofstream dataFile;
    dataFile.open(dataFilename);
    // Printf class names and ratios for each column
    for (unsigned ir = 0; ir < ratioList.size(); ir++) {
        auto ratio = ratioList[ir];
        for (int ic = 0; ic < maxClass; ic++) dataFile << cNames[ic] << "-" << ratio/10. << "%"<< "\t";
    }
    dataFile << "\n";
    for (unsigned ir = 0; ir < ratioList.size(); ir++) {
        for (int ic = 0; ic < maxClass; ic++) dataFile << results[ic][ir] << "\t";
    }
    dataFile << "\n";
    dataFile.close();
    std::cout << "\nSuccessfuly saved results in " << dataFilename << "\n";

    return 0;
}
