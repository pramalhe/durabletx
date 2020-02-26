/*
 * Copyright 2018-2020
 *   Andreia Correia <andreia.veiga@unine.ch>
 *   Pedro Ramalhete <pramalhe@gmail.com>
 *   Pascal Felber <pascal.felber@unine.ch>
 *
 * This work is published under the MIT license. See LICENSE.txt
 */
#ifndef _TRINITY_SEQ_MAIN_BACK_VOLATILE_REPLICA_FLAT_COMBINING_H_
#define _TRINITY_SEQ_MAIN_BACK_VOLATILE_REPLICA_FLAT_COMBINING_H_

#include <atomic>
#include <cassert>
#include <functional>
#include <cstring>
#include <thread>       // Needed by this_thread::yield()
#include <sys/mman.h>   // Needed if we use mmap()
#include <sys/types.h>  // Needed by open() and close()
#include <sys/stat.h>
#include <linux/mman.h> // Needed by MAP_SHARED_VALIDATE
#include <fcntl.h>
#include <unistd.h>     // Needed by close()
#include <type_traits>

/*
 * <h1> Trinity + Volatile Region + Flat-Combining </h1>
 *
 * In this version of Trinity we write first the 'seq' then the 'main' and then the 'back'.
 * This variant has a volatile replica of 'main', which means we can use ranges and it is much faster.
 * There is a volatile region (VR) and a PM region. Each consecutive 24 bytes (3x64bit words) in the VR region
 * maps to a cacheline in the PM region. A cacheline in the PM region is composed of:
 * main    - 24 bytes
 * back    - 24 bytes
 * seq     - 8 bytes
 * padding - 8 bytes
 */


// Macros needed for persistence
#ifdef PWB_IS_CLFLUSH
  /*
   * More info at http://elixir.free-electrons.com/linux/latest/source/arch/x86/include/asm/special_insns.h#L213
   * Intel programming manual at https://www.intel.com/content/dam/www/public/us/en/documents/manuals/64-ia-32-architectures-optimization-manual.pdf
   * Use these for Broadwell CPUs (cervino server)
   */
  #define PWB(addr)              __asm__ volatile("clflush (%0)" :: "r" (addr) : "memory")                      // Broadwell only works with this.
  #define PFENCE()               {}                                                                             // No ordering fences needed for CLFLUSH (section 7.4.6 of Intel manual)
  #define PSYNC()                {}                                                                             // For durability it's not obvious, but CLFLUSH seems to be enough, and PMDK uses the same approach
#elif PWB_IS_CLWB
  /* Use this for CPUs that support clwb, such as the SkyLake SP series (c5 compute intensive instances in AWS are an example of it) */
  #define PWB(addr)              __asm__ volatile(".byte 0x66; xsaveopt %0" : "+m" (*(volatile char *)(addr)))  // clwb() only for Ice Lake onwards
  #define PFENCE()               __asm__ volatile("sfence" : : : "memory")
  #define PSYNC()                __asm__ volatile("sfence" : : : "memory")
#elif PWB_IS_NOP
  /* pwbs are not needed for shared memory persistency (i.e. persistency across process failure) */
  #define PWB(addr)              {}
  #define PFENCE()               __asm__ volatile("sfence" : : : "memory")
  #define PSYNC()                __asm__ volatile("sfence" : : : "memory")
#elif PWB_IS_CLFLUSHOPT
  /* Use this for CPUs that support clflushopt, which is most recent x86 */
  #define PWB(addr)              __asm__ volatile(".byte 0x66; clflush %0" : "+m" (*(volatile char *)(addr)))    // clflushopt (Kaby Lake)
  #define PFENCE()               __asm__ volatile("sfence" : : : "memory")
  #define PSYNC()                __asm__ volatile("sfence" : : : "memory")
#else
#error "You must define what PWB is. Choose PWB_IS_CLFLUSHOPT if you don't know what your CPU is capable of"
#endif


