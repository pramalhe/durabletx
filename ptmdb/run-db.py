#!/usr/bin/env python
import os


db_size_option = " --num=1000000"
db_bench_option = " --benchmarks=fillrandom,overwrite,fillseq,readrandom,readwhilewriting"
db_bench_fillseekseq = " --benchmarks=fillseekseq --reads=1000"


#
# Databases
# If you run with pmemkv, make sure to create the folder /mnt/pmem0/pmemkv/
#
db_list = [
    ["bin/db_bench_trinvrtl2", "results_db_bench_trinvrtl2.txt"],
    ["bin/db_bench_trinvrfc",  "results_db_bench_trinvrfc.txt"],
    ["otherdb/redodb/bin/db_bench_redoopt", "results_db_bench_redoopt.txt"],
    ["~/pmemkv-tools/pmemkv_bench --db_size_in_gb=32 --db=/mnt/pmem0/pmemkv", "results_db_bench_pmemkv.txt"],
    ["~/rocksdb/db_bench --db=/mnt/pmem0/rocksdb --sync --compression_type=none", "results_db_bench_rocksdb.txt"],
]

# List of threads to run with
thread_list = [
    " --threads=1",
    " --threads=2",
    " --threads=4",
    " --threads=8",
    " --threads=10",
    " --threads=16",
    " --threads=20",
    " --threads=24",
    " --threads=32",
    " --threads=40"
    ]

print "\n+++ Running DB benchmarks +++\n"
os.system("mkdir /mnt/pmem0/pmemkv/")
for db in db_list:
    os.system("rm " + db[1])
    os.system("date >> " + db[1])
    for db_thread_option in thread_list:
        for mmap_attempts in range(10):
            os.system("make persistencyclean")
            os.system("sleep 2")
            header = db[0] + db_bench_option + db_size_option + db_thread_option
            print header
            os.system("echo >> " + db[1])
            os.system("echo " + header + " >> " + db[1])
            os.system("echo   >> " + db[1]) 
            ret = os.system(header + " >> " + db[1])
            if ret != 42:  # We retry the mmap() if it gave us the wrong address
                break      
    print "\nRunning for fillseekseq"
    for db_thread_option in thread_list:
        for mmap_attempts in range(10):
            os.system("make persistencyclean")
            os.system("sleep 2")
            header =db[0] + db_bench_fillseekseq + db_size_option + db_thread_option
            print header
            os.system("echo >> " + db[1])
            os.system("echo " + header + " >> " + db[1])
            os.system("echo   >> " + db[1]) 
            ret = os.system(header + " >> " + db[1])
            if ret != 42:  # We retry the mmap() if it gave us the wrong address
                break      
    os.system("date >> " + db[1])
    
os.system("make persistencyclean")
