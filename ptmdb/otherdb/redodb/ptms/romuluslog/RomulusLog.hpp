/*
 * Copyright 2017-2020
 *   Andreia Correia <andreia.veiga@unine.ch>
 *   Pedro Ramalhete <pramalhe@gmail.com>
 *   Pascal Felber <pascal.felber@unine.ch>
 *
 * This work is published under the MIT license. See LICENSE.txt
 */
#ifndef _ROMULUS_LOG_PERSISTENT_TRANSACTIONAL_MEMORY_
#define _ROMULUS_LOG_PERSISTENT_TRANSACTIONAL_MEMORY_
#include <atomic>
#include <cstdint>
#include <cassert>
#include <string>
#include <cstring>      // std::memcpy()
#include <sys/mman.h>   // Needed if we use mmap()
#include <sys/types.h>  // Needed by open() and close()
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>     // Needed by close()
#include <linux/mman.h> // Needed by MAP_SHARED_VALIDATE
#include <stdio.h>
#include <functional>

#include "../common/pfences.h"
#include "../common/ThreadRegistry.hpp"
#include "../rwlocks/CRWWPSpinLock.hpp"

// Size of the persistent memory region
#ifndef PM_REGION_SIZE
#define PM_REGION_SIZE (2*1024*1024*1024ULL) // 2GB by default (to run on laptop)
#endif
// DAX flag (MAP_SYNC) is needed for Optane but not for /dev/shm/
#ifdef PM_USE_DAX
#define PM_FLAGS       MAP_SYNC
#else
#define PM_FLAGS       0
#endif
// Name of persistent file mapping
#ifndef PM_FILE_NAME
#define PM_FILE_NAME   "/dev/shm/romulus_log_shared"
#endif

namespace romuluslog {

// Forward declaration of RomulusLog to create a global instance
class RomulusLog;
extern RomulusLog gRomLog;

extern uint64_t g_main_size;
extern uint8_t* g_main_addr;

// Counter of nested write transactions
extern thread_local int64_t tl_nested_write_trans;
// Counter of nested read-only transactions
extern thread_local int64_t tl_nested_read_trans;
extern bool histoOn;
extern bool histoflag;


#ifdef USE_ESLOCO
#include "../common/EsLoco.hpp"
// forward declaration
template<typename T> struct persist;
#else
typedef void* mspace;
extern void* mspace_malloc(mspace msp, size_t bytes);
extern void mspace_free(mspace msp, void* mem);
extern mspace create_mspace_with_base(void* base, size_t capacity, int locked);
#endif

/*
 * <h1> Romulus Log </h1>
 * TODO: explain this...
 *
 *
 *
 */
class RomulusLog {
    // Id for sanity check of Romulus
    static const uint64_t MAGIC_ID = 0x1337BAB2;

    // Number of indexes available for put_object/get_object
    static const int NUM_ROOT_PTRS = 100;

    // Possible values for "state"
    static const int IDLE = 0;
    static const int MUTATING = 1;
    static const int COPYING = 2;

    // Number of log entries in a chunk of the log
    static const int CHUNK_SIZE = 1024;

    // Member variables
    const char* MMAP_FILENAME = PM_FILE_NAME;
    bool dommap;
    int fd = -1;
    uint8_t* base_addr;
    uint8_t* real_base_addr;
    uint64_t max_size;
    uint8_t* main_addr;
    uint8_t* back_addr;
    CRWWPSpinLock rwlock {};

    // Stuff used by the Flat Combining mechanism
    static const int CLPAD = 128/sizeof(uintptr_t);
    alignas(128) std::atomic< std::function<void()>* >* fc; // array of atomic pointers to functions
    const int maxThreads;

    // Each log entry is two words (8+8 = 16 bytes)
    struct LogEntry {
        size_t    offset;  // Pointer offset in bytes, relative to main_addr
        uint64_t  length;  // Range length of data at pointer offset
    };

    struct LogChunk {
        LogEntry  entries[CHUNK_SIZE];
        uint64_t  num_entries { 0 };
        LogChunk* next        { nullptr };
    };

