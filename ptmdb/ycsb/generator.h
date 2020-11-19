#pragma once
#include <random>
#include <atomic>
#include <mutex>
#include <cstdint>

#define FNV_OFFSET_BASIS_64 0xCBF29CE484222325L
#define FNV_PRIME_64 1099511628211L


uint64_t fnvhash64(uint64_t val) {
    //from http://en.wikipedia.org/wiki/Fowler_Noll_Vo_hash
    uint64_t hashval = FNV_OFFSET_BASIS_64;
    for (int i = 0; i < 8; i++) {
        uint64_t octet = val & 0x00ff;
        val = val >> 8;
        hashval = hashval ^ octet;
        hashval = hashval * FNV_PRIME_64;
    }
    return hashval;
}

enum class DBOperation: uint64_t {
    INSERT,
    READ,
    SCAN,
    UPDATE,
    READMODIFYWRITE
};

const char* DBOperationString[] = {
    "Add",
    "Read",
    "Scan",
    "Update",
    "ReadModifyWrite"
};


class Generator {
protected:
    std::mt19937_64 generator_;
public:
    Generator(uint64_t s): generator_(s) {}

    virtual ~Generator() {}

    virtual uint64_t next_val() = 0;
};

class CounterGenerator: public Generator {
private:
    std::atomic<uint64_t> counter_;

public:
    explicit CounterGenerator(uint64_t count_start)
    : Generator(0),
      counter_(count_start) {}

    virtual uint64_t next_val() {
        return counter_.fetch_add(1);
    }
};

class AcknowledgedCounterGenerator: public CounterGenerator {
private:
    static const uint64_t WINDOW_SIZE = (1 << 20);
    static const uint64_t WINDOW_MASK = (1 << 20) - 1;

    bool slide_window_[WINDOW_SIZE] = {false};
    std::mutex window_mutex_;
    std::atomic<uint64_t> limit_;

public:
    explicit AcknowledgedCounterGenerator(uint64_t count_start)
    : CounterGenerator(count_start),
      limit_(count_start - 1) {}

    uint64_t last_val() {
        /*if (limit_.load() % 100000 == 0) {
      fprintf(stderr, "DEBUG: %lu\n", limit_.load());
    }*/
        return limit_.load();
    }

    void acknowledge(uint64_t value) {
        //fprintf(stderr, "DEBUG %lu\n", value);
        uint64_t current_slot = (uint64_t)(value & WINDOW_MASK);
        if (slide_window_[current_slot]) {
            abort();
            throw new std::runtime_error("Too many unacknowledged insertion keys.");
        }

        slide_window_[current_slot] = true;

        if (window_mutex_.try_lock()) {
            // move a contiguous sequence from the window
            // over to the "limit" variable
            try {
                // Only loop through the entire window at most once.
                uint64_t before_first_slot = (limit_.load() & WINDOW_MASK);
                uint64_t index;
                for (index = limit_.load() + 1; index != before_first_slot; ++index) {
                    uint64_t slot = (uint64_t)(index & WINDOW_MASK);
                    if (!slide_window_[slot]) {
                        break;
                    }

                    slide_window_[slot] = false;
                }

                limit_.store(index - 1);
            } catch (...) {

            }
            window_mutex_.unlock();
        }
    }
};

class ZipfianGenerator: public Generator {
protected:
    AcknowledgedCounterGenerator *trx_insert_keyseq_;
    uint64_t lb_;
    uint64_t ub_;
    uint64_t total_range_;
    double zipf_factor_;

public:
    explicit ZipfianGenerator(uint64_t s, AcknowledgedCounterGenerator *keyseq, uint64_t u, uint64_t l=0, double factor=1.0)
    : Generator(s),
      trx_insert_keyseq_(keyseq),
      lb_(l),
      ub_(u),
      total_range_(u - l),
      zipf_factor_(factor) {}

    virtual uint64_t next_val() {
        if (trx_insert_keyseq_) {
            ub_ = trx_insert_keyseq_->last_val();
            total_range_ = ub_ - lb_;
        }
        return lb_ + next(total_range_);
    }

    uint64_t next(uint64_t range) {
        return zipf_distribution<uint64_t, double>(range, zipf_factor_)(generator_);
    }
};

class ScrambledZipfianGenerator: public ZipfianGenerator {
public:
    explicit ScrambledZipfianGenerator(uint64_t s, AcknowledgedCounterGenerator *keyseq, uint64_t u, uint64_t l=0, double factor=1.0)
    : ZipfianGenerator(s, keyseq, u, l, factor) {}

    virtual uint64_t next_val() {
        uint64_t ret = ZipfianGenerator::next_val();
        return lb_ + fnvhash64(ret) % total_range_;
    }
};

class UniformGenerator: public Generator {
private:
    AcknowledgedCounterGenerator *trx_insert_keyseq_;
    uint64_t lb_;
    uint64_t ub_;

public:
    explicit UniformGenerator(uint64_t s, AcknowledgedCounterGenerator *keyseq, uint64_t u, uint64_t l=0)
    : Generator(s),
      trx_insert_keyseq_(keyseq),
      lb_(l),
      ub_(u) {};

    virtual uint64_t next_val() {
        if (trx_insert_keyseq_) {
            ub_ = trx_insert_keyseq_->last_val();
        }
        return lb_ + std::uniform_int_distribution<uint64_t>(lb_, ub_)(generator_);
    }
};

class DiscreteGenerator: public Generator {
private:
    std::vector<std::pair<DBOperation, double>> values_;

public:
    explicit DiscreteGenerator(uint64_t s): Generator(s) {}

    virtual uint64_t next_val() {
        double val = std::uniform_real_distribution<double>(0.0, 1.0)(generator_);

        for (auto p : values_) {
            double pw = p.second;
            if (val < pw) {
                return static_cast<uint64_t>(p.first);
            }

            val -= pw;
        }

        throw new std::runtime_error("Should not reach here.");
    }

    void add_value(DBOperation value, double weight) {
        values_.push_back(std::make_pair(value, weight));
    }
};

class SkewedLatestGenerator: public Generator {
private:
    AcknowledgedCounterGenerator *trx_insert_keyseq_;
    ZipfianGenerator gen_;

public:
    SkewedLatestGenerator(uint64_t s, AcknowledgedCounterGenerator *keyseq, uint64_t u, uint64_t l=0, double factor=1.0)
: Generator(s),
  trx_insert_keyseq_(keyseq),
  gen_(s, keyseq, u, l, factor) {}

    uint64_t next_val() {
        uint64_t max = trx_insert_keyseq_->last_val();
        return max - gen_.next(max);
    }
};