namespace trinityvrfc {

//
// User configurable variables.
// Feel free to change these if you need larger transactions, or more threads.
//

// Start address of persistent memory region
#ifndef PM_REGION_BEGIN
#define PM_REGION_BEGIN 0x7fea00000000
#endif
// Size of the persistent memory region
#ifndef PM_REGION_SIZE
#define PM_REGION_SIZE 1024*1024*1024ULL // 1GB for now
#endif
// Name of persistent file mapping (back)
#ifndef PM_FILE_NAME
#define PM_FILE_NAME   "/dev/shm/trinityvrfc_shared"
#endif

// Maximum number of registered threads that can execute transactions
static const int REGISTRY_MAX_THREADS = 128;
// End address of mapped persistent memory
static uint8_t* PREGION_END = (uint8_t*)(PM_REGION_BEGIN+PM_REGION_SIZE);
// Maximum number of root pointers available for the user. MUST be a multiple of 8.
static const uint64_t MAX_ROOT_POINTERS = 64;
// Name of volatile file mapping (main)
static const char * VFILE_NAME = "/dev/shm/trinityvrfc_shared_main";
// Start address of volatile memory region (VR)
static uint8_t* VREGION_ADDR = (uint8_t*)0x7f0000000000;


// Returns the cache line of the address
#define ADDR2CL(_addr) (uint8_t*)((size_t)(_addr) & (~63ULL))
// Number of bytes from the begining of a PCL to the 'seq'
#define PCL_SEQ_OFFSET      (24+24)
// Number of bytes from the begining of a PCL to the 'back'
#define PCL_BACK_OFFSET      (24)

// Flush each cache line in a range
static inline void flushFromTo(void* from, void* to) noexcept {
    const uint64_t cache_line_size = 64;
    uint8_t* ptr = (uint8_t*)(((uint64_t)from) & (~(cache_line_size-1)));
    for (; ptr < (uint8_t*)to; ptr += cache_line_size) PWB(ptr);
}


//
// Thread Registry stuff
//
extern void thread_registry_deregister_thread(const int tid);

// An helper class to do the checkin and checkout of the thread registry
struct ThreadCheckInCheckOut {
    static const int NOT_ASSIGNED = -1;
    int tid { NOT_ASSIGNED };
    ~ThreadCheckInCheckOut() {
        if (tid == NOT_ASSIGNED) return;
        thread_registry_deregister_thread(tid);
    }
};

extern thread_local ThreadCheckInCheckOut tl_tcico;

// Forward declaration of global/singleton instance
class ThreadRegistry;
extern ThreadRegistry gThreadRegistry;

/*
 * <h1> Registry for threads </h1>
 *
 * This is singleton type class that allows assignement of a unique id to each thread.
 * The first time a thread calls ThreadRegistry::getTID() it will allocate a free slot in 'usedTID[]'.
 * This tid wil be saved in a thread-local variable of the type ThreadCheckInCheckOut which
 * upon destruction of the thread will call the destructor of ThreadCheckInCheckOut and free the
 * corresponding slot to be used by a later thread.
 */
class ThreadRegistry {
private:
    alignas(128) std::atomic<bool>      usedTID[REGISTRY_MAX_THREADS];   // Which TIDs are in use by threads
    alignas(128) std::atomic<int>       maxTid {-1};                     // Highest TID (+1) in use by threads

public:
    ThreadRegistry() {
        for (int it = 0; it < REGISTRY_MAX_THREADS; it++) {
            usedTID[it].store(false, std::memory_order_relaxed);
        }
    }

    // Progress condition: wait-free bounded (by the number of threads)
    int register_thread_new(void) {
        for (int tid = 0; tid < REGISTRY_MAX_THREADS; tid++) {
            if (usedTID[tid].load(std::memory_order_acquire)) continue;
            bool unused = false;
            if (!usedTID[tid].compare_exchange_strong(unused, true)) continue;
            // Increase the current maximum to cover our thread id
            int curMax = maxTid.load();
            while (curMax <= tid) {
                maxTid.compare_exchange_strong(curMax, tid+1);
                curMax = maxTid.load();
            }
            tl_tcico.tid = tid;
            return tid;
        }
        printf("ERROR: Too many threads, registry can only hold %d threads\n", REGISTRY_MAX_THREADS);
        assert(false);
    }

    // Progress condition: wait-free population oblivious
    inline void deregister_thread(const int tid) {
        usedTID[tid].store(false, std::memory_order_release);
    }

    // Progress condition: wait-free population oblivious
    static inline uint64_t getMaxThreads(void) {
        return gThreadRegistry.maxTid.load(std::memory_order_acquire);
    }

    // Progress condition: wait-free bounded (by the number of threads)
    static inline int getTID(void) {
        int tid = tl_tcico.tid;
        if (tid != ThreadCheckInCheckOut::NOT_ASSIGNED) return tid;
        return gThreadRegistry.register_thread_new();
    }
};


// Each cache line in the PM region is one of these
struct PMCacheLine {
    uint64_t main[3];
    uint64_t back[3];
    uint64_t seq;
    uint64_t pad;
} __attribute__((packed));



/*
 * EsLoco is an Extremely Simple memory aLOCatOr
 *
 * It is based on intrusive singly-linked lists (a free-list), one for each power of two size.
 * All blocks are powers of two, the smallest size enough to contain the desired user data plus the block header.
 * There is an array named 'freelists' where each entry is a pointer to the head of a stack for that respective block size.
 * Blocks are allocated in powers of 2 of words (64bit words).
 * Each block has an header with two words: the size of the node (in words), the pointer to the next node.
 * The minimum block size is 4 words, with 2 for the header and 2 for the user.
 * When there is no suitable block in the freelist, it will create a new block from the remaining pool.
 *
 * EsLoco was designed for usage in PTMs but it doesn't have to be used only for that.
 * Average number of stores for an allocation is 1.
 * Average number of stores for a de-allocation is 2.
 *
 * Memory layout:
 * ------------------------------------------------------------------------
 * | poolTop | freelists[0] ... freelists[61] | ... allocated objects ... |
 * ------------------------------------------------------------------------
 */
template <template <typename> class P>
class EsLoco {
private:
    struct block {
        P<block*>   next;   // Pointer to next block in free-list (when block is in free-list)
        P<uint64_t> size;   // Exponent of power of two of the size of this block in bytes.
    };

    const bool debugOn = false;

    // Volatile data
    uint8_t* poolAddr {nullptr};
    uint64_t poolSize {0};

    // Pointer to array of persistent heads of free-list
    block* freelists {nullptr};
    // Volatile pointer to persistent pointer to last unused address (the top of the pool)
    P<uint8_t*>* poolTop {nullptr};