    // There is always at least one (empty) chunk in the log, it's the head
    LogChunk* log_head = new LogChunk;
    LogChunk* log_tail = log_head;

    // One instance of this is at the start of base_addr, in persistent memory
    struct PersistentHeader {
        uint64_t         id {0};          // Validates intialization
        std::atomic<int> state {IDLE};    // Current state of consistency
        void**           objects {};      // Objects directory
#ifdef USE_ESLOCO
#else
        mspace           ms {};           // Pointer to allocator's metadata
#endif
        uint64_t         used_size {0};   // It has to be the last, to calculate the used_size
    };

    PersistentHeader* per {nullptr};      // Volatile pointer to start of persistent memory
    uint64_t log_size = 0;
    bool logEnabled = true;

#ifdef USE_ESLOCO
    EsLoco<persist> *esloco {nullptr};
#endif


public:

    int* histo  = new int[300]; // array of atomic pointers to functions
    int storecount = 0;

    // Flush touched cache lines
    inline static void flush_range(uint8_t* addr, size_t length) noexcept {
        const int cache_line_size = 64;
        uint8_t* ptr = addr;
        uint8_t* last = addr + length;
        for (; ptr < last; ptr += cache_line_size) {
        	PWB(ptr);
        }
    }

    // Flush touched cache lines
    inline static void log_flush_range(uint8_t* addr, size_t length) noexcept {
        const int cache_line_size = 64;
        uint8_t* ptr = addr;
        uint8_t* last = addr + length;
        for (; ptr < last; ptr += cache_line_size) {
        	gRomLog.add_to_log(ptr,8);
        	PWB(ptr);
        }
    }

private:

    // Copy the data from 'main' to 'back'
	void copyMainToBack() {
		uint64_t size = std::min(per->used_size,g_main_size);
		std::memcpy(back_addr, main_addr, size);
		flush_range(back_addr, size);
	}

	// Copy the data from 'back' to 'main'
	void copyBackToMain() {
		uint64_t size = std::min(per->used_size,g_main_size);
		std::memcpy(main_addr, back_addr, size);
		flush_range(main_addr, size);
	}

    /*
     * Called to make every store persistent on main and back region
     */
    inline void apply_pwb(uint8_t* from_addr) {
        // Apply the log to the instance on 'to_addr', copying data from the instance at 'from_addr'
        LogChunk* chunk = log_head;
        while (chunk != nullptr) {
            for (int i = 0; i < chunk->num_entries; i++) {
                LogEntry& e = chunk->entries[i];
                //std::memcpy(to_addr + e.offset, from_addr + e.offset, e.length);
                flush_range(from_addr + e.offset, e.length);

            }
            chunk = chunk->next;
        }
    }

    /*
     * Called at the end of a transaction to replicate the mutations on "back",
     * or when abort_transaction() is called by the user, to rollback the
     * mutations on "main".
     * Deletes the log as it is being applied.
     */
    inline void apply_log(uint8_t* from_addr, uint8_t* to_addr) {
        // Apply the log to the instance on 'to_addr', copying data from the instance at 'from_addr'
        LogChunk* chunk = log_head;
        while (chunk != nullptr) {
            for (int i = 0; i < chunk->num_entries; i++) {
                LogEntry& e = chunk->entries[i];
                //printf("entry %i of %d from addr %p to addr %p  offset=%ld\n", i, chunk->num_entries, from_addr + e.offset, to_addr + e.offset, e.offset);
                std::memcpy(to_addr + e.offset, from_addr + e.offset, e.length);
            }
            chunk = chunk->next;
        }
    }

    inline void clear_log() {
        LogChunk* chunk = log_head->next;
        while (chunk != nullptr) {
            LogChunk* next = chunk->next;
            delete chunk;
            chunk = next;
        }
        // Clear the log, leaving one chunk for next transaction, with zero'ed entries
        log_tail = log_head;
        log_head->num_entries = 0;
        log_head->next = nullptr;
    }


public:

