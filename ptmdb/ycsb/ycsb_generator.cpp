#include <stdint.h>
#include <random>
#include <chrono>
#include <string>
#include <atomic>
#include <bitset>
#include <mutex>
#include <exception>
#include <cstdio>
#include <cstring>
#include <vector>
#include <algorithm>
#include <fstream>
#include <iostream>
#include "zipf.h"
#include "generator.h"
#include "workload.h"





void Workload::GenerateKeyFromInt(uint64_t v, char* key) {
    char* start = key;
    char* pos = start;

    int bytes_to_fill = std::min(wp_.key_size_ - static_cast<int>(pos - start), (uint32_t)8);
    //printf("bytes_to_fill=%d %ld\n", bytes_to_fill, workload_properties_.key_size_);
    for (int i = 0; i < bytes_to_fill; ++i) {
        pos[i] = (v >> ((bytes_to_fill - i - 1) << 3)) & 0xFF;
    }

    pos += bytes_to_fill;
    if (wp_.key_size_ > pos - start) {
        std::memset(pos, '0', wp_.key_size_ - (pos - start));
    }
}


void Workload::init(WorkloadProperties p) {
  wp_ = p;

  operation_chooser_ = new DiscreteGenerator((uint64_t)time(NULL));
  transaction_insert_keysequence_ = new AcknowledgedCounterGenerator(p.insert_start_);
  if (p.request_distribution_ == Distribution::ZIPFIAN) {
    key_chooser_ = new ScrambledZipfianGenerator((uint64_t)time(NULL), transaction_insert_keysequence_, p.insert_start_ - 1, p.lb_);
  } else if (p.request_distribution_ == Distribution::UNIFORM) {
    key_chooser_ = new UniformGenerator((uint64_t)time(NULL), transaction_insert_keysequence_, p.insert_start_ - 1, p.lb_);
  } else if (p.request_distribution_ == Distribution::LATEST) {
    key_chooser_ = new SkewedLatestGenerator((uint64_t)time(NULL), transaction_insert_keysequence_, p.insert_start_ - 1, p.lb_);
  }
  if (p.scan_length_distribution_ == Distribution::ZIPFIAN) {
    scan_length_chooser_ = new ZipfianGenerator((uint64_t)time(NULL), NULL, p.max_scan_len_, p.min_scan_len_);
  } else if (p.scan_length_distribution_ == Distribution::UNIFORM) {
    scan_length_chooser_ = new UniformGenerator((uint64_t)time(NULL), NULL, p.max_scan_len_, p.min_scan_len_);
  }

  if (p.insert_proportion_ > 0) {
    operation_chooser_->add_value(DBOperation::INSERT, p.insert_proportion_);
  }
  if (p.read_proportion_ > 0) {
    operation_chooser_->add_value(DBOperation::READ, p.read_proportion_);
  }
  if (p.scan_proportion_ > 0) {
    operation_chooser_->add_value(DBOperation::SCAN, p.scan_proportion_);
  }
  if (p.update_proportion_ > 0) {
    operation_chooser_->add_value(DBOperation::UPDATE, p.update_proportion_);
  }
  if (p.readmodifywrite_proportion_ > 0) {
    operation_chooser_->add_value(DBOperation::READMODIFYWRITE, p.readmodifywrite_proportion_);
  }
}


//
// This needs to do two things:
// 1. create 'load' files with the keys (can be in sequence)
// 2. create 'run' files that take from a zipfian distribution
//
int main(int argc, char* argv[]) {
    WorkloadProperties workloada {};
    workloada.record_count       = 1*1000*1000;   // Number of records in the database
    workloada.operation_count    = 10*1000*1000;  // Number of operations to execute in the database
    workloada.update_proportion_ = 0.5;
    workloada.read_proportion_   = 0.5;
    workloada.key_size_          = 20;
    workloada.value_size_        = 100;
    workloada.insert_start_      = 1*1000*1000;
    workloada.load_file          = "a-load-";
    workloada.run_file           = "a-run-";

    WorkloadProperties workloadb {};
    workloadb.record_count       = 1*1000*1000;   // Number of records in the database
    workloadb.operation_count    = 10*1000*1000;  // Number of operations to execute in the database
    workloadb.update_proportion_ = 0.05;
    workloadb.read_proportion_   = 0.95;
    workloadb.key_size_          = 20;
    workloadb.value_size_        = 100;
    workloadb.insert_start_      = 1*1000*1000;
    workloadb.load_file          = "b-load-";
    workloadb.run_file           = "b-run-";

    char key[workloada.key_size_+1];
    int threadcount = 1;
    Workload w {};

    if (argc >= 3) {
	if (strcmp(argv[1],"a") == 0) w.init(workloada); // YCSB-A
	else if (strcmp(argv[1],"b") == 0) w.init(workloadb); // YCSB-B
	else printf("ERROR: unknown workload [%s]\n", argv[1]);
        threadcount = atoi(argv[2]);
    } else {
	printf("Usage is ./ycsb_generator [a|b] <num-threads>\n");
	w.init(workloada);
    }

    // Generate the 'load' files
    for (int it = 0; it < threadcount; it++) {
        std::ofstream loadFile;
        std::string filename = "workloads/" + w.wp_.load_file + std::to_string(threadcount) + "." + std::to_string(it);
        std::cout << "Generating file " << filename << "\n";
        loadFile.open(filename);
        for (uint64_t i = 0; i < w.wp_.record_count/threadcount; i++) {
            loadFile << "Add " << w.wp_.insert_start_+i << "\n";
        }
        loadFile.close();
    }

    // Generate the 'run' files
    for (int it = 0; it < threadcount; it++) {
        std::ofstream runFile;
        std::string filename = "workloads/" + w.wp_.run_file + std::to_string(threadcount) + "." + std::to_string(it);
        std::cout << "Generating file " << filename << "\n";
        runFile.open(filename);
        for (uint64_t i = 0; i < w.wp_.operation_count/threadcount; i++) {
            runFile << DBOperationString[w.operation_chooser_->next_val()] << " " << (w.key_chooser_->next_val()+w.wp_.insert_start_) << "\n";
        }
        runFile.close();
    }

    return 0;
}
