#ifndef _PMDK_TM_PERSISTENCY_
#define _PMDK_TM_PERSISTENCY_
#include <type_traits>

#define PMDK_STM

#ifdef PMDK_STM
#include <shared_mutex>         // You can comment this out if you use instead our C-RW-WP reader-writer lock
#include <libpmemobj++/p.hpp>
#include <libpmemobj++/transaction.hpp>
#include <libpmemobj++/pool.hpp>
#include <libpmemobj++/allocator.hpp>
#endif

// This can also be changed in the Makefile
// Size of the persistent memory region
#ifndef PM_REGION_SIZE
#define PM_REGION_SIZE (2*1024*1024*1024ULL) // 2GB by default (to run on laptop)
#endif
// Name of persistent file mapping
#ifndef PM_FILE_NAME
#define PM_FILE_NAME   "/dev/shm/pmdk_shared_friedman"
#endif

namespace pmdk {

#ifdef PMDK_STM

using namespace pmem::obj;

// Needed by our benchmarks
struct tmbase {
};

auto gpop = pool_base::create(PM_FILE_NAME, "", (size_t)(PM_REGION_SIZE));

std::shared_timed_mutex grwlock {};

thread_local int tl_nested_write_trans {0};
thread_local int tl_nested_read_trans {0};

#endif

// Ugly hack just to make PMDK work with our root pointers
static void* g_objects[100];

/*
 * <h1> Wrapper for libpmemobj from pmem.io </h1>
 *
 * http://pmem.io/pmdk/cpp_obj/
 *
 */
class PMDKTM {

public:
    PMDKTM()  {
        for (int i = 0; i < 100; i++) g_objects[i] = nullptr;

    }

    ~PMDKTM() { }

    struct tmbase : public pmdk::tmbase { };

    static std::string className() { return "PMDK"; }


    static inline void* get_object(int idx) {
        return g_objects[idx];  // TODO: fix me
    }

    static inline void put_object(int idx, void* obj) {
        g_objects[idx] = obj;  // TODO: fix me
        //PWB(&per->objects[idx]);
    }


    inline void begin_transaction() {
    }

    inline void end_transaction() {
    }

    inline void recover_if_needed() {
    }

    inline void abort_transaction(void) {
    }


    template<class F> static void transaction(F&& func) {
#ifdef PMDK_STM
        transaction::run(gpop, func);
#endif
    }

    template<class F> static void updateTx(F&& func) {
#ifdef PMDK_STM
        if (tl_nested_write_trans > 0) {
            transaction::run(gpop, func);
            return;
        }
        ++tl_nested_write_trans;
        grwlock.lock();
        transaction::run(gpop, func);
        grwlock.unlock();
        --tl_nested_write_trans;
#endif
    }

    template<class F> static void readTx(F&& func) {
#ifdef PMDK_STM
        if (tl_nested_read_trans > 0) {
            func();
            return;
        }
        ++tl_nested_read_trans;
        grwlock.lock_shared();
        func();
        grwlock.unlock_shared();
        --tl_nested_read_trans;
#endif
    }


    /*
     * Allocator
     * Must be called from within a transaction
     */
    template <typename T, typename... Args>
    static T* tmNew(Args&&... args) {
        void *addr = nullptr;
#ifdef PMDK_STM
        auto oid = pmemobj_tx_alloc(sizeof(T), 0);
        addr = pmemobj_direct(oid);
#endif
        return new (addr) T(std::forward<Args>(args)...); // placement new
    }


    /*
     * De-allocator
     * Must be called from within a transaction
     */
    template<typename T>
    static void tmDelete(T* obj) {
#ifdef PMDK_STM
        if (obj == nullptr) return;
        obj->~T();
        pmemobj_tx_free(pmemobj_oid(obj));
#endif
    }

    /* Allocator for C methods */
    static void* pmalloc(size_t size) {
        void* ptr = nullptr;
#ifdef PMDK_STM
        auto oid = pmemobj_tx_alloc(size, 0);
        ptr = pmemobj_direct(oid);
#endif
        return ptr;
    }


    /* De-allocator for C methods (like memcached) */
    static void pfree(void* ptr) {
#ifdef PMDK_STM
        pmemobj_tx_free(pmemobj_oid(ptr));
#endif
    }

