#!/usr/bin/env python
import os

bin_folder = "bin/"
time_duration = "20"   # duration of each run in seconds

# Sequential benchmarks and ptms
seq_benchmark_list = [
    "pq-ll-seq-", 
    "pset-btree-1m-seq-",
    "pset-hash-1m-seq-",
    "pset-ll-10k-seq-",
    "pset-tree-1m-seq-",
    "psps-integer-seq-", 
    ]
    
seq_ptm_list = [
    "oflf",
    "pmdk",
    "romlogfc",
    "trinityfc",
    "trinityvrfc",
    "quadrafc",
    "quadravrfc",
    ]

print "\n+++ Running sequential microbenchmarks (pin threads to first core) +++\n"
for ptm in seq_ptm_list:
    for benchmark in seq_benchmark_list:
        os.system("make persistencyclean")
        os.system("sleep 2")
        print bin_folder+benchmark+ptm
        os.system("taskset 0x1 "+bin_folder+benchmark+ptm+" "+time_duration)

