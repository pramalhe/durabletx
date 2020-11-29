#!/usr/bin/env python
import os

bin_folder = "bin/"
time_duration = "20"   # duration of each run in seconds

# Concurrent benchmarks and ptms
conc_benchmark_list = [
    "pq-ll-",
    "pset-tree-1m-",
    "pset-btree-1m-", 
    "pset-ziptree-1m-",
    ]

conc_ptm_list = [
    "oflf", 
    "pmdk",
    "romlogfc",
    "quadrafc",
    "quadravrfc",
    "trinityfc",
    "trinityvrfc",
    "trinitytl2",
    "trinityvrtl2",
    ]


print "\n\n+++ Running concurrent microbenchmarks +++\n"
for ptm in conc_ptm_list:
    for benchmark in conc_benchmark_list:
        print bin_folder+benchmark+ptm
        os.system("make persistencyclean")
        for mmap_attempts in range(10):
            os.system("sleep 2")
            ret = os.system(bin_folder+benchmark+ptm+" "+time_duration)
            if ret != 42:  # We retry the mmap() if it gave us the wrong address
                break      



