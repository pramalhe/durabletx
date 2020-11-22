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
    "bin/db_bench_trinvrtl2",
    "bin/db_bench_trinvrfc",
    "otherdb/redodb/bin/db_bench_redoopt",
    "~/pmemkv-tools/pmemkv_bench --db_size_in_gb=32 --db=/mnt/pmem0/pmemkv", 
    "~/rocksdb/db_bench --db=/mnt/pmem0/rocksdb --sync",
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
for db in db_list:
    os.system("rm results.txt")
    for db_thread_option in thread_list:
        os.system("make persistencyclean")
        os.system("sleep 2")
        print db + db_bench_option + db_size_option + db_thread_option
        os.system("echo   >> results.txt") 
        os.system("echo" + db_thread_option + " >> results.txt") 
        os.system(db + db_bench_option + db_size_option + db_thread_option + " >> results.txt")
    print "\nRunning for fillseekseq"
    for db_thread_option in thread_list:
        os.system("make persistencyclean")
        os.system("sleep 2")
        print db + db_bench_fillseekseq + db_size_option + db_thread_option
        os.system("echo   >> results.txt") 
        os.system("echo" + db_thread_option + " >> results.txt") 
        os.system(db + db_bench_fillseekseq + db_size_option + db_thread_option + " >> results.txt")
    
os.system("make persistencyclean")