    /*
     * Adds to the log the current contents of the memory location starting at
     * 'addr' with a certain 'length' in bytes
     */
    inline void add_to_log(void* addr, int length) noexcept {
       if (!logEnabled) return;
       // If the log has more than 1/4 of the entire size then skip the log
       // and copy the used size of the main region.
       if (log_size > per->used_size/4) {
           logEnabled = false;
           return;
       }

       size_t addrCL = ((size_t)addr)>>6;
       // Get the current chunk of log and if it is already full then create a new chunk and add the entry there.
       LogChunk* chunk = log_tail;

       bool sameCL = false;

       if(addrCL == (size_t)((uint8_t*)addr+length)>>6){
           sameCL = true;
           int size = chunk->num_entries;
           for(int i=size-1;i>=0 && i>size-16;i--){
               LogEntry& e1 = chunk->entries[i];

               size_t offCL = (size_t)(e1.offset+main_addr)>>6;
               if(e1.length==64 && (size_t)(offCL<<6) == (size_t)(e1.offset+main_addr)){
                   if(offCL == addrCL) return;
               }
           }
       }
       if (chunk->num_entries == CHUNK_SIZE) {
           chunk = new LogChunk();
           log_tail->next = chunk;
           log_tail = chunk;
       }
       LogEntry& e = chunk->entries[chunk->num_entries];
       if(histoOn) gRomLog.storecount+=2;

       if(sameCL){
           size_t cl =addrCL<<6;
           e.offset = (uint8_t*)cl - main_addr;
           e.length = 64;
       }else {
           e.offset = (uint8_t*)addr - main_addr;
           e.length = length;
       }
       log_size += length;
       chunk->num_entries++;
    }


    RomulusLog() : dommap{true},maxThreads{128} {
       fc = new std::atomic< std::function<void()>* >[maxThreads*CLPAD];
       for (int i = 0; i < maxThreads; i++) {
           fc[i*CLPAD].store(nullptr, std::memory_order_relaxed);
       }
       // Filename for the mapping file
       /*if (dommap) {

       }*/
       init();
    }


    ~RomulusLog() {
       delete[] fc;
       // Must do munmap() if we did mmap()
       if (dommap) {
           munmap(base_addr, max_size);
           close(fd);
       }
       if(histoflag){
        for(int i=0;i<300;i++){
            std::cout<<i<<":"<<histo[i]<<"\n";
        }
       }
    }

    void ns_init() {
       base_addr = (uint8_t*)0x7fdd40000000;

       real_base_addr = (uint8_t*)(((size_t)(base_addr + 128)<<6)>>6);

       max_size = PM_REGION_SIZE;
       // Check if the file already exists or not
       struct stat buf;
       if (stat(MMAP_FILENAME, &buf) == 0) {
           // File exists
           //std::cout << "Re-using memory region\n";
           fd = open(MMAP_FILENAME, O_RDWR|O_CREAT, 0755);
           assert(fd >= 0);
           // mmap() memory range
           uint8_t* got_addr = (uint8_t *)mmap(base_addr, max_size, (PROT_READ | PROT_WRITE), MAP_SHARED_VALIDATE | PM_FLAGS, fd, 0);
           if (got_addr == MAP_FAILED || got_addr != base_addr) {
               perror("ERROR: mmap() is not working !!! ");
               printf("got_addr = %p instead of %p\n", got_addr, base_addr);
               assert(false);
           }
           per = reinterpret_cast<PersistentHeader*>(real_base_addr- sizeof(PersistentHeader));
           if (per->id != MAGIC_ID) createFile();
           g_main_size = (max_size -(real_base_addr - base_addr))/2;
           main_addr = real_base_addr;
           back_addr = main_addr + g_main_size;
           g_main_addr = main_addr;
    #ifdef USE_ESLOCO
           esloco = new EsLoco<persist>(main_addr, g_main_size, false);
    #endif
           recover();
       } else {
           createFile();
       }
    }