    // Number of blocks in the freelists array.
    // Each entry corresponds to an exponent of the block size: 2^4, 2^5, 2^6... 2^40
    static const int kMaxBlockSize = 40; // 1 TB of memory should be enough

    // For powers of 2, returns the highest bit, otherwise, returns the next highest bit
    uint64_t highestBit(uint64_t val) {
        uint64_t b = 0;
        while ((val >> (b+1)) != 0) b++;
        if (val > (1ULL << b)) return b+1;
        return b;
    }

    // align at cache lines (128 bytes)
    uint8_t* aligned(uint8_t* addr) {
        return (uint8_t*)((size_t)addr & (~0x3FULL)) + 128;
    }

public:
    void init(void* addressOfMemoryPool, size_t sizeOfMemoryPool, bool clearPool=true) {
        // Align the base address of the memory pool
        poolAddr = aligned((uint8_t*)addressOfMemoryPool);
        poolSize = sizeOfMemoryPool + (uint8_t*)addressOfMemoryPool - poolAddr;
        // The first thing in the pool is a pointer to the top of the pool
        poolTop = (P<uint8_t*>*)poolAddr;
        // The second thing in the pool is the array of freelists
        freelists = (block*)(poolAddr + sizeof(*poolTop));
        if (clearPool) {
            for (int i = 0; i < kMaxBlockSize; i++) freelists[i].next.pstore(nullptr);
            // The size of the freelists array in bytes is sizeof(block)*kMaxBlockSize
            // Align to cache line boundary (DCAS needs 16 byte alignment)
            poolTop->pstore(aligned(poolAddr + sizeof(*poolTop) + sizeof(block)*kMaxBlockSize));
        }
        if (debugOn) printf("Starting EsLoco with poolAddr=%p and poolSize=%ld, up to %p\n", poolAddr, poolSize, poolAddr+poolSize);
    }

    // Resets the metadata of the allocator back to its defaults
    void reset() {
        std::memset(poolAddr, 0, sizeof(block)*kMaxBlockSize);
        poolTop->pstore(nullptr);
    }

    // Returns the number of bytes that may (or may not) have allocated objects, from the base address to the top address
    uint64_t getUsedSize() {
        return poolTop->pload() - poolAddr;
    }

    // Takes the desired size of the object in bytes.
    // Returns pointer to memory in pool, or nullptr.
    // Does on average 1 store to persistent memory when re-utilizing blocks.
    void* malloc(size_t size) {
        P<uint8_t*>* top = (P<uint8_t*>*)(((uint8_t*)poolTop));
        block* flists = (block*)(((uint8_t*)freelists));
        // Adjust size to nearest (highest) power of 2
        uint64_t bsize = highestBit(size + sizeof(block));
        if (debugOn) printf("malloc(%ld) requested,  block size exponent = %ld\n", size, bsize);
        block* myblock = nullptr;
        // Check if there is a block of that size in the corresponding freelist
        if (flists[bsize].next.pload() != nullptr) {
            if (debugOn) printf("Found available block in freelist\n");
            // Unlink block
            myblock = flists[bsize].next;
            flists[bsize].next = myblock->next;          // pstore()
        } else {
            if (debugOn) printf("Creating new block from top, currently at %p\n", top->pload());
            // Couldn't find a suitable block, get one from the top of the pool if there is one available
            if (top->pload() + (1<<bsize) > poolSize + poolAddr) return nullptr;
            myblock = (block*)top->pload();
            top->pstore(top->pload() + (1<<bsize));      // pstore()
            myblock->size = bsize;                       // pstore()
        }
        if (debugOn) printf("returning ptr = %p\n", (void*)((uint8_t*)myblock + sizeof(block)));
        // Return the block, minus the header
        return (void*)((uint8_t*)myblock + sizeof(block));
    }

    // Takes a pointer to an object and puts the block on the free-list.
    // Does on average 2 stores to persistent memory.
    void free(void* ptr) {
        if (ptr == nullptr) return;
        block* flists = (block*)(((uint8_t*)freelists));
        block* myblock = (block*)((uint8_t*)ptr - sizeof(block));
        if (debugOn) printf("free(%p)  block size exponent = %ld\n", ptr, myblock->size.pload());
        // Insert the block in the corresponding freelist
        myblock->next = flists[myblock->size].next;      // pstore()
        flists[myblock->size].next = myblock;            // pstore()
    }
};


// Needed by our benchmarks
struct tmbase {
};


// Pause to prevent excess processor bus usage
#if defined( __sparc )
#define Pause() __asm__ __volatile__ ( "rd %ccr,%g0" )
#elif defined( __i386 ) || defined( __x86_64 )
#define Pause() __asm__ __volatile__ ( "pause" : : : )
#else
#define Pause() std::this_thread::yield();
#endif


/**
 * <h1> C-RW-WP </h1>
 *
 * A C-RW-WP reader-writer lock with writer preference and using a
 * Ticket Lock as Cohort.
 * This is starvation-free for writers and for readers, but readers may be
 * starved by writers.
 * C-RW-WP paper:         http://dl.acm.org/citation.cfm?id=2442532
 *
 * This variant of C-RW-WP has two modes on the writersMutex so that readers can
 * enter (but not writers). This is specific to DualZone because it's ok to let
 * the readers enter the 'main' while we're replicating modifications on 'back'.
 *
 */
class CRWWPSpinLock {

private:
    class SpinLock {
        alignas(128) std::atomic<int> writers {0};
    public:
        inline bool isLocked() { return (writers.load() != 0); }
        inline void lock() {
            while (!tryLock()) Pause();
        }
        inline bool tryLock() {
            if (writers.load() != 0) return false;
            int tmp = 0;
            return writers.compare_exchange_strong(tmp,2);
        }
        inline void unlock() {
            writers.store(0, std::memory_order_release);
        }
    };

