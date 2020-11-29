/*
 * Copyright 2018-2020
 *   Andreia Correia <andreia.veiga@unine.ch>
 *   Pedro Ramalhete <pramalhe@gmail.com>
 *   Pascal Felber <pascal.felber@unine.ch>
 *
 * This work is published under the MIT license. See LICENSE.txt
 */
#ifndef _TRINITY_TL2_PERSISTENT_TRANSACTIONAL_MEMORY_H_
#define _TRINITY_TL2_PERSISTENT_TRANSACTIONAL_MEMORY_H_

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
 * <h1> Trinity + Persistent TL2 </h1>
 * See durable transactions paper
 *
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


namespace trinitytl2 {

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
#define PM_FILE_NAME   "/dev/shm/trinitytl2_shared"
#endif

// Maximum number of registered threads that can execute transactions
static const int REGISTRY_MAX_THREADS = 128;
// Maximum number of stores in the WriteSet per transaction
static const uint64_t TX_MAX_STORES = 40*1024;
// Maximum number of loads in one transaction
static const uint64_t TX_MAX_LOADS = 1024*1024;

// End address of mapped persistent memory
static uint8_t* PM_REGION_END = ((uint8_t*)PM_REGION_BEGIN+PM_REGION_SIZE);
// Maximum number of root pointers available for the user
static const uint64_t MAX_ROOT_POINTERS = 64;

// TL2 constants
static const uint64_t LOCKED  = 0x8000000000000000ULL;
static const int TX_IS_NONE   = 0;
static const int TX_IS_READ   = 1;
static const int TX_IS_UPDATE = 2;

// Returns the cache line of the address (this is for x86 only)
#define ADDR2CL(_addr) (uint8_t*)((size_t)(_addr) & (~63ULL))



// A 'Locked-Sequence' is a uint64_t which has a sequence (56 bits) a thread-id (7 bits) and a lock/unlock state (1 bit)
typedef uint64_t lseq_t;
// Function that returns the tid of a p.seq. The lock+tid is the 8 highest bits of p.seq.
inline static uint64_t lseq2tid(lseq_t lseq) { return (lseq >> (64-8)) & 0x7F; }
// Returns the sequence in a lseq
inline static uint64_t lseq2seq(lseq_t lseq) { return (lseq & 0xFFFFFFFFFFFFFFULL); }
// Creates a new locked-sequence
inline static lseq_t composeLseq(uint64_t lock, uint64_t tid, uint64_t seq) { return (lock | (tid << (64-8)) | seq); }
// Helper functions
inline static bool isLocked(uint64_t lseq) { return (lseq & LOCKED); }
inline static bool isUnlocked(uint64_t lseq) { return !(lseq & LOCKED); }
// Helper method: sl is unlocked and lower than the clock, or it's locked by me
static inline bool isUnlockedOrLockedByMe(uint64_t rClock, uint64_t tid, uint64_t sl) {
    return ( (isUnlocked(sl) && (lseq2seq(sl) <= rClock)) || (isLocked(sl) && lseq2tid(sl) == tid) );
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


// T is typically a pointer to a node, but it can be integers or other stuff, as long as it fits in 64 bits
template<typename T> struct persist {
    // We mark these as 'volatile' to ensure that the stores are ordered
    alignas(32) volatile uint64_t main;
    volatile uint64_t             back;
    std::atomic<lseq_t>           lseq;  // Lock plus sequence (for TL2 and Trinity)
    uint64_t                      pad;

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

    // Assignment operator from a persist<T>
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

    // Methods that are defined later because they have compilation dependencies
    inline T pload() const;
    inline void pstore(T newVal);
};


/*
 * EsLocoTu? is an Extremely Simple memory aLOCatOr Two
 *
 * It is based on intrusive singly-linked stacks (a free-list), one for each power of two size.
 * All blocks are powers of two, the smallest size enough to contain the desired user data plus the block header.
 * There is an array named 'freelists' where each entry is a pointer to the head of a stack for that respective block size.
 * Blocks are allocated in powers of 2 of words (64bit words).
 * Each block has an header with two words: the size of the node (in words), the pointer to the next node.
 * The minimum block size is 4 words, with 2 for the header and 2 for the user.
 * When there is no suitable block in the freelist, it will create a new block from the remaining pool.
 *
 * EsLoco was designed for usage in PTMs but it doesn't have to be used only for that.
 * EsLocoTu scales while EsLoco does not.
 * Average number of stores for an allocation is 1.
 * Average number of stores for a de-allocation is 2.
 *
 * Memory layout:
 * ---------------------------------------------------------------------------------------------------
 * | poolTop | gfreelists[0..50] | threadflists[0..REGISTRY_MAX_THREADS] | ... allocated objects ... |
 * ---------------------------------------------------------------------------------------------------
 */
template <template <typename> class P>
class EsLoco2 {
private:
    const bool debugOn = false;
    // Number of blocks in the freelists array.
    // Each entry corresponds to an exponent of the block size: 2^4, 2^5, 2^6... 2^40
    static const int kMaxBlockSize = 50; // 1024 TB of memory should be enough
    // Size after which we trigger a cleanup of the thread-local list/pool
    static const uint64_t kMaxListSize = 64;
    // Size of a thread-local slab (in bytes). When it runs out, we take another slab from poolTop.
    // Larger allocations go directly on poolTop which means doing a lot of large allocations from
    // multiple threads will result in contention.
    static const uint64_t kSlabSize = 4*1024*1024;

    struct block {
        P<block*>   next;   // Pointer to next block in free-list (when block is in free-list)
        P<uint64_t> size2;   // Exponent of power of two of the size of this block in bytes.
    };

    // A persistent free-list of blocks
    struct FList {
        P<uint64_t> count;   // Number of blocks in the stack
        P<block*>   head;    // Head of the stack (where we push and pop)
        P<block*>   tail;    // Tail of the stack (used for faster transfers to/from gflist)

        // Push a block into this free list, incrementing the volatile
        // counter and adjusting the tail if needed. Does two pstores min.
        void pushBlock(block* myblock) {
            block* lhead = head;
            if (lhead == nullptr) {
                assert(count == 0);
                tail = myblock;                    // pstore()
            }
            myblock->next = lhead;                 // pstore()
            head = myblock;                        // pstore()
            count++;
        }

        // Pops a block from a given (persistent) free list, decrementing the
        // volatile counter and setting the tail to nullptr if list becomes empty.
        // Does 2 pstores.
        block* popBlock(void) {
            assert(count != 0);
            assert(tail != nullptr);
            count--;
            block* myblock = head;
            head = myblock->next;                // pstore()
            if (head == nullptr) tail = nullptr; // pstore()
            return myblock;
        }

        bool isFull(void) { return (count >= kMaxListSize); }
        bool isEmpty() { return (count == 0); }

        // Transfer this list to another free-list (oflist).
        // We use this method to transfer from the thread-local list to the global list.
        void transfer(FList* oflist) {
            assert(count != 0);
            assert(tail != nullptr);
            if (oflist->tail == nullptr) {
                assert (oflist->head == nullptr);
                oflist->tail = tail;                           // pstore()
            }
            tail->next = oflist->head;                         // pstore()
            oflist->head = head;                               // pstore()
            oflist->count += count;                            // pstore()
            count = 0;                                         // pstore()
            tail = nullptr;                                    // pstore()
            head = nullptr;                                    // pstore()
        }
    };

    // To avoid false sharing and lock conflicts, we put each thread-local data in one of these.
    // This is persistent (in PM).
    struct TLData {
        uint64_t     padding[8];
        // Thread-local array of persistent heads of free-list
        FList        tflists[kMaxBlockSize];
        // Thread-local slab pointer
        P<uint8_t*>  slabtop;
        // Thread-loal slab size
        P<uint64_t>  slabsize;
        TLData() {
            for (int i = 0; i < kMaxBlockSize; i++) {
                tflists[i].count = 0;             // pstore()
                tflists[i].head = nullptr;        // pstore()
                tflists[i].tail = nullptr;        // pstore()
            }
            slabtop = nullptr;                    // pstore()
            slabsize = 0;                         // pstore()
        }
    };

    // Complete metadata. This is PM
    struct ELMetadata {
        P<uint8_t*> top;
        FList       gflists[kMaxBlockSize];
        TLData      tldata[REGISTRY_MAX_THREADS];
        // This constructor should be called from within a transaction
        ELMetadata() {
            // The 'top' starts after the metadata header
            top = ((uint8_t*)this) + sizeof(ELMetadata);         // pstore()
            for (int i = 0; i < kMaxBlockSize; i++) {
                gflists[i].count = 0;             // pstore()
                gflists[i].head = nullptr;        // pstore()
                gflists[i].tail = nullptr;        // pstore()
            }
            for (int it = 0; it < REGISTRY_MAX_THREADS; it++) {
                new (&tldata[it]) TLData();                      // pstores()
            }
        }
    };

    // Volatile data
    uint8_t* poolAddr {nullptr};
    uint64_t poolSize {0};

    // Pointer to location of PM metadata of EsLoco
    ELMetadata* meta {nullptr};


    // For powers of 2, returns the highest bit, otherwise, returns the next highest bit
    uint64_t highestBit(uint64_t val) {
        uint64_t b = 0;
        while ((val >> (b+1)) != 0) b++;
        if (val > (1ULL << b)) return b+1;
        return b;
    }

    uint8_t* aligned(uint8_t* addr) {
        return (uint8_t*)((size_t)addr & (~0x3FULL)) + 128;
    }


public:
    void init(void* addressOfMemoryPool, size_t sizeOfMemoryPool, bool clearPool=true) {
        // Align the base address of the memory pool
        poolAddr = aligned((uint8_t*)addressOfMemoryPool);
        poolSize = sizeOfMemoryPool + (uint8_t*)addressOfMemoryPool - poolAddr;
        // The first thing in the pool is the ELMetadata
        meta = (ELMetadata*)poolAddr;
        if (clearPool) new (meta) ELMetadata();
        if (debugOn) printf("Starting EsLoco2 with poolAddr=%p and poolSize=%ld, up to %p\n", poolAddr, poolSize, poolAddr+poolSize);
    }

    // Resets the metadata of the allocator back to its defaults
    void reset() {
        std::memset(poolAddr, 0, sizeof(block)*kMaxBlockSize*(REGISTRY_MAX_THREADS+1));
        meta->top.pstore(nullptr);
    }

    // Returns the number of bytes that may (or may not) have allocated objects, from the base address to the top address
    uint64_t getUsedSize() {
        return meta->top.pload() - poolAddr;
    }

    // Takes the desired size of the object in bytes.
    // Returns pointer to memory in pool, or nullptr.
    // Does on average 1 store to persistent memory when re-utilizing blocks.
    void* malloc(size_t size) {
        const int tid = ThreadRegistry::getTID();
        // Adjust size to nearest (highest) power of 2
        uint64_t bsize = highestBit(size + sizeof(block));
        uint64_t asize = (1ULL << bsize);
        if (debugOn) printf("malloc(%ld) requested,  block size exponent = %ld\n", size, bsize);
        block* myblock = nullptr;
        // Check if there is a block of that size in the corresponding freelist
        if (!meta->tldata[tid].tflists[bsize].isEmpty()) {
            if (debugOn) printf("Found available block in thread-local freelist\n");
            myblock = meta->tldata[tid].tflists[bsize].popBlock();
        } else if (asize < kSlabSize/2) {
            // Allocations larger than kSlabSize/2 go on the global top
            if (asize >= meta->tldata[tid].slabsize.pload()) {
                if (debugOn) printf("Not enough space on current slab of thread %d for %ld bytes. Taking new slab from top\n",tid, asize);
                // Not enough space for allocation. Take a new slab from the global top and
                // discard the old one. This means fragmentation/leak is at most kSlabSize/2.
                if (meta->top.pload() + kSlabSize > poolSize + poolAddr) {
                    printf("EsLoco2: Out of memory for slab %ld for %ld bytes allocation\n", kSlabSize, size);
                    return nullptr;
                }
                meta->tldata[tid].slabtop = meta->top.pload();   // pstore()
                meta->tldata[tid].slabsize = kSlabSize;          // pstore()
                meta->top.pstore(meta->top.pload() + kSlabSize); // pstore()
                // TODO: add the remains of the slab to thread-local freelist
            }
            // Take space from the slab
            myblock = (block*)meta->tldata[tid].slabtop.pload();
            myblock->size2 = bsize;                                       // pstore()
            meta->tldata[tid].slabtop.pstore((uint8_t*)myblock + asize); // pstore()
            meta->tldata[tid].slabsize -= asize;                         // pstore()
        } else if (!meta->gflists[bsize].isEmpty()) {
            if (debugOn) printf("Found available block in thread-global freelist\n");
            myblock = meta->gflists[bsize].popBlock();
        } else {
            if (debugOn) printf("Creating new block from top, currently at %p\n", meta->top.pload());
            // Couldn't find a suitable block, get one from the top of the pool if there is one available
            if (meta->top.pload() + asize > poolSize + poolAddr) {
                printf("EsLoco2: Out of memory for %ld bytes allocation\n", size);
                return nullptr;
            }
            myblock = (block*)meta->top.pload();
            meta->top.pstore(meta->top.pload() + asize);                 // pstore()
            myblock->size2 = bsize;                                       // pstore()
        }
        if (debugOn) printf("returning ptr = %p\n", (void*)((uint8_t*)myblock + sizeof(block)));
        //printf("malloc(%ld) = %p\n", asize, (void*)((uint8_t*)myblock + sizeof(block)));
        // Return the block, minus the header
        return (void*)((uint8_t*)myblock + sizeof(block));
    }

    // Takes a pointer to an object and puts the block on the free-list.
    // When the thread-local freelist becomes too large, all blocks are moved to the thread-global free-list.
    // Does on average 2 stores to persistent memory.
    void free(void* ptr) {
        //printf("free(%p)\n", ptr);
        if (ptr == nullptr) return;
        const int tid = ThreadRegistry::getTID();
        block* myblock = (block*)((uint8_t*)ptr - sizeof(block));
        if (debugOn) printf("free(%p)  block size exponent = %ld\n", ptr, myblock->size2.pload());
        // Insert the block in the corresponding freelist
        uint64_t bsize = myblock->size2;
        meta->tldata[tid].tflists[bsize].pushBlock(myblock);
        if (meta->tldata[tid].tflists[bsize].isFull()) {
            if (debugOn) printf("Moving thread-local flist for bsize=%ld to thread-global flist\n", bsize);
            // Move all objects in the thread-local list to the corresponding thread-global list
            meta->tldata[tid].tflists[bsize].transfer(&meta->gflists[bsize]);
        }
    }
};


// Needed by our benchmarks
struct tmbase {
};


struct ReadSet {
    std::atomic<uint64_t>*  log[TX_MAX_LOADS];
    uint64_t                size {0};          // Number of loads in the readSet for the current transaction

    inline bool validate(uint64_t rClock, uint64_t tid) {
        for (uint64_t i = 0; i < size; i++) {
            uint64_t sl = log[i]->load(std::memory_order_acquire);
            if (!isUnlockedOrLockedByMe(rClock, tid, sl)) return false;
        }
        return true;
    }

    inline void add(std::atomic<uint64_t>* mutex) {
        log[size++] = mutex;
        assert (size != TX_MAX_LOADS);
    }
};

// Size of an Optane block in 64bit words. Used to prevent "false sharing".
#define PM_PAD  (256/sizeof(uint64_t))

// The persistent metadata is a 'header'.
struct PMetadata {
    static const uint64_t   MAGIC_ID = 0x1337bab4;
    void*                   root {nullptr}; // Immutable once assigned
    uint64_t                id {0};
    uint64_t                padding[8-2];
    // Each thread has its own p_seq
    volatile uint64_t       p_seq[REGISTRY_MAX_THREADS*PM_PAD];
    PMetadata() {
        for (int it = 0; it < REGISTRY_MAX_THREADS; it++) {
            p_seq[it*PM_PAD] = 1;
            PWB(&p_seq[it*PM_PAD]);
        }
    }
};

// Address of Persistent Metadata.
// This relies on PMetadata being the first thing in the persistent region.
static PMetadata* const pmd = (PMetadata*)PM_REGION_BEGIN;

// Used to identify aborted transactions
struct AbortedTx {};
static constexpr AbortedTx AbortedTxException {};

// Volatile log (write-set)
// One day in the future, if we want to suport large transactions we can make this a dynamic array.
struct AppendLog {
    static const uint64_t CHUNK_SIZE = 16*1024*1024ULL; // Maximum transaction size (in number of modified persist<T>)
    uint64_t   size {0};
    void**     addr;
    AppendLog* next {nullptr};

    AppendLog() {
        addr = new void*[CHUNK_SIZE];
    }
    ~AppendLog() {
        delete[] addr;
    }

    inline void add(void* ptr) {
        addr[size++] = ptr;
        assert (size != CHUNK_SIZE);
    }

    // Apply all the modifications on each back.
    inline void applyOnBack() {
        for (uint64_t i = 0; i < size; i++) {
            persist<uint64_t>* p = (persist<uint64_t>*)addr[i];
            p->back = p->main;
        }
    }

    // Rollback modifications: called from abortTx() to revert changes.
    inline void rollbackMain() {
        for (uint64_t i = 0; i < size; i++) {
            persist<uint64_t>* p = (persist<uint64_t>*)addr[i];
            p->main = p->back;
        }
    }

    // Unlock the lseq in all persist
    inline void unlock(uint64_t nextClock, uint64_t tid) {
        for (uint64_t i = 0; i < size; i++) {
            persist<uint64_t>* p = (persist<uint64_t>*)addr[i];
            p->lseq.store(composeLseq(0,tid,nextClock), std::memory_order_release);
            // We don't really need to flush, but let's do it anyways: it's faster than
            // letting the cache coherence system do it for us.
            //PWB(p);
        }
        // No need for fence, flushing is just to improve performance
    }

    // TODO: fine tune this to see if it really is an optimization in all cases
    inline void flushPWB() {
        // If we have more than 16 modified persist<T>, we just flush all, otherwise,
        // we try to group per cache line, in the hopes of saving a couple of PWB().
        if (size > 16) {
            for (uint64_t i = 0; i < size; i++) PWB(addr[i]);
        } else {
            for (uint64_t i = 0; i < size; i++) {
                // Check if we already flushed this cache line
                const void* addr_i_cl = ADDR2CL(addr[i]);
                bool flushit = true;
                for (uint64_t j = 0; j < i; j++) {
                    if (ADDR2CL(addr[j]) == addr_i_cl) {
                        flushit = false;
                        break;
                    }
                }
                if (flushit) PWB(addr[i]);
            }
        }
    }
};


// Thread-local data
struct OpData {
    uint64_t              tid;
    uint64_t              rClock {0};
    int                   tx_type {TX_IS_NONE}; // This is used by persist::load() to figure out if it needs to save a load on the read-set or not
    ReadSet               readSet;              // The (volatile) read set
    AppendLog             v_log {};             // Write-set: The append-only log of modified persist<T>
    uint64_t              nestedTrans {0};      // Number of nested transactions
    uint64_t              myrand;
    uint64_t              numAborts {0};
    uint64_t              numCommits {0};
    uint64_t              padding[16];
};

// This is used by addToLog() to know which OpData instance to use for the current transaction
extern thread_local OpData* tl_opdata;

class Trinity;
extern Trinity gTrinity;


class Trinity {
private:
    // Padding on x86 should be on 2 cache lines
    static const int                       CLPAD = 128/sizeof(uintptr_t);
    bool                                   reuseRegion {false};                 // used by the constructor and initialization
    int                                    pfd {-1};
    alignas(128) std::atomic<uint64_t>     gClock {1};
    alignas(128) OpData                   *opDesc;
    EsLoco2<persist>                       esloco {};

public:
    struct tmbase : public trinitytl2::tmbase { };

    Trinity() {
        opDesc = new OpData[REGISTRY_MAX_THREADS];
        for (uint64_t it=0; it < REGISTRY_MAX_THREADS; it++) {
            opDesc[it].tid = it;
            opDesc[it].myrand = (it+1)*12345678901234567ULL;
        }
        mapPersistentRegion(PM_FILE_NAME, (uint8_t*)PM_REGION_BEGIN, PM_REGION_SIZE);
    }

    ~Trinity() {
        uint64_t totalAborts = 0;
        uint64_t totalCommits = 0;
        for (int it=0; it < REGISTRY_MAX_THREADS; it++) {
            totalAborts += opDesc[it].numAborts;
            totalCommits += opDesc[it].numCommits;
        }
        printf("totalAborts=%ld  totalCommits=%ld  abortRatio=%.1f%% \n", totalAborts, totalCommits, 100.*totalAborts/(1+totalCommits));
        delete[] opDesc;
    }

    static std::string className() { return "Trinity-TL2"; }

    void mapPersistentRegion(const char* filename, uint8_t* regionAddr, const uint64_t regionSize) {
        // Check that the header with the logs leaves at least half the memory available to the user
        if (sizeof(PMetadata) > regionSize/2) {
            printf("ERROR: the size of the header in persistent memory is so large that it takes more than half the whole persistent memory\n");
            printf("Please reduce some of the settings in TrinityTL2.hpp and try again\n");
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
        if (got_addr == MAP_FAILED) {
            perror("ERROR: mmap() returned MAP_FAILED !!! ");
            assert(false);
        }
        if (got_addr != regionAddr) {
            printf("got_addr = %p instead of %p\n", got_addr, regionAddr);
            perror("Retry mmap() in a couple of seconds");
            std::exit(42);
        }
        // If the file has just been created or if the header is not consistent, clear everything.
        // Otherwise, re-use and recover to a consistent state.
        if (reuseRegion) {
            recover();
            readTx([&]{
            	esloco.init(regionAddr+sizeof(PMetadata), regionSize-sizeof(PMetadata), false);
            });
        } else {
            new (regionAddr) PMetadata();
            // We reset the entire memory region because the 'lseq' have to be zero
            resetPM(regionAddr, regionSize);
            updateTx([&]{
            	esloco.init(regionAddr+sizeof(PMetadata), regionSize-sizeof(PMetadata), true);
            	pmd->root = esloco.malloc(sizeof(persist<void*>)*MAX_ROOT_POINTERS);
            });
            PWB(&pmd->root);
            PFENCE();
            pmd->id = PMetadata::MAGIC_ID;
            PWB(&pmd->id);
            PFENCE();
        }
    }

    // This is equivalent to
    // std:memset(regionAddr+sizeof(PMetadata), 0, regionSize-sizeof(PMetadata));
    void resetPM(uint8_t* regionAddr, const uint64_t regionSize) {
        // TODO: we can optimize by doing a memcmp() first, block by block
        if (regionSize > 4*1024*1024*1024ULL) printf("memset() on large PM region. This will take a while...\n");
        const uint64_t kPageSize = 4*1024*1024; // 4 MB pages
        uint8_t* zpage = new uint8_t[kPageSize];
        std::memset(zpage, 0, kPageSize);
        uint8_t* page = regionAddr + sizeof(PMetadata);
        uint8_t* end = regionAddr + regionSize;
        std:memset(page, 0, ((uint64_t)page) % kPageSize);
        while (page+kPageSize < end) {
            if (!std::memcmp(page, zpage, kPageSize)) std::memset(page, 0, kPageSize);
            page += kPageSize;
        }
        // TODO: last section
        delete[] zpage;
    }

    inline void beginTx(OpData* myd, const int tid) {
        // Clear the logs of the previous transaction
        myd->v_log.size = 0;
        myd->readSet.size = 0;
        myd->rClock = gClock.load(); // This is GVRead()
        if (myd->tx_type == TX_IS_UPDATE) {
            pmd->p_seq[tid*PM_PAD] = myd->rClock;
            PWB(&pmd->p_seq[tid*PM_PAD]);
            // No need for pfence because the first pstore() will issue a CAS
        }
    }

    // This PTM uses redo-log with eager locking, which means that by now all locks have been acquired.
    inline bool endTx(OpData* myd, const int tid) {
        // Check if this is a read-only transaction and if so, commit immediately
        if (myd->v_log.size == 0) return true;
        // This fence is needed by undo log to prevent re-ordering with the last store
        // and the reading of the gClock.
        std::atomic_thread_fence(std::memory_order_seq_cst);
        // Validate the read-set
        if (!myd->readSet.validate(myd->rClock, tid)) {
            abortTx(myd, tid);
            return false;
        }
        // Flush seq+main modifications
        myd->v_log.flushPWB();
        // The FAA is 'hijacked' to act as a persistence fence
        uint64_t nextClock = gClock.fetch_add(1)+1;
        // Modifications in persist<T> must be flushed before advancing p_seq
        pmd->p_seq[tid*PM_PAD] = nextClock;
        PWB(&pmd->p_seq[tid*PM_PAD]);
        PSYNC();
        // Apply modifications on back
        myd->v_log.applyOnBack();
        // Unlock and set new sequence and flush them (no fence needed)
        myd->v_log.unlock(nextClock, tid);
        myd->numCommits++;
        return true;
    }

    // This is called only from persist::load()/store() and endTx() if the read-set validation fails.
    inline void abortTx(OpData* myd, const int tid) {
        // Rollback modifications and flush them
        myd->v_log.rollbackMain();
        myd->v_log.flushPWB();
        // The FAA acts a PFENCE() and seq-cst fence
        uint64_t nextClock = gClock.fetch_add(1)+1;
        pmd->p_seq[tid*PM_PAD] = nextClock;
        PWB(&pmd->p_seq[tid*PM_PAD]);
        PFENCE();                               // Modifications in persist<T> are done before incrementing p_seq
        // Unlock and set new sequence and flush them (no fence needed)
        myd->v_log.unlock(nextClock, tid);
        myd->numAborts++;
    }

    template<typename F> void transaction(F&& func, int txType=TX_IS_UPDATE) {
        const int tid = ThreadRegistry::getTID();
        OpData* myd = &opDesc[tid];
        if (myd->nestedTrans > 0) {
            func();
            return ;
        }
        tl_opdata = myd;
        myd->tx_type = txType;
        ++myd->nestedTrans;
        for (uint64_t attempt = 0; ; attempt++) {
            backoff(myd, attempt);
            beginTx(myd, tid);
            try {
                func();
            } catch (AbortedTx&) {
                abortTx(myd, tid);
                continue;
            }
            if (endTx(myd, tid)) break;
        }
        tl_opdata = nullptr;
        --myd->nestedTrans;
    }

    // It's silly that these have to be static, but we need them for the (SPS) benchmarks due to templatization
    //template<typename R, typename F> static R updateTx(F&& func) { return gTrinity.transaction<R>(func, TX_IS_UPDATE); }
    //template<typename R, typename F> static R readTx(F&& func) { return gTrinity.transaction<R>(func, TX_IS_READ); }
    template<typename F> static void updateTx(F&& func) { gTrinity.transaction(func, TX_IS_UPDATE); }
    template<typename F> static void readTx(F&& func) { gTrinity.transaction(func, TX_IS_READ); }
    // There are no sequential durable transactions in TL2 but we "emulate it" with a concurrent+durable tx
    template<typename F> static void updateTxSeq(F&& func) { gTrinity.transaction(func, TX_IS_UPDATE); }
    template<typename F> static void readTxSeq(F&& func) { gTrinity.transaction(func, TX_IS_READ); }


    // Scan the PM for any persist<> with a sequence equal to p_seq.
    // If there are any, revert them by copying back to main and resetting
    // the seq to zero.
    //
    // If p_seq == seq+1 then copy back to main
    // If p_seq == seq   then copy main to back
    void recover() {
        // The persists start after PMetadata
        persist<uint64_t>* pstart = (persist<uint64_t>*)(PM_REGION_BEGIN + sizeof(PMetadata));
        persist<uint64_t>* pend = (persist<uint64_t>*)(PM_REGION_BEGIN + PM_REGION_SIZE);
        persist<uint64_t>* p; // iterator variable
        for (p = pstart; p < pend; p++) {
            const lseq_t lseq = p->lseq.load(std::memory_order_relaxed);
        	if (isUnlocked(lseq)) continue;
        	const uint64_t tid = lseq2tid(lseq);
            if (lseq2seq(lseq) == pmd->p_seq[tid*PM_PAD]) {
                p->main = p->back;    // ordered store
            } else {
                p->back = p->main;    // ordered store
            }
            p->lseq = 0;              // ordered store
            PWB(p);
        }
        PSYNC();
        // The global clock must be equal to the highest of the p_seqs
        uint64_t maxClock = 0;
        for (int it = 0; it < REGISTRY_MAX_THREADS; it++) {
            if (pmd->p_seq[it*PM_PAD] > maxClock) maxClock = pmd->p_seq[it*PM_PAD];
        }
        gClock.store(maxClock);
    }

    // Random number generator used by the backoff scheme
    inline uint64_t marsagliaXORV(uint64_t x) {
        if (x == 0) x = 1;
        x ^= x << 6;
        x ^= x >> 21;
        x ^= x << 7;
        return x;
    }

    // Backoff for a random amount of steps in the range [16, 16*attempt]. Inspired by TL2
    inline void backoff(OpData* myopd, uint64_t attempt) {
        if (attempt < 3) return;
        if (attempt == 10000) printf("Ooops, looks like we're stuck attempt=%ld\n", attempt);
        myopd->myrand = marsagliaXORV(myopd->myrand);
        uint64_t stall = (myopd->myrand & attempt) + 1;
        stall *= 16;
        std::atomic<uint64_t> iter {0};
        while (iter.load() < stall) iter.fetch_add(1);
        if (stall > 1000) std::this_thread::yield();
    }

    // TODO: Remove these two once we make CX have void transactions
    template<typename R,class F> inline static R readTx(F&& func) {
        gTrinity.transaction([&]() {func();}, TX_IS_READ);
        return R{};
    }
    template<typename R,class F> inline static R updateTx(F&& func) {
        gTrinity.transaction([&]() {func();}, TX_IS_UPDATE);
        return R{};
    }

    template <typename T, typename... Args> static T* tmNew(Args&&... args) {
        OpData* const myd = tl_opdata;
        if (myd->nestedTrans == 0) {
            printf("ERROR: Can not allocate outside a transaction\n");
            return nullptr;
        }
        void* ptr = gTrinity.esloco.malloc(sizeof(T));
        // If we get nullptr then we've ran out of PM space
        assert(ptr != nullptr);
        // If there is an abort during the 'new placement', the transactions rolls
        // back and will automatically revert the changes in the allocator metadata.
        new (ptr) T(std::forward<Args>(args)...);  // new placement
        return (T*)ptr;
    }

    template<typename T> static void tmDelete(T* obj) {
        if (obj == nullptr) return;
        OpData* const myd = tl_opdata;
        if (myd->nestedTrans == 0) {
            printf("ERROR: Can not de-allocate outside a transaction\n");
            return;
        }
        obj->~T();
        gTrinity.esloco.free(obj);
    }

    static void* tmMalloc(size_t size) {
        OpData* const myopd = tl_opdata;
        if (myopd->nestedTrans == 0) {
            printf("ERROR: Can not allocate outside a transaction\n");
            return nullptr;
        }
        void* obj = gTrinity.esloco.malloc(size);
        return obj;
    }

    static void tmFree(void* obj) {
        if (obj == nullptr) return;
        OpData* const myopd = tl_opdata;
        if (myopd->nestedTrans == 0) {
            printf("ERROR: Can not de-allocate outside a transaction\n");
            return;
        }
        gTrinity.esloco.free(obj);
    }

    static void* pmalloc(size_t size) {
        OpData* const myopd = tl_opdata;
        if (myopd->nestedTrans == 0) {
            printf("ERROR: Can not allocate outside a transaction\n");
            return nullptr;
        }
        return gTrinity.esloco.malloc(size);
    }

    static void pfree(void* obj) {
        if (obj == nullptr) return;
        OpData* const myopd = tl_opdata;
        if (myopd->nestedTrans == 0) {
            printf("ERROR: Can not de-allocate outside a transaction\n");
            return;
        }
        gTrinity.esloco.free(obj);
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


// Store interposing: acquire the lock and write in 'main'
template<typename T> inline void persist<T>::pstore(T newVal) {
    OpData* const myd = tl_opdata;
    const uint8_t* valaddr = (uint8_t*)this;
    // Do not log nor lock this store unless it's in the memory region and we're in a transaction
    if (myd == nullptr || valaddr < (uint8_t*)PM_REGION_BEGIN || valaddr > PM_REGION_END) {
        main = (uint64_t)newVal;
        return;
    }
    lseq_t sl = lseq.load(std::memory_order_acquire);
    if (isUnlockedOrLockedByMe(myd->rClock, myd->tid, sl)) {
    	const uint64_t p_seq = pmd->p_seq[myd->tid*PM_PAD];
        if ((sl & LOCKED) || lseq.compare_exchange_strong(sl, composeLseq(LOCKED, myd->tid, p_seq))) {
            // If it's the first time we touch this persist<T> in this tx: log it.
            if (isUnlocked(sl)) myd->v_log.add(this);
            main = (uint64_t)newVal;
            return;
        }
    }
    throw AbortedTxException;
}

// This is similar to an undo-log load interposing: do single check
template<typename T> inline T persist<T>::pload() const {
    OpData* const myd = tl_opdata;
    // Check if we're outside a transaction
    if (myd == nullptr) return (T)main;
    uint64_t lval = main;
    lseq_t sl = lseq.load(std::memory_order_acquire);
    if (!isUnlockedOrLockedByMe(myd->rClock, myd->tid, sl)) throw AbortedTxException;
    // When in a write tx, loads must be added to the read-set
    if (myd->tx_type == TX_IS_UPDATE && isUnlocked(sl)) myd->readSet.add((std::atomic<uint64_t>*)&lseq);
    return (T)lval;
}


//
// Wrapper methods to the global TM instance. The user should use these:
//
template<typename R, typename F> static R updateTx(F&& func) { return gTrinity.transaction<R>(func, TX_IS_UPDATE); }
template<typename R, typename F> static R readTx(F&& func) { return gTrinity.transaction<R>(func, TX_IS_READ); }
template<typename F> static void updateTx(F&& func) { gTrinity.transaction(func, TX_IS_UPDATE); }
template<typename F> static void readTx(F&& func) { gTrinity.transaction(func, TX_IS_READ); }
template<typename T, typename... Args> T* tmNew(Args&&... args) { return Trinity::tmNew<T>(args...); }
template<typename T> void tmDelete(T* obj) { Trinity::tmDelete<T>(obj); }
static void* tmMalloc(size_t size) { return Trinity::tmMalloc(size); }
static void tmFree(void* obj) { Trinity::tmFree(obj); }


//
// Place these in a .cpp if you include this header from multiple files (compilation units)
//
// Global/singleton to hold all the thread registry functionality
ThreadRegistry gThreadRegistry {};
// PTM singleton
Trinity gTrinity {};
// Thread-local data of the current ongoing transaction
thread_local OpData* tl_opdata {nullptr};
// This is where every thread stores the tid it has been assigned when it calls getTID() for the first time.
// When the thread dies, the destructor of ThreadCheckInCheckOut will be called and de-register the thread.
thread_local ThreadCheckInCheckOut tl_tcico {};
// Helper function for thread de-registration
void thread_registry_deregister_thread(const int tid) {
    gThreadRegistry.deregister_thread(tid);
}

}
#endif /* _TRINITY_TL2_PERSISTENT_TRANSACTIONAL_MEMORY_H_ */