    void createFile(){
       // File doesn't exist
       fd = open(MMAP_FILENAME, O_RDWR|O_CREAT, 0755);
       assert(fd >= 0);
       if (lseek(fd, max_size-1, SEEK_SET) == -1) {
           perror("lseek() error");
       }
       if (write(fd, "", 1) == -1) {
           perror("write() error");
       }
       // mmap() memory range
       uint8_t* got_addr = (uint8_t *)mmap(base_addr, max_size, (PROT_READ | PROT_WRITE), MAP_SHARED, fd, 0);
       if (got_addr == MAP_FAILED) {
           printf("got_addr = %p  %p\n", got_addr, MAP_FAILED);
           perror("ERROR: mmap() is not working !!! ");
           assert(false);
       }
       // No data in persistent memory, initialize
       per = new ((real_base_addr- sizeof(PersistentHeader))) PersistentHeader;
       //uint8_t* lostBytes = real_base_addr - base_addr;
       g_main_size = (max_size-(real_base_addr - base_addr))/2;
       main_addr = real_base_addr;
       back_addr = main_addr + g_main_size;
       g_main_addr = main_addr;
       PWB(&per->id);
       PWB(&per->state);
       // We need to call create_mspace_with_base() from within a transaction so that
       // the modifications on 'main' get replicated on 'back'. This means we temporarily
       // need to set the 'used_size' to 'main_size' to make sure everything is copied.
       begin_transaction();
       // Just to force the copy of the whole main region
       per->used_size = g_main_size;
    #ifdef USE_ESLOCO
       esloco = new EsLoco<persist>(main_addr, g_main_size, false);
       per->objects = (void**)esloco->malloc(sizeof(void*)*NUM_ROOT_PTRS);
    #else
       per->ms = create_mspace_with_base(main_addr, g_main_size, false);
       per->objects = (void**)mspace_malloc(per->ms, sizeof(void*)*NUM_ROOT_PTRS);
    #endif
       for (int i = 0; i < NUM_ROOT_PTRS; i++) {
           per->objects[i] = nullptr;
           add_to_log(&per->objects[i],sizeof(void*));
           PWB(&per->objects[i]);
       }
       end_transaction();
       // The used bytes in the main region
       per->used_size = (uint8_t*)(&per->used_size) - (uint8_t*)real_base_addr+128;
       flush_range((uint8_t*)per,sizeof(PersistentHeader));
       PFENCE();
       // Finally, set the id to confirm that the whole initialization process has completed
       per->id = MAGIC_ID;
       PWB(&per->id);
       PSYNC();
       consistency_check();
    }

    static std::string className() { return "RomulusLog"; }


    template <typename T>
    static inline T* get_object(int idx) {
        // Equivalent to persist<void*>.pload()
        return (T*)gRomLog.per->objects[idx];
    }

    template <typename T>
    static inline void put_object(int idx, T* obj) {
        // Equivalent to persist<void*>.pstore()
        gRomLog.add_to_log(&gRomLog.per->objects[idx],sizeof(T*));
        gRomLog.per->objects[idx] = obj;
        PWB(&gRomLog.per->objects[idx]);
    }

