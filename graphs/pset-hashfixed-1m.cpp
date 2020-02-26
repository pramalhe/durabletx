#include <iostream>
#include <fstream>
#include <cstring>
#include "pdatastructures/TMHashMapFixedSizeWF.hpp"
#include "pdatastructures/TMHashMapFixedSize.hpp"
#include "ptms/ptm.h"

#ifdef USE_CXPTM
#include "ptms/cxptm/CXPTM.hpp"
#define DATA_FILE "data/pset-hashfixed-1m-cxptm.txt"
#elif defined USE_CXREDO
#include "ptms/cxredo/CXRedo.hpp"
#define DATA_FILE "data/pset-hashfixed-1m-cxredo.txt"
#elif defined USE_CXREDOTIMED
#include "ptms/cxredotimed/CXRedoTimed.hpp"
#define DATA_FILE "data/pset-hashfixed-1m-cxredotimed.txt"
#elif defined USE_ROMLR
#include "ptms/romuluslr/RomulusLR.hpp"
#define DATA_FILE "data/pset-hashfixed-1m-romlr.txt"
#elif defined USE_ROMLOG
#include "ptms/romuluslog/RomulusLog.hpp"
#define DATA_FILE "data/pset-hashfixed-1m-romlog.txt"
#elif defined USE_ROM_LOG_FC
#include "ptms/romuluslog/RomLogFC.hpp"
#define DATA_FILE "data/pset-hashfixed-1m-romlogfc.txt"
#elif defined USE_OFLF
#include "ptms/onefile/OneFilePTMLF.hpp"
#define DATA_FILE "data/pset-hashfixed-1m-oflf.txt"
#elif defined USE_OFWF
#include "ptms/onefile/OneFilePTMWF.hpp"
#define DATA_FILE "data/pset-hashfixed-1m-ofwf.txt"
#endif
#include "PBenchmarkSets.hpp"

#ifndef DATA_FILE
#define DATA_FILE "data/pset-hashfixed-1m-" PTM_FILEXT ".txt"
#endif

int main(int argc, char* argv[]) {
    const std::string dataFilename { DATA_FILE };
    vector<int> threadList = { 1, 2, 4, 8, 10, 16, 20, 24, 32, 40 }; // For Castor
    vector<int> ratioList = { 1000, 100, 10 };                       // Permil ratio: 100%, 10%, 1%
    const int numKeys = 1000*1000;                               // Number of keys in the set
    const int numRuns = 1;                                           // 5 runs for the paper
    // Read the number of seconds from the command line or use 20 seconds as default
    long secs = (argc >= 2) ? atoi(argv[1]) : 20;
    seconds testLength {secs};
    uint64_t results[threadList.size()][ratioList.size()];
    std::string cName;
    // Reset results
    std::memset(results, 0, sizeof(uint64_t)*threadList.size()*ratioList.size());

    double totalHours = (double)ratioList.size()*threadList.size()*testLength.count()*numRuns/(60.*60.);
    std::cout << "This benchmark is going to take " << totalHours << " hours to complete\n";
#ifdef USE_PMDK
    std::cout << "If you use PMDK, don't forget to set 'export PMEM_IS_PMEM_FORCE=1'\n";
#endif

    PBenchmarkSets<uint64_t> bench {};
    for (unsigned ir = 0; ir < ratioList.size(); ir++) {
        auto ratio = ratioList[ir];
        for (unsigned it = 0; it < threadList.size(); it++) {
            auto nThreads = threadList[it];
            std::cout << "\n----- Persistent Sets (HashSet)   numKeys=" << numKeys << "   ratio=" << ratio/10. << "%   threads=" << nThreads << "   runs=" << numRuns << "   length=" << testLength.count() << "s -----\n";
#ifdef USE_CXPTM
            results[it][ir] = bench.benchmark<TMHashMapFixedSizeWF<uint64_t,uint64_t,cx::CX,cx::persist>,                               cx::CX>                  (cName, ratio, testLength, numRuns, numKeys, nThreads);
#elif defined USE_CXREDO
            results[it][ir] = bench.benchmark<TMHashMapFixedSizeWF<uint64_t,uint64_t,cxredo::CXRedo,cxredo::persist>,                   cxredo::CXRedo>          (cName, ratio, testLength, numRuns, numKeys, nThreads);
#elif defined USE_CXREDOTIMED
            results[it][ir] = bench.benchmark<TMHashMapFixedSizeWF<uint64_t,uint64_t,cxredotimed::CXRedoTimed,cxredotimed::persist>,    cxredotimed::CXRedoTimed>(cName, ratio, testLength, numRuns, numKeys, nThreads);
#elif defined USE_ROMLR
            results[it][ir] = bench.benchmark<TMHashMapFixedSize<uint64_t,uint64_t,romuluslr::RomulusLR,romuluslr::persist>,     romuluslr::RomulusLR>    (cName, ratio, testLength, numRuns, numKeys, nThreads);
#elif defined USE_ROMLOG
            results[it][ir] = bench.benchmark<TMHashMapFixedSize<uint64_t,uint64_t,romuluslog::RomulusLog,romuluslog::persist>,  romuluslog::RomulusLog>  (cName, ratio, testLength, numRuns, numKeys, nThreads);
#elif defined USE_ROM_LOG_FC
            results[it][ir] = bench.benchmark<TMHashMapFixedSize<uint64_t,uint64_t,romlogfc::RomLog,romlogfc::persist>,          romlogfc::RomLog>        (cName, ratio, testLength, numRuns, numKeys, nThreads);
#elif defined USE_OFLF
            results[it][ir] = bench.benchmark<TMHashMapFixedSizeWF<uint64_t,uint64_t,poflf::OneFileLF,poflf::tmtype>,                   poflf::OneFileLF>        (cName, ratio, testLength, numRuns, numKeys, nThreads);
#elif defined USE_OFWF
            results[it][ir] = bench.benchmark<TMHashMapFixedSizeWF<uint64_t,uint64_t,pofwf::OneFileWF,pofwf::tmtype>,                   pofwf::OneFileWF>        (cName, ratio, testLength, numRuns, numKeys, nThreads);
#else
            results[it][ir] = bench.benchmark<TMHashMapFixedSize<uint64_t,uint64_t,PTM_CLASS,PTM_TYPE>, PTM_CLASS>(cName, ratio, testLength, numRuns, numKeys, nThreads);
#endif
        }
    }

    // Export tab-separated values to a file to be imported in gnuplot or excel
    ofstream dataFile;
    dataFile.open(dataFilename);
    dataFile << "Threads\t";
    // Printf class names and ratios for each column
    for (unsigned ir = 0; ir < ratioList.size(); ir++) {
        auto ratio = ratioList[ir];
        dataFile << cName << "-" << ratio/10. << "%"<< "\t";
    }
    dataFile << "\n";
    for (int it = 0; it < threadList.size(); it++) {
        dataFile << threadList[it] << "\t";
        for (unsigned ir = 0; ir < ratioList.size(); ir++) {
            dataFile << results[it][ir] << "\t";
        }
        dataFile << "\n";
    }
    dataFile.close();
    std::cout << "\nSuccessfuly saved results in " << dataFilename << "\n";

    return 0;
}