    class RIStaticPerThread {
    private:
        static const uint64_t NOT_READING = 0;
        static const uint64_t READING = 1;
        static const int CLPAD = 128/sizeof(uint64_t);
        alignas(128) std::atomic<uint64_t>* states;

    public:
        RIStaticPerThread() {
            states = new std::atomic<uint64_t>[REGISTRY_MAX_THREADS*CLPAD];
            for (int tid = 0; tid < REGISTRY_MAX_THREADS; tid++) {
                states[tid*CLPAD].store(NOT_READING, std::memory_order_relaxed);
            }
        }

        ~RIStaticPerThread() {
            delete[] states;
        }

        inline void arrive(const int tid) noexcept {
            states[tid*CLPAD].store(READING);
        }

        inline void depart(const int tid) noexcept {
            states[tid*CLPAD].store(NOT_READING, std::memory_order_release);
        }

        inline bool isEmpty() noexcept {
            const int maxTid = ThreadRegistry::getMaxThreads();
            for (int tid = 0; tid < maxTid; tid++) {
                if (states[tid*CLPAD].load() != NOT_READING) return false;
            }
            return true;
        }
    };

    SpinLock splock {};
    RIStaticPerThread ri {};

public:
    std::string className() { return "C-RW-WP-SpinLock"; }

    inline void exclusiveLock() {
        splock.lock();
        while (!ri.isEmpty()) Pause();
    }

    inline bool tryExclusiveLock() {
        return splock.tryLock();
    }

    inline void exclusiveUnlock() {
        splock.unlock();
    }

    inline void sharedLock(const int tid) {
        while (true) {
            ri.arrive(tid);
            if (!splock.isLocked()) break;
            ri.depart(tid);
            while (splock.isLocked()) Pause();
        }
    }

    inline void sharedUnlock(const int tid) {
        ri.depart(tid);
    }

    inline void waitForReaders(){
        while (!ri.isEmpty()) {} // spin waiting for readers
    }
};


// Counter of nested write transactions
extern thread_local int64_t tl_nested_write_trans;
// Counter of nested read-only transactions
extern thread_local int64_t tl_nested_read_trans;


// T can be anything: we support ranges in this implementation
template<typename T> struct persist {
    T vrmain;

    persist() { }

    persist(T initVal) { pstore(initVal); }

    // Casting operator
    operator T() { return pload(); }

    // Casting to const
    operator T() const { return pload(); }

    // Prefix increment operator: ++x
    void operator++ () { pstore(pload()+1); }
    // Prefix decrement operator: --x
    void operator-- () { pstore(pload()-1); }
    void operator++ (int) { pstore(pload()+1); }
    void operator-- (int) { pstore(pload()-1); }
    persist<T>& operator+= (const T& rhs) { pstore(pload() + rhs); return *this; }
    persist<T>& operator-= (const T& rhs) { pstore(pload() - rhs); return *this; }

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

    // Operator arrow ->
    T operator->() { return pload(); }

    // Copy constructor
    persist<T>(const persist<T>& other) { pstore(other.pload()); }

    // Assignment operator from an tmtype
    persist<T>& operator=(const persist<T>& other) {
        pstore(other.pload());
        return *this;
    }

    // Assignment operator from a value
    persist<T>& operator=(T value) {
        pstore(value);
        return *this;
    }

    // Operator &
    T* operator&() {
        return (T*)this;
    }

    // There is no load interposing in Trinity
    inline T pload() const { return (T)vrmain; }

    // Defined later because of compilation dependencies
    inline void pstore(T newVal);
};


// The persistent metadata is a 'header' that contains all the logs.
// It is located after back, in the persistent region.
// We hard-code the location of the pwset, so make sure it's the FIRST thing in PMetadata.
struct PMetadata {
    static const uint64_t   MAGIC_ID = 0x1337babb;
    void*                   root {nullptr}; // Immutable once assigned
    uint64_t                id {0};
    volatile uint64_t       p_seq {1};
    uint64_t                padding[8-3];   // Padding ensures that what comes after PMetadata is cache-line aligned
};

// Size of presistent memory region without metadata
static uint64_t PM_SIZE = (PM_REGION_SIZE-sizeof(PMetadata));
// Size of volatile memory region (VR)
static uint64_t VR_SIZE = (PM_SIZE)*24ULL/64;
// End address of volatile memory region (VR)
static uint8_t* VREGION_END = VREGION_ADDR+VR_SIZE;
// start of PM without metadata
static uint8_t* PM_REGION_START = ((uint8_t*)PM_REGION_BEGIN+sizeof(PMetadata));
// Get the address of the PMCacheLine in PM
#define VR_2_PCL(_addr)  ((((size_t)_addr - (size_t)VREGION_ADDR)/24)*64 + (size_t)PM_REGION_START)
// Convert an address of a volatile byte into a PM byte of 'main'.
#define VR_2_PM(_addr)   ((((size_t)_addr - (size_t)VREGION_ADDR)/24)*64 + ( ((size_t)_addr-(size_t)VREGION_ADDR)%24 ) + (size_t)PM_REGION_START)
// Convert from a generic PM address to a VR address
#define PM_2_VR(_addr)   ((((size_t)_addr - (size_t)PM_REGION_START)/64)*24 + ( ((size_t)_addr-(size_t)PM_REGION_START)%64 ) + (size_t)VREGION_ADDR)

// Address of Persistent Metadata (start of back).
// This relies on PMetadata being the first thing in 'back'.
static PMetadata* const pmd = (PMetadata*)PM_REGION_BEGIN;

// Helper function to save a range of modifications.
inline void storeRange(void* vraddr, uint64_t size, uint64_t p_seq) {
    PMCacheLine* pclBeg = (PMCacheLine*)VR_2_PCL(vraddr);
    PMCacheLine* pclEnd = (PMCacheLine*)VR_2_PCL(((uint8_t*)vraddr) + size-1);
    // If (T) is large, it may be stored across multiple PMCacheLines
    for (PMCacheLine* pcl = pclBeg; pcl <= pclEnd; pcl++) {
        if (pcl->seq == p_seq) continue;
        // Copy main to back for each pcl
        std::memcpy(&pcl->back[0], &pcl->main[0], 24);                 // Ordered store
        asm volatile ("" : : : "memory");
        pcl->seq = p_seq;                                              // Ordered store
        asm volatile ("" : : : "memory");
        // Copy VR to main (all 24 bytes)
        std::memcpy(&pcl->main[0], (void*)PM_2_VR(&pcl->main[0]), 24); // Ordered store
        // We know we're only going to touch each pcl one time, therefore, flush it as we go
        PWB(pcl);
    }
}

// Volatile log of modified ranges
struct AppendLog {
    // We pre-allocate a write-set with this many entries and if more are needed,
    // we grow them dynamically during the transaction and then delete them when
    // reset() is called on beginTx().
    static const int64_t MAX_ENTRIES = 16*1024;

