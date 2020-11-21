/*
 * This benchmark executes Sequential Writes (integer write)
 */
#include <iostream>
#include <fstream>
#include <cstring>

// There is a different file name depending on the pwb used
#ifdef PWB_IS_CLWB
#define PWB_STR "clwb.txt"
#endif
#ifdef PWB_IS_CLFLUSHOPT
#define PWB_STR "clflushopt.txt"
#endif
#ifdef PWB_IS_CLFLUSH
#define PWB_STR "clflush.txt"
#endif
#ifdef PWB_IS_NOP
#define PWB_STR "nop.txt"
#endif

#ifdef USE_ROM_LOG_FC
#include "ptms/romuluslog/RomLogFC.hpp"
#define DATA_FILE "data/seqwrite-integer-seq-romlogfc-"
#elif defined USE_ROM_LOG_2F_FC
#include "ptms/romuluslog/RomLog2FFC.hpp"
#define DATA_FILE "data/seqwrite-integer-seq-romlog2ffc-"
#elif defined USE_ROM_LOG_TS_FC
#include "ptms/romuluslog/RomLogTSFC.hpp"
#define DATA_FILE "data/seqwrite-integer-seq-romlogtsfc-"
#elif defined USE_ROMLOG
#include "ptms/romuluslog/RomulusLog.hpp"
#define DATA_FILE "data/seqwrite-integer-seq-romlog-"
#endif
#include "PBenchmarkSeqWriteSequential.hpp"



int main(int argc, char* argv[]) {
    const std::string dataFilename { DATA_FILE PWB_STR };
    vector<long> writeTxList = { 1, 2, 4, 8, 16, 32, 64, 128, 256, 512, 1024 };   // Number of writes per transactions
    const int numRuns = 1;                                  // 5 runs for the paper
    // Read the number of seconds from the command line or use 20 seconds as default
    long secs = (argc >= 2) ? atoi(argv[1]) : 20;
    seconds testLength {secs};
    const int EMAX_CLASS = 10;
    int maxClass = 0;
    uint64_t results[writeTxList.size()];
    std::string cName;
    // Reset results
    std::memset(results, 0, sizeof(uint64_t)*writeTxList.size());

    // SPS Benchmarks multi-threaded
    std::cout << "\n----- Sequential Persistent SeqWrite Benchmark (single-threaded integer array sequential write) -----\n";
    for (int is = 0; is < writeTxList.size(); is++) {
        int nWords = writeTxList[is];
        PBenchmarkSeqWriteSequential bench(1);
        std::cout << "\n----- threads=" << 1 << "   runs=" << numRuns << "   length=" << testLength.count() << "s   arraySize=" << arraySize << "   writes/tx=" << nWords << " -----\n";
#ifdef USE_ROM_LOG_FC // Specialized version of Romulus Log
        results[is] = bench.SeqWriteInteger<romlogfc::RomLog,        romlogfc::persist>    (cName, testLength, nWords, numRuns);
#elif defined USE_ROM_LOG_2F_FC
        results[is] = bench.SeqWriteInteger<romlog2ffc::RomLog,      romlog2ffc::persist>  (cName, testLength, nWords, numRuns);
#elif defined USE_ROM_LOG_TS_FC
        results[is] = bench.SeqWriteInteger<romlogtsfc::RomLog,      romlogtsfc::persist>  (cName, testLength, nWords, numRuns);
#elif defined USE_ROMLOG
        results[is] = bench.SeqWriteInteger<romuluslog::RomulusLog,  romuluslog::persist>  (cName, testLength, nWords, numRuns);
#endif
    }
    std::cout << "\n";

    // Export tab-separated values to a file to be imported in gnuplot or excel
    ofstream dataFile;
    dataFile.open(dataFilename);
    dataFile << "Swaps/Tx\t" << cName << "\n";
    for (unsigned is = 0; is < writeTxList.size(); is++) {
        int nWords = writeTxList[is];
        dataFile << nWords << "\t" << results[is] << "\n";
    }
    dataFile.close();
    std::cout << "\nSuccessfuly saved results in " << dataFilename << "\n";

    return 0;
}