    void ns_reset(){
        per->id = MAGIC_ID;
        PWB(&per->id);
        PFENCE();
        std::memset(base_addr,0,max_size);

        // No data in persistent memory, initialize
        per = new (base_addr) PersistentHeader;
        g_main_size = (max_size - sizeof(PersistentHeader))/2;
        main_addr = base_addr + sizeof(PersistentHeader);
        back_addr = main_addr + g_main_size;
        PWB(&per->id);
        PWB(&per->state);
        // We need to call create_mspace_with_base() from within a transaction so that
        // the modifications on 'main' get replicated on 'back'. This means we temporarily
        // need to set the 'used_size' to 'main_size' to make sure everything is copied.
        begin_transaction();
        // Just to force the copy of the whole main region
        per->used_size = g_main_size;
    #ifdef USE_ESLOCO
        esloco = new EsLoco<persist>(main_addr, g_main_size, false);
        per->objects = (void**)esloco->malloc(sizeof(void*)*NUM_ROOT_PTRS);
    #else
        per->ms = create_mspace_with_base(main_addr, g_main_size, false);
        per->objects = (void**)mspace_malloc(per->ms, sizeof(void*)*NUM_ROOT_PTRS);
    #endif
        for (int i = 0; i < NUM_ROOT_PTRS; i++) {
            per->objects[i] = nullptr;
            add_to_log(&per->objects[i],sizeof(void*));
            PWB(&per->objects[i]);
        }
        end_transaction();
        // The used bytes in the main region
        per->used_size = (uint8_t*)(&per->used_size) - ((uint8_t*)base_addr+sizeof(PersistentHeader))+128;
        flush_range((uint8_t*)per,sizeof(PersistentHeader));
        PFENCE();
        // Finally, set the id to confirm that the whole initialization process has completed
        per->id = MAGIC_ID;
        PWB(&per->id);
        PSYNC();
    }

    static void reset() {
        gRomLog.ns_reset();
    }

    /*
     * Must be called at the beginning of each (write) transaction.
     * This function has strict semantics.
     */
    inline void begin_transaction() {
        // Check for nested transaction
        tl_nested_write_trans++;
        if (tl_nested_write_trans != 1) return;
        per->state.store(MUTATING, std::memory_order_relaxed);
        PWB(&per->state);
        // One PFENCE() is enough for all user modifications because no ordering is needed between them.
        PFENCE();
    }


    /*
     * Must be called at the end of each (write) transaction.
     * This function has strict semantics.
     */
    inline void end_transaction() {
        // Check for nested transaction
        --tl_nested_write_trans;
        if (tl_nested_write_trans != 0) return;
        // Do a PFENCE() to make persistent the stores done in 'main' and on
        // the Romulus persistent data (due to memory allocation). We only care
        // about ordering here, not durability, therefore, no need to block.
        apply_pwb(main_addr);
        PFENCE();
        per->state.store(COPYING, std::memory_order_relaxed);
        PWB(&per->state);
        PWB(&per->used_size);
        // PSYNC() here to have ACID Durability on the mutations done to 'main'
        // and make the change of state visible.
        PSYNC();
        // Apply log, copying data from 'main' to 'back'
        if (logEnabled) {
            apply_log(main_addr, back_addr);
            apply_pwb(back_addr);
        } else {
            copyMainToBack();
            logEnabled = true;
        }
        clear_log();
        log_size = 0;
        PFENCE();
        per->state.store(IDLE, std::memory_order_relaxed);
    }


    bool compareMainAndBack() {
        if (std::memcmp(main_addr, back_addr, g_main_size) != 0) {
            void* firstaddr = nullptr;
            int sumdiff = 0;
            for (size_t idx = 0; idx < g_main_size-sizeof(size_t); idx++) {
                if (*(main_addr+idx) != *(back_addr+idx)) {
                    printf("Difference at %p  main=%ld  back=%ld\n", main_addr+idx, *(int64_t*)(main_addr+idx), *(int64_t*)(back_addr+idx));
                    sumdiff++;
                    if (firstaddr == nullptr) firstaddr = main_addr+idx;
                }
            }
            if (sumdiff != 0) {
                printf("sumdiff=%d bytes\n", sumdiff);
                printf("\nThere seems to be a missing persist<T> in your code.\n");
                printf("Rerun with gdb and set a watchpoint using the command\nwatch * %p\n\n", firstaddr);
            }
            assert(sumdiff == 0);
        }
        return true;
    }


    /*
     * Recovers from an incomplete transaction if needed
     */
    inline void recover() {
        int lstate = per->state.load(std::memory_order_relaxed);
        if (lstate == IDLE) {
            return;
        } else if (lstate == COPYING) {
            printf("RomulusLog: Recovery from COPYING...\n");
            copyMainToBack();
        } else if (lstate == MUTATING) {
            printf("RomulusLog: Recovery from MUTATING...\n");
            copyBackToMain();
        } else {
            assert(false);
            // ERROR: corrupted state
        }
        PFENCE();
        per->state.store(IDLE, std::memory_order_relaxed);
        return;
    }

