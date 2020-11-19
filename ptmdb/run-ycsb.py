#!/usr/bin/env python
import os

# YCSB Workload A
#db_bench_option = " workloads/a"
#db_bench_option = " ~/pronto-v1.1/benchmark/workloads/1m/a"
#db_bench_option = " ~/pramalhe/pronto-v1.1/benchmark/workloads/1m/a"
#YCSB Workload B
#db_bench_option = " workloads/b"
#db_bench_option = " ~/pronto-v1.1/benchmark/workloads/1m/b"
#db_bench_option = " ~/pramalhe/pronto-v1.1/benchmark/workloads/1m/b"



#
# Databases
#
db_list = [
#    "bin/ycsb_trinvrtl2",
#    "bin/ycsb_trinvrfc",
    "bin/ycsb_redodb",
#    "bin/ycsb_pmemkv",
#    "~/rocksdb/ycsb_tool --db=/mnt/pmem0/rocksdb --sync"
    ]

# List of threads to run with
thread_list = [
    " 1",
    " 2",
    " 4",
    " 8",
    " 16",
    " 20",
    " 24",
    " 32",
    " 40"
    ]

print "\n+++ Running YCSB benchmarks +++\n"
for db in db_list:
    os.system("rm results-ycsb.txt")
    for db_thread_option in thread_list:
        os.system("make persistencyclean")
        os.system("sleep 2")
        print db + db_thread_option + db_bench_option
        os.system("echo   >> results-ycsb.txt") 
        os.system("echo" + db_thread_option + " >> results-ycsb.txt") 
        os.system(db + db_thread_option + db_bench_option + " >> results-ycsb.txt")
    
os.system("make persistencyclean")