    // Doesn't actually do any checking. That functionality exists only for RomulusLog and RomulusLR
    static bool consistency_check(void) {
        return true;
    }


    // TODO: Remove these two once we make CX have void transactions
    template<typename R,class F>
    inline static R readTx(F&& func) {
        readTx( [&]() {func();} );
        return R{};
    }
    template<typename R,class F>
    inline static R updateTx(F&& func) {
        updateTx( [&]() {func();} );
        return R{};
    }

    // Sequential durable transactions
    template<class F> static void updateTxSeq(F&& func) {
#ifdef PMDK_STM
        if (tl_nested_write_trans > 0) {
            func();
            return;
        }
        ++tl_nested_write_trans;
        transaction::run(gpop, func);
        --tl_nested_write_trans;
#endif
    }
    template<typename F> static void readTxSeq(F&& func) { func(); }

};


/*
 * Definition of persist<> type
 */
template<typename T>
struct persist {
#ifdef PMDK_STM
    // Stores the actual value
    pmem::obj::p<T> val {};   // This is where the magic happens in libpmemobj
#else
    T val {};
#endif
    persist() { }

    persist(T initVal) {
        pstore(initVal);
    }

    // Casting operator
    operator T() {
        return pload();
    }

    // Casting to const
    operator T() const { return pload(); }

    // Prefix increment operator: ++x
    void operator++ () {
        pstore(pload()+1);
    }

    // Prefix decrement operator: --x
    void operator-- () {
        pstore(pload()-1);
    }

    void operator++ (int) {
        pstore(pload()+1);
    }

    void operator-- (int) {
        pstore(pload()-1);
    }

    // Equals operator
    template <typename Y, typename = typename std::enable_if<std::is_convertible<Y, T>::value>::type>
    bool operator == (const persist<Y> &rhs) { return pload() == rhs; }
    // Difference operator: first downcast to T and then compare
    template <typename Y, typename = typename std::enable_if<std::is_convertible<Y, T>::value>::type>
    bool operator != (const persist<Y> &rhs) { return pload() != rhs; }
    // Relational operators
    template <typename Y, typename = typename std::enable_if<std::is_convertible<Y, T>::value>::type>
    bool operator < (const persist<Y> &rhs) { return pload() < rhs; }
    template <typename Y, typename = typename std::enable_if<std::is_convertible<Y, T>::value>::type>
    bool operator > (const persist<Y> &rhs) { return pload() > rhs; }
    template <typename Y, typename = typename std::enable_if<std::is_convertible<Y, T>::value>::type>
    bool operator <= (const persist<Y> &rhs) { return pload() <= rhs; }
    template <typename Y, typename = typename std::enable_if<std::is_convertible<Y, T>::value>::type>
    bool operator >= (const persist<Y> &rhs) { return pload() >= rhs; }

    T operator % (const T& rhs) {
        return pload() % rhs;
    }

    // Operator arrow ->
    T operator->() {
        return pload();
    }

    // Operator &
    T* operator&() {
#ifdef PMDK_STM
        return (T*)&val.get_ro();  // tsc, tsc: bad way to take away constness, but p<> is inflexible
#else
        return &val;
#endif
    }

    // Copy constructor
    persist<T>(const persist<T>& other) {
        pstore(other.pload());
    }

    // Assignment operator from an atomic_mwc
    persist<T>& operator=(const persist<T>& other) {
        pstore(other.pload());
        return *this;
    }

    // Assignment operator from a value
    persist<T>& operator=(T value) {
        pstore(value);
        return *this;
    }

    persist<T>& operator&=(T value) {
        pstore(pload() & value);
        return *this;
    }

    persist<T>& operator|=(T value) {
        pstore(pload() | value);
        return *this;
    }
    persist<T>& operator+=(T value) {
        pstore(pload() + value);
        return *this;
    }
    persist<T>& operator-=(T value) {
        pstore(pload() - value);
        return *this;
    }

    inline void pstore(T newVal) {
        val = newVal;
    }

    inline T pload() const {
        return val;
    }
};




} // end of pmdk namespace

#undef PMDK_STM

#endif   // _PMDK_TM_PERSISTENCY_