    struct LogEntry {
        void*     vraddr;
        uint32_t  length;
    };

    int64_t       size {0};
    LogEntry      entries[MAX_ENTRIES];
    AppendLog*    next {nullptr};

    inline void reset() {
        if (next != nullptr) {
            next->reset();   // Recursive call on the linked list of logs
            delete next;
            next = nullptr;
        }
        size = 0;
    }

    // Adds VR ranges to the log
    inline void add(void* vraddr, uint32_t length) {
        if (size != MAX_ENTRIES) {
            entries[size].vraddr = vraddr;
            entries[size].length = length;
            size++;
        } else {
            // The current node is full, therefore, search a vacant node or create a new one
            if (next == nullptr) next = new AppendLog();
            next->add(vraddr, length); // recursive call to add()
        }
    }

    inline void persistAndFlush(uint64_t p_seq) {
        for (int64_t i = size-1; i >= 0; i--) {
            storeRange(entries[i].vraddr, entries[i].length, p_seq);
        }
        if (next != nullptr) next->persistAndFlush(p_seq);  // Recursive call to persistAndFlush()
    }
};


class Trinity;
extern Trinity gTrinity;


class Trinity {
private:
    // Padding on x86 should be on 2 cache lines
    static const int                       CLPAD = 128/sizeof(uintptr_t);
    static const uint64_t                  metadataSizeVR = sizeof(PMetadata)*64/24;
    bool                                   reuseRegion {false};                 // used by the constructor and initialization
    int                                    pfd {-1};
    int                                    vfd {-1};
    CRWWPSpinLock                          rwlock {};
    // Array of atomic pointers to functions (used by Flat Combining)
    std::atomic< std::function<void()>* >* fc;
    EsLoco<persist>                        esloco {};

public:
    struct tmbase : public trinityvrfc::tmbase { };
    AppendLog                              v_log {};

    Trinity() {
        assert(sizeof(PMCacheLine) == 64);
        assert(sizeof(PMetadata)%64 == 0);
        fc = new std::atomic< std::function<void()>* >[REGISTRY_MAX_THREADS*CLPAD];
        for (int i = 0; i < REGISTRY_MAX_THREADS; i++) {
            fc[i*CLPAD].store(nullptr, std::memory_order_relaxed);
        }
        mapPersistentRegion(PM_FILE_NAME, (uint8_t*)PM_REGION_BEGIN, PM_REGION_SIZE);
        // The size of the volatile region is 24/64 the size of the PM region
        mapVolatileRegion(VFILE_NAME, VREGION_ADDR, VR_SIZE);
    }

    ~Trinity() {
        delete[] fc;
    }

    static std::string className() { return "Trinity-VR-FC"; }

