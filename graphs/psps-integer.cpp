/*
 * This benchmark executes SPS (integer swap)
 */
#include <iostream>
#include <fstream>
#include <cstring>
#include "PBenchmarkSPS.hpp"
#include "ptms/ptm.h"

#ifdef USE_CXPTM
#include "ptms/cxptm/CXPTM.hpp"
#define DATA_FILE "data/psps-integer-cxptm.txt"
#elif defined USE_CXPUC
#error "SPS not (yet) implemented for CX-PUC"
#include "ptms/cxpuc/CXPUC.hpp"
#define DATA_FILE "data/psps-integer-cxpuc.txt"
#elif defined USE_CXREDO
#include "ptms/cxredo/CXRedo.hpp"
#define DATA_FILE "data/psps-integer-cxredo.txt"
#elif defined USE_CXREDOTIMED
#include "ptms/cxredotimed/CXRedoTimed.hpp"
#define DATA_FILE "data/psps-integer-cxredotimed.txt"
#elif defined USE_ROMLR
#include "ptms/romuluslr/RomulusLR.hpp"
#define DATA_FILE "data/psps-integer-romlr.txt"
#elif defined USE_ROMLOG
#include "ptms/romuluslog/RomulusLog.hpp"
#define DATA_FILE "data/psps-integer-romlog.txt"
#elif defined USE_ROM_LOG_FC
#include "ptms/romuluslog/RomLogFC.hpp"
#define DATA_FILE "data/psps-integer-romlogfc.txt"
#elif defined USE_OFLF
#include "ptms/onefile/OneFilePTMLF.hpp"
#define DATA_FILE "data/psps-integer-oflf.txt"
#elif defined USE_OFWF
#include "ptms/onefile/OneFilePTMWF.hpp"
#define DATA_FILE "data/psps-integer-ofwf.txt"
#endif

#ifndef DATA_FILE
#define DATA_FILE "data/psps-integer-" PTM_FILEXT ".txt"
#endif