    // Same as begin/end transaction, but with a lambda.
    template<typename R, class F>
    R transaction(F&& func) {
        begin_transaction();
        R retval = func();
        end_transaction();
        return retval;
    }

    template<class F>
    static void transaction(F&& func) {
        gRomLog.begin_transaction();
        func();
        gRomLog.end_transaction();
    }


    /*
     * Non static, thread-safe
     * Progress: Blocking (starvation-free)
     */
    template<class Func>
    void ns_write_transaction(Func&& mutativeFunc) {
        if (tl_nested_write_trans > 0) {
            mutativeFunc();
            return;
        }
        std::function<void()> myfunc = mutativeFunc;
        int tid = ThreadRegistry::getTID();
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

        if(histoflag) storecount=0;
        per->state.store(MUTATING, std::memory_order_relaxed);
        PWB(&per->state);
        // One PFENCE() is enough for all user modifications because no ordering is needed between them.
        PFENCE();
        rwlock.waitForReaders();

        ++tl_nested_write_trans;
        // Apply all mutativeFunc
        for (int i = 0; i < maxTid; i++) {
            if (lfc[i] == nullptr) continue;
            (*lfc[i])();
        }
        apply_pwb(main_addr);
        PFENCE();
        per->state.store(COPYING, std::memory_order_relaxed);
        PWB(&per->state);
        // PSYNC() here to have ACID Durability on the mutations done to 'main' and make the change of state visible
        PSYNC();
        // After changing changing state to COPYING all applied mutativeFunc are visible and persisted
        for (int i = 0; i < maxTid; i++) {
            if (lfc[i] == nullptr) continue;
            fc[i*CLPAD].store(nullptr, std::memory_order_release);
        }
        // Apply log, copying data from 'main' to 'back'
        if (logEnabled) {
            apply_log(main_addr, back_addr);
            apply_pwb(back_addr);
        } else {
            copyMainToBack();
            logEnabled = true;
        }
        clear_log();
        log_size = 0;

        PFENCE();
        per->state.store(IDLE, std::memory_order_relaxed);
        if(histoflag) histo[storecount]++;
        rwlock.exclusiveUnlock();
        --tl_nested_write_trans;
    }

