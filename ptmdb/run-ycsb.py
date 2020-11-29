#!/usr/bin/env python
import os

# YCSB Workloads A and B
workload_list = [ " workloads/1m/a", " workloads/1m/b" ] 

#
# Databases
#
db_list = [
    ["bin/ycsb_trinvrtl2", "results_ycsb_trinvrtl2.txt"],
    ["bin/ycsb_trinvrfc",  "results_ycsb_trinvrfc.txt"],
    ["bin/ycsb_redodb",    "results_ycsb_redodb.txt"],
    ["bin/ycsb_pmemkv",    "results_ycsb_pmemkv.txt"],
    ["bin/ycsb_rocksdb",   "results_ycsb_rocksdb.txt"],
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
    os.system("rm " + db[1])
    os.system("date >> " + db[1])
    for workload in workload_list:
        for db_thread_option in thread_list:
            for mmap_attempts in range(10):
                os.system("make persistencyclean")
                os.system("sleep 2")
                os.system("""rm /mnt/pmem0/pmemkv_pool ; pmempool create -l "pmemkv" -s 2G obj /mnt/pmem0/pmemkv_pool""")
                header = db[0] + db_thread_option + workload
                print header 
                os.system("echo " + header + " >> " + db[1]) 
                ret = os.system(header + " >> " + db[1])
                if ret != 42:  # We retry the mmap() if it gave us the wrong address
                    break      
                
    os.system("date >> " + db[1])
    
os.system("make persistencyclean")