int main(int argc, char* argv[]) {
    const std::string dataFilename { DATA_FILE };
    vector<int> threadList = { 1, 2, 4, 8, 10, 16, 20, 24, 32, 40 }; // For castor
    vector<long> swapsPerTxList = { 1, 4, 8, 16, 32, 64, 128/*, 256, 512*/ };    // With 512 it gets stuck on TL2 variants
    const int numRuns = 1;                                   // 5 runs for the paper
    // Read the number of seconds from the command line or use 20 seconds as default
    long secs = (argc >= 2) ? atoi(argv[1]) : 20;
    seconds testLength {secs};
    uint64_t results[threadList.size()][swapsPerTxList.size()];
    std::string cName;
    // Reset results
    std::memset(results, 0, sizeof(uint64_t)*threadList.size()*swapsPerTxList.size());

    // SPS Benchmarks multi-threaded
#ifdef USE_PMDK
    std::cout << "If you use PMDK, don't forget to set 'export PMEM_IS_PMEM_FORCE=1'\n";
#endif
    std::cout << "\n----- Persistent SPS Benchmark (multi-threaded integer array swap) -----\n";
    for (unsigned is = 0; is < swapsPerTxList.size(); is++) {
        int nWords = swapsPerTxList[is];
        for (unsigned it = 0; it < threadList.size(); it++) {
            int nThreads = threadList[it];
            PBenchmarkSPS bench(nThreads);
            std::cout << "\n----- threads=" << nThreads << "   runs=" << numRuns << "   length=" << testLength.count() << "s   arraySize=" << arraySize << "   swaps/tx=" << nWords << " -----\n";
#ifdef USE_CXPTM
            results[it][is] = bench.benchmarkSPSInteger<cx::CX,                  cx::persist>           (cName, testLength, nWords, numRuns);
#elif defined USE_CXPUC
            results[it][is] = bench.benchmarkSPSInteger<cxpuc::CX,               cxpuc::persist>        (cName, testLength, nWords, numRuns);
#elif defined USE_CXREDO
            results[it][is] = bench.benchmarkSPSInteger<cxredo::CXRedo,          cxredo::persist>       (cName, testLength, nWords, numRuns);
#elif defined USE_CXREDOTIMED
            results[it][is] = bench.benchmarkSPSInteger<cxredotimed::CXRedoTimed,cxredotimed::persist>  (cName, testLength, nWords, numRuns);
#elif defined USE_REDO_LOG_FC
            results[it][is] = bench.benchmarkSPSInteger<redologfc::RedoLog,      redologfc::persist>    (cName, testLength, nWords, numRuns);
#elif defined USE_ROMLR
            results[it][is] = bench.benchmarkSPSInteger<romuluslr::RomulusLR,    romuluslr::persist>    (cName, testLength, nWords, numRuns);
#elif defined USE_ROMLOG
            results[it][is] = bench.benchmarkSPSInteger<romuluslog::RomulusLog,  romuluslog::persist>   (cName, testLength, nWords, numRuns);
#elif defined USE_ROM_LOG_FC
            results[it][is] = bench.benchmarkSPSInteger<romlogfc::RomLog,        romlogfc::persist>     (cName, testLength, nWords, numRuns);
#elif defined USE_OFLF
            results[it][is] = bench.benchmarkSPSInteger<poflf::OneFileLF,        poflf::tmtype>         (cName, testLength, nWords, numRuns);
#elif defined USE_OFWF
            results[it][is] = bench.benchmarkSPSInteger<pofwf::OneFileWF,        pofwf::tmtype>         (cName, testLength, nWords, numRuns);
#elif defined USE_DUALZONE_2F_FC
            results[it][is] = bench.benchmarkSPSInteger<dualzone2ffc::DualZone,  dualzone2ffc::persist> (cName, testLength, nWords, numRuns);
#elif defined USE_DZ_V1
            results[it][is] = bench.benchmarkSPSInteger<dzv1::DualZone,          dzv1::persist>         (cName, testLength, nWords, numRuns);
#elif defined USE_DZ_V2
            results[it][is] = bench.benchmarkSPSInteger<dzv2::DualZone,          dzv2::persist>         (cName, testLength, nWords, numRuns);
#elif defined USE_DZ_V3
            results[it][is] = bench.benchmarkSPSInteger<dzv3::DualZone,          dzv3::persist>         (cName, testLength, nWords, numRuns);
#elif defined USE_DZ_V4
            results[it][is] = bench.benchmarkSPSInteger<dzv4::DualZone,          dzv4::persist>         (cName, testLength, nWords, numRuns);
#elif defined USE_DZ_TL2
            results[it][is] = bench.benchmarkSPSInteger<dztl2::DualZone,         dztl2::persist>        (cName, testLength, nWords, numRuns);
#elif defined USE_REDO_LOG_TL2
            results[it][is] = bench.benchmarkSPSInteger<redologtl2::RedoLog,     redologtl2::persist>   (cName, testLength, nWords, numRuns);
#else
            results[it][is] = bench.benchmarkSPSInteger<PTM_CLASS,PTM_TYPE>(cName, testLength, nWords, numRuns);
#endif
        }
        std::cout << "\n";
    }

    // Export tab-separated values to a file to be imported in gnuplot or excel
    ofstream dataFile;
    dataFile.open(dataFilename);
    dataFile << "Threads\t";
    // Printf class names for each column plus the corresponding thread
    for (unsigned is = 0; is < swapsPerTxList.size(); is++) {
        int nWords = swapsPerTxList[is];
        dataFile << cName << "-" << nWords <<"\t";
    }
    dataFile << "\n";
    for (unsigned it = 0; it < threadList.size(); it++) {
        dataFile << threadList[it] << "\t";
        for (unsigned is = 0; is < swapsPerTxList.size(); is++) {
            dataFile << results[it][is] << "\t";
        }
        dataFile << "\n";
    }
    dataFile.close();
    std::cout << "\nSuccessfuly saved results in " << dataFilename << "\n";



    return 0;
}
