#!/usr/bin/env python
import os

# We only support workload A and workload B
workload_list = [ " a", " b" ]

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

print "\n+++ Generating YCSB workloads +++\n"
for workload in workload_list:
    for thread in thread_list:
        print "bin/ycsb_generator" + workload + thread
        os.system("bin/ycsb_generator" + workload + thread)

# Move all the workloads to workloads/1m/
os.system("mv workloads/a-* workloads/1m/")
os.system("mv workloads/b-* workloads/1m/")
    
