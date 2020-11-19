#pragma once
#include <string>
#include <time.h>


enum class Distribution: int32_t {
  ZIPFIAN,
  UNIFORM,
  LATEST
};


//
// YCSB workload properties
// Taken from here: https://github.com/brianfrankcooper/YCSB/wiki/Core-Properties
//
struct WorkloadProperties {
    uint32_t record_count = 0;
    uint32_t operation_count = 0;
    uint32_t key_size_ = 0;
    uint32_t value_size_ = 0;
    uint64_t lb_ = 0;
    uint64_t insert_start_ = 0;
    uint64_t min_scan_len_ = 0;
    uint64_t max_scan_len_ = 0;
    double insert_proportion_ = 0.0;
    double read_proportion_ = 0.0;
    double scan_proportion_ = 0.0;
    double update_proportion_ = 0.0;
    double readmodifywrite_proportion_ = 0.0;
    Distribution request_distribution_ = Distribution::ZIPFIAN;
    Distribution scan_length_distribution_ = Distribution::UNIFORM;
    std::string load_file {};
    std::string run_file {};
};

class Workload {
 public:
    WorkloadProperties wp_; // Workload Properties
    DiscreteGenerator *operation_chooser_;
    Generator *key_chooser_;
    AcknowledgedCounterGenerator *transaction_insert_keysequence_;
    Generator *scan_length_chooser_;

  Workload() {}

  ~Workload() {
    delete operation_chooser_;
    delete key_chooser_;
    delete transaction_insert_keysequence_;
    delete scan_length_chooser_;
  }

  void GenerateKeyFromInt(uint64_t v, char* key);
  void init(WorkloadProperties p);
};