    void mapPersistentRegion(const char* filename, uint8_t* regionAddr, const uint64_t regionSize) {
        // Check that the header with the logs leaves at least half the memory available to the user
        if (sizeof(PMetadata) > regionSize/2) {
            printf("ERROR: the size of the logs in persistent memory is so large that it takes more than half the whole persistent memory\n");
            printf("Please reduce some of the settings in TrinityVRFC.hpp and try again\n");
            assert(false);
        }
        // Check if the file already exists or not
        struct stat buf;
        if (stat(filename, &buf) == 0) {
            // File exists
            pfd = open(filename, O_RDWR|O_CREAT, 0755);
            assert(pfd >= 0);
            reuseRegion = true;
        } else {
            // File doesn't exist
            pfd = open(filename, O_RDWR|O_CREAT, 0755);
            assert(pfd >= 0);
            if (lseek(pfd, regionSize-1, SEEK_SET) == -1) {
                perror("lseek() error");
            }
            if (write(pfd, "", 1) == -1) {
                perror("write() error");
            }
        }
        // Try one time to mmap() with DAX. If it fails due to MAP_FAILED, then retry without DAX.
        // We may fail because the address is not available. Retry at most 4 times, then give up.
        uint64_t dax_flag = MAP_SYNC;
        void* got_addr = NULL;
        for (int i = 0; i < 4; i++) {
            got_addr = mmap(regionAddr, regionSize, (PROT_READ | PROT_WRITE), MAP_SHARED_VALIDATE | dax_flag, pfd, 0);
            if (got_addr == regionAddr) {
                if (dax_flag == 0) printf("WARNING: running without DAX enabled\n");
                break;
            }
            if (got_addr == MAP_FAILED) {
                // Failed to mmap(). Let's try without DAX.
                dax_flag = 0;
            } else {
                // mmap() worked but in wrong address. Let's unmap() and try again.
                munmap(got_addr, regionSize);
                // Sleep for a second before retrying the mmap()
                usleep(1000);
            }
        }
        if (got_addr == MAP_FAILED || got_addr != regionAddr) {
            printf("got_addr = %p instead of %p\n", got_addr, regionAddr);
            perror("ERROR: mmap() is not working !!! ");
            assert(false);
        }
        // Check if the header is consistent and only then can we attempt to re-use, otherwise we clear everything that's there
        if (reuseRegion) reuseRegion = (pmd->id == PMetadata::MAGIC_ID);
    }

    // Maps the volatile memory region
    void mapVolatileRegion(const char* filename, uint8_t* regionAddr, const uint64_t regionSize) {
        // Check if the file already exists or not
        struct stat buf;
        if (stat(filename, &buf) == 0) {
            // File exists
            vfd = open(filename, O_RDWR|O_CREAT, 0755);  // One day add O_DIRECT
            assert(vfd >= 0);
        } else {
            // File doesn't exist
            vfd = open(filename, O_RDWR|O_CREAT, 0755);  // One day add O_DIRECT
            assert(vfd >= 0);
            if (lseek(vfd, regionSize-1, SEEK_SET) == -1) {
                perror("lseek() error");
            }
            if (write(vfd, "", 1) == -1) {
                perror("write() error");
            }
        }
        // mmap() volatile memory range 'main'
        uint8_t* got_addr = (uint8_t *)mmap(regionAddr, regionSize, (PROT_READ | PROT_WRITE), MAP_SHARED, vfd, 0);
        if (got_addr == MAP_FAILED || got_addr != regionAddr) {
            printf("got_addr = %p  %p  %p\n", got_addr, regionAddr, MAP_FAILED);
            perror("ERROR: mmap() is not working !!! ");
            assert(false);
        }
        // If the file has just been created or if the header is not consistent, clear everything.
        // Otherwise, re-use and recover to a consistent state.
        if (reuseRegion) {
            recover();
            // Copy all contents of PM's 'main's to VR, making them continuous
            for (uint64_t cl = 0; cl < PM_SIZE/64; cl++) {
                std::memcpy(VREGION_ADDR+(cl*24), (uint8_t*)PM_REGION_START+(cl*64), 24);
            }
            readTx([&] () {
                esloco.init(regionAddr, regionSize, false);
            });
        } else {
            new (pmd) PMetadata();
            pmd->p_seq = 1;
            PWB(&pmd->p_seq);
            // We reset the entire memory region because the 'seq' have to be zero.
            // This operation can be be super SLOOWWW on a large PM region.
            // Think of this memset() as "formatting' the PM.
            std::memset(PM_REGION_START, 0, PM_SIZE);
            // Reset VR
            std::memset(regionAddr, 0, regionSize);
            updateTx([&] () {
                // Initialize the allocator and allocate an array for the root pointers
                esloco.init(regionAddr, regionSize, true);
                pmd->root = esloco.malloc(sizeof(persist<void*>)*MAX_ROOT_POINTERS);
            });
            PWB(&pmd->root);
            PFENCE();
            pmd->id = PMetadata::MAGIC_ID;
            PWB(&pmd->id);
            PFENCE();
            // From this point on, a crash will trigger the "reuseRegion" code path
        }
    }

    // Start a (single-threaded) durable transaction
    inline void beginTx() {
        tl_nested_write_trans++;
        if (tl_nested_write_trans > 1) return;
    }

    // End a (single-threaded) durable transaction
    inline void endTx() {
        tl_nested_write_trans--;
        if (tl_nested_write_trans > 0) return;
        v_log.persistAndFlush(pmd->p_seq);
        PFENCE();                               // Modifications in persist<T> are done before incrementing p_seq
        pmd->p_seq = pmd->p_seq + 1;
        PWB(&pmd->p_seq);
        PSYNC();                                // Durable commit
        v_log.reset();
    }

