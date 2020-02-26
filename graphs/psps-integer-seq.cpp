/*
 * This benchmark executes SPS (integer swap)
 */
#include <iostream>
#include <fstream>
#include <cstring>
#include "PBenchmarkSPSSequential.hpp"
#include "ptms/ptm.h"

#ifdef USE_ROM_LOG_FC
#include "ptms/romuluslog/RomLogFC.hpp"
#define DATA_FILE "data/psps-integer-seq-romlogfc-"
#elif defined USE_ROM_LOG_2F_FC
#include "ptms/romuluslog/RomLog2FFC.hpp"
#define DATA_FILE "data/psps-integer-seq-romlog2ffc-"
#elif defined USE_ROM_LOG_TS_FC
#include "ptms/romuluslog/RomLogTSFC.hpp"
#define DATA_FILE "data/psps-integer-seq-romlogtsfc-"
#elif defined USE_ROMLOG
#include "ptms/romuluslog/RomulusLog.hpp"
#define DATA_FILE "data/psps-integer-seq-romlog-"
#endif

#ifndef DATA_FILE
#define DATA_FILE "data/psps-integer-seq-" PTM_FILEXT ".txt"
#endif


int main(int argc, char* argv[]) {
    const std::string dataFilename { DATA_FILE };
    vector<long> swapsPerTxList = { 1, 2, 4, 8, 16, 32, 64, 128, 256, 512, 1024 };   // Number of swaps per transactions
    const int numRuns = 1;                                  // 5 runs for the paper
    // Read the number of seconds from the command line or use 20 seconds as default
    long secs = (argc >= 2) ? atoi(argv[1]) : 20;
    seconds testLength {secs};
    const int EMAX_CLASS = 10;
    int maxClass = 0;
    uint64_t results[swapsPerTxList.size()];
    std::string cName;
    // Reset results
    std::memset(results, 0, sizeof(uint64_t)*swapsPerTxList.size());

    // SPS Benchmarks multi-threaded
    std::cout << "\n----- Sequential Persistent SPS Benchmark (single-threaded integer array swap) -----\n";
    for (int is = 0; is < swapsPerTxList.size(); is++) {
        int nWords = swapsPerTxList[is];
        PBenchmarkSPSSequential bench(1);
        std::cout << "\n----- threads=" << 1 << "   runs=" << numRuns << "   length=" << testLength.count() << "s   arraySize=" << arraySize << "   swaps/tx=" << nWords << " -----\n";
#ifdef USE_ROM_LOG_FC // Specialized version of Romulus Log
        results[is] = bench.SPSInteger<romlogfc::RomLog,        romlogfc::persist>    (cName, testLength, nWords, numRuns);
#elif defined USE_ROM_LOG_2F_FC
        results[is] = bench.SPSInteger<romlog2ffc::RomLog,      romlog2ffc::persist>  (cName, testLength, nWords, numRuns);
#elif defined USE_ROM_LOG_TS_FC
        results[is] = bench.SPSInteger<romlogtsfc::RomLog,      romlogtsfc::persist>  (cName, testLength, nWords, numRuns);
#elif defined USE_ROMLOG
        results[is] = bench.SPSInteger<romuluslog::RomulusLog,  romuluslog::persist>  (cName, testLength, nWords, numRuns);
#else
        results[is] = bench.SPSInteger<PTM_CLASS,PTM_TYPE>(cName, testLength, nWords, numRuns);
#endif
    }
    std::cout << "\n";

    // Export tab-separated values to a file to be imported in gnuplot or excel
    ofstream dataFile;
    dataFile.open(dataFilename);
    dataFile << "Swaps/Tx\t" << cName << "\n";
    for (unsigned is = 0; is < swapsPerTxList.size(); is++) {
        int nWords = swapsPerTxList[is];
        dataFile << nWords << "\t" << results[is] << "\n";
    }
    dataFile.close();
    std::cout << "\nSuccessfuly saved results in " << dataFilename << "\n";

    return 0;
}