    // Non-static thread-safe read-only transaction
    template<class Func>
    void ns_read_transaction(Func&& readFunc) {
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




    // static thread-safe read-only transaction
    static void begin_read_transaction() {
        int tid = ThreadRegistry::getTID();
        gRomLog.rwlock.sharedLock(tid);
    }

    // static thread-safe read-only transaction
    static void end_read_transaction() {
        int tid = ThreadRegistry::getTID();
        gRomLog.rwlock.sharedUnlock(tid);
    }

    /*
     * Allocator
     * This method calls Doug Lea's allocator to get a piece of persistent
     * memory large enough to hold T, and then calls the constructor of T for
     * that memory region.
     */
    template <typename T, typename... Args>
    static T* alloc(Args&&... args) {
    	//if(histoflag) histoOn = true;
        const RomulusLog& r = gRomLog;
#ifdef USE_ESLOCO
        void* addr = r.esloco->malloc(sizeof(T));
        assert(addr != nullptr);
#else
        void* addr = mspace_malloc(r.per->ms, sizeof(T));
        assert(addr != 0);
#endif
        T* ptr = new (addr) T(std::forward<Args>(args)...); // placement new
        if (r.per->used_size < (uint8_t*)addr - r.main_addr + sizeof(T)+128) {
            r.per->used_size = (uint8_t*)addr - r.main_addr + sizeof(T)+128;
            PWB(&r.per->used_size);
        }
        //if(histoflag) histoOn = false;
        return ptr;
    }


    /*
     * De-allocator
     * Calls destructor of T and then reclaims the memory using Doug Lea's free
     */
    template<typename T>
    static void free(T* obj) {
        if (obj == nullptr) return;
        //if(histoflag) histoOn =true;
        obj->~T();
#ifdef USE_ESLOCO
        gRomLog.esloco->free(obj);
#else
        mspace_free(gRomLog.per->ms,obj);
#endif
        //if(histoflag) histoOn =false;
    }


    /* Allocator for C methods (like memcached) */
    static void* pmalloc(size_t size) {
    	//if(histoflag) histoOn =true;
        const RomulusLog& r = gRomLog;
#ifdef USE_ESLOCO
        void* addr = r.esloco->malloc(size);
        assert(addr != nullptr);
#else
        void* addr = mspace_malloc(r.per->ms, size);
        assert(addr != 0);
#endif
        if (r.per->used_size < (uint8_t*)addr - r.main_addr + size + 128) {
            r.per->used_size = (uint8_t*)addr - r.main_addr + size + 128;
            PWB(&r.per->used_size);
        }
        //if(histoflag) histoOn =false;
        return addr;
    }


    /* De-allocator for C methods (like memcached) */
    static void pfree(void* ptr) {
    	//if(histoflag) histoOn =true;
#ifdef USE_ESLOCO
        gRomLog.esloco->free(ptr);
#else
        mspace_free(gRomLog.per->ms,ptr);
#endif
    	//if(histoflag) histoOn =false;
    }

    static void init() {
    	gRomLog.ns_init();
    }

    template<class F> static void read_transaction(F&& func) { gRomLog.ns_read_transaction(func); }
    template<class F> static void write_transaction(F&& func) { gRomLog.ns_write_transaction(func); }
    //template<typename R, typename F> static R readTx(F&& func) { return gRomLog.ns_read_transaction(func); }
    template<typename F> static void readTx(F&& func) { gRomLog.ns_read_transaction(func); }

    /*
     * Thread-safe. Compares the contents of 'main' and 'back'.
     * This method MUST be called outside a transaction.
     */
    static bool consistency_check(void) {
        if (tl_nested_write_trans > 0) {
            printf("Warning: don't call consistency_check() inside a transaction\n");
        } else {
            while (!gRomLog.rwlock.tryExclusiveLock()) std::this_thread::yield();
            gRomLog.compareMainAndBack();
            gRomLog.rwlock.exclusiveUnlock();
        }
        return true;
    }
};



/*
 * Definition of persist<> type for RomulusLog.
 * In RomulusLog we need to interpose the stores and add them to the log, see pstore().
 */
template<typename T>
struct persist {
    // Stores the actual value
    T val;

    persist() { }

    persist(T initVal) {
        pstore(initVal);
    }

    // Casting operator
    operator T() {
        return pload();
    }

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

    // Equals operator: first downcast to T and then compare
    bool operator == (const T& otherval) const {
        return pload() == otherval;
    }

    // Difference operator: first downcast to T and then compare
    bool operator != (const T& otherval) const {
        return pload() != otherval;
    }

    // Relational operators
    bool operator < (const T& rhs) {
        return pload() < rhs;
    }
    bool operator > (const T& rhs) {
        return pload() > rhs;
    }
    bool operator <= (const T& rhs) {
        return pload() <= rhs;
    }
    bool operator >= (const T& rhs) {
        return pload() >= rhs;
    }

    T operator % (const T& rhs) {
        return pload() % rhs;
    }

    // Operator arrow ->
    T operator->() {
        return pload();
    }

    // Operator &
    T* operator&() {
        return &val;
    }

    // Copy constructor
    persist<T>(const persist<T>& other) {
        pstore(other.pload());
    }

    // Assignment operator
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
        const uint8_t* valaddr = (uint8_t*)&val;
        if (valaddr >= g_main_addr && valaddr < g_main_addr+g_main_size) gRomLog.add_to_log(&val,sizeof(T));
    }

    inline T pload() const {
        return val;
    }
};

} // end of romuluslog namespace
#endif  // _ROMULUS_LOG_PERSISTENT_TRANSACTIONAL_MEMORY_