    // Same as begin/end transaction, but with a lambda.
    // Calling abort_transaction() from within the lambda is not allowed.
    template<typename R, class F>
    R transaction(F&& func) {
        beginTx();
        R retval = func();
        endTx();
        return retval;
    }

    template<class F>
    static void transaction(F&& func) {
        gTrinity.beginTx();
        func();
        gTrinity.endTx();
    }

    // Scan the PM for any persist<> with a sequence equal to p_seq.
    // If there are any, revert them by copying back to main and resetting
    // the seq to zero.
    // Scan the PM again for any persist<> with a sequence equal to p_seq.
    // If there are any, copy the 'back' to the 'main'.
    void recover() {
        // The persists start after PMetadata
        PMCacheLine* pbeg = (PMCacheLine*)(PM_REGION_BEGIN + sizeof(PMetadata));
        PMCacheLine* pend = (PMCacheLine*)(PM_REGION_BEGIN + PM_REGION_SIZE);
        const uint64_t p_seq = pmd->p_seq;
        for (PMCacheLine* pcl = pbeg; pcl < pend; pcl++) {
            if (pcl->seq == p_seq) {
                // Copy back to main
                std::memcpy(&pcl->main[0], &pcl->back[0], 24); // ordered stores
                asm volatile("": : :"memory");
                pcl->seq = 0;                                  // ordered store
                PWB(pcl);
            }
        }
        PSYNC();
    }

    /*
     * Non static, thread-safe
     * Progress: Blocking (starvation-free)
     */
    template<class F> void ns_write_transaction(F&& mutativeFunc) {
        if (tl_nested_write_trans > 0) {
            mutativeFunc();
            return;
        }
        std::function<void()> myfunc = mutativeFunc;
        const int tid = ThreadRegistry::getTID();
        // Add our mutation to the array of flat combining
        fc[tid*CLPAD].store(&myfunc, std::memory_order_release);
        // Lock writersMutex
        while (true) {
            if (rwlock.tryExclusiveLock()) break;
            // Check if another thread executed my mutation
            if (fc[tid*CLPAD].load(std::memory_order_acquire) == nullptr) return;
            std::this_thread::yield();
        }
        bool somethingToDo = false;
        const int maxTid = ThreadRegistry::getMaxThreads();
        // Save a local copy of the flat combining array
        std::function<void()>* lfc[maxTid];
        for (int i = 0; i < maxTid; i++) {
            lfc[i] = fc[i*CLPAD].load(std::memory_order_acquire);
            if (lfc[i] != nullptr) somethingToDo = true;
        }
        // Check if there is at least one operation to apply
        if (!somethingToDo) {
            rwlock.exclusiveUnlock();
            return;
        }
        rwlock.waitForReaders();
        beginTx();
        // Apply all mutativeFunc
        for (int i = 0; i < maxTid; i++) {
            if (lfc[i] == nullptr) continue;
            (*lfc[i])();
        }
        // Durable commit happens here
        endTx();
        // Inform the other threads their transactions are committed/durable
        for (int i = 0; i < maxTid; i++) {
            if (lfc[i] == nullptr) continue;
            fc[i*CLPAD].store(nullptr, std::memory_order_release);
        }
        // Release the lock
        rwlock.exclusiveUnlock();
    }

    // Non-static thread-safe read-only transaction
    template<class F> void ns_read_transaction(F&& readFunc) {
        if (tl_nested_read_trans > 0) {
            readFunc();
            return;
        }
        int tid = ThreadRegistry::getTID();
        ++tl_nested_read_trans;
        rwlock.sharedLock(tid);
        readFunc();
        rwlock.sharedUnlock(tid);
        --tl_nested_read_trans;
    }

    // It's silly that these have to be static, but we need them for the (SPS) benchmarks due to templatization
    template<typename F> static void updateTx(F&& func) { gTrinity.ns_write_transaction(func); }
    template<typename F> static void readTx(F&& func) { gTrinity.ns_read_transaction(func); }
    // Sequential durable transactions
    template<typename F> static void updateTxSeq(F&& func) { gTrinity.beginTx(); func(); gTrinity.endTx(); }
    template<typename F> static void readTxSeq(F&& func) { func(); }

    // TODO: Remove these two once we make CX have void transactions
    template<typename R,class F> inline static R readTx(F&& func) {
        gTrinity.ns_read_transaction([&]() {func();});
        return R{};
    }
    template<typename R,class F> inline static R updateTx(F&& func) {
        gTrinity.ns_write_transaction([&]() {func();});
        return R{};
    }

    template <typename T, typename... Args> static T* tmNew(Args&&... args) {
        if (tl_nested_write_trans == 0) {
            printf("ERROR: Can not allocate outside a transaction\n");
            return nullptr;
        }
        T* ptr = (T*)gTrinity.esloco.malloc(sizeof(T));
        // If we get nullptr then we've ran out of PM space
        assert(ptr != nullptr);
        new (ptr) T(std::forward<Args>(args)...);
        return ptr;
    }

