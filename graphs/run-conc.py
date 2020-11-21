#!/usr/bin/env python
import os

bin_folder = "bin/"
time_duration = "20"   # duration of each run in seconds

# Concurrent benchmarks and ptms
conc_benchmark_list = [
    "pq-ll-",
    "pset-tree-1m-",
    "psps-integer-", 
    "pset-btree-1m-", 
    "pset-hashfixed-1m-", 
    "pset-ziptree-1m-",
#    "pset-ravl-1m-",
#    "pq-fat-",
#    "pstack-ll-",
#    "pstack-fat-",
#    "pset-hash-1m-", 
#    "pset-skiplist-1m-",
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
        os.system("sleep 2")
        os.system(bin_folder+benchmark+ptm+" "+time_duration)