    template<typename T> static void tmDelete(T* obj) {
        if (obj == nullptr) return;
        if (tl_nested_write_trans == 0) {
            printf("ERROR: Can not de-allocate outside a transaction\n");
            return;
        }
        obj->~T(); // Execute destructor as part of the current transaction
        tmFree(obj);
    }

    static void* tmMalloc(size_t size) {
        if (tl_nested_write_trans == 0) {
            printf("ERROR: Can not allocate outside a transaction\n");
            return nullptr;
        }
        void* obj = gTrinity.esloco.malloc(size);
        return obj;
    }

    static void tmFree(void* obj) {
        if (obj == nullptr) return;
        if (tl_nested_write_trans == 0) {
            printf("ERROR: Can not de-allocate outside a transaction\n");
            return;
        }
        gTrinity.esloco.free(obj);
    }

    // TODO: change this to ptmMalloc()
    static void* pmalloc(size_t size) {
        if (tl_nested_write_trans == 0) {
            printf("ERROR: Can not allocate outside a transaction\n");
            return nullptr;
        }
        return gTrinity.esloco.malloc(size);
    }

    // TODO: change this to ptmFree()
    static void pfree(void* obj) {
        if (obj == nullptr) return;
        if (tl_nested_write_trans == 0) {
            printf("ERROR: Can not de-allocate outside a transaction\n");
            return;
        }
        gTrinity.esloco.free(obj);
    }

    static void* tmMemcpy(void* dest, const void* src, std::size_t count) {
        void* result = std::memcpy(dest, src, count);
        if (tl_nested_write_trans != 0) gTrinity.v_log.add(dest, count);
        return result;
    }

    static int tmMemcmp(const void* lhs, const void* rhs, std::size_t count) {
        // Loads are done in-place on the volatile replica (VR) therefore regular memcmp() works fine
        return std::memcmp(lhs, rhs, count);
    }

    static int tmStrcmp(const char* lhs, const char* rhs, std::size_t count) {
        // Loads are done in-place on the volatile replica (VR) therefore regular memcmp() works fine
        return std::strncmp(lhs, rhs, count);
    }

    static void* tmMemset(void* dest, int ch, std::size_t count) {
        void* result = std::memset(dest, ch, count);
        if (tl_nested_write_trans != 0) gTrinity.v_log.add(dest, count);
        return result;
    }

    static std::size_t tmStrlen(const char* str) {
        // Loads are done in-place on the volatile replica (VR) therefore regular strlen() works fine
        return std::strlen(str);
    }

    // Get a root pointer
    static inline void* get_object(int idx) {
        return ((persist<void*>*)pmd->root)[idx].pload();
    }

    // Set a root pointer
    static inline void put_object(int idx, void* obj) {
        ((persist<void*>*)pmd->root)[idx].pstore(obj);
    }
};


// Store interposing
template<typename T> inline void persist<T>::pstore(T newVal) {
    uint8_t* vraddr = (uint8_t*)&vrmain;
    // We don't log outside PM nor outside a transaction
    if (tl_nested_write_trans != 0 && vraddr >= VREGION_ADDR && vraddr < VREGION_END) {
        gTrinity.v_log.add(vraddr, sizeof(T));
    }
    vrmain = newVal;
}


//
// Wrapper methods to the global TM instance. The user should use these:
//
template<typename R, typename F> static R updateTx(F&& func) { return gTrinity.updateTx<R>(func); }
template<typename R, typename F> static R readTx(F&& func) { return gTrinity.readTx<R>(func); }
template<typename F> static void updateTx(F&& func) { gTrinity.updateTx(func); }
template<typename F> static void readTx(F&& func) { gTrinity.readTx(func); }
template<typename F> static void updateTxSeq(F&& func) { gTrinity.beginTx(); func(); gTrinity.endTx(); }
template<typename F> static void readTxSeq(F&& func) { func(); }
template<typename T, typename... Args> T* tmNew(Args&&... args) { return Trinity::tmNew<T>(std::forward<Args>(args)...); }
template<typename T> void tmDelete(T* obj) { Trinity::tmDelete<T>(obj); }
inline static void* get_object(int idx) { return Trinity::get_object(idx); }
inline static void put_object(int idx, void* obj) { Trinity::put_object(idx, obj); }
inline static void* tmMalloc(size_t size) { return Trinity::tmMalloc(size); }
inline static void tmFree(void* obj) { Trinity::tmFree(obj); }


#ifndef INCLUDED_FROM_MULTIPLE_CPP
//
// Place these in a .cpp if you include this header from multiple files (compilation units)
//
// Global/singleton to hold all the thread registry functionality
ThreadRegistry gThreadRegistry {};
// PTM singleton
Trinity gTrinity {};
// Counter of nested write transactions
thread_local int64_t tl_nested_write_trans {0};
// Counter of nested read-only transactions
thread_local int64_t tl_nested_read_trans {0};
// This is where every thread stores the tid it has been assigned when it calls getTID() for the first time.
// When the thread dies, the destructor of ThreadCheckInCheckOut will be called and de-register the thread.
thread_local ThreadCheckInCheckOut tl_tcico {};
// Helper function for thread de-registration
void thread_registry_deregister_thread(const int tid) {
    gThreadRegistry.deregister_thread(tid);
}
#endif

}
#endif /* _TRINITY_SEQ_MAIN_BACK_FLAT_COMBINING_H_ */
