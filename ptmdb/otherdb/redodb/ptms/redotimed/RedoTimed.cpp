/*
 * Copyright 2017-2020
 *   Andreia Correia <andreia.veiga@unine.ch>
 *   Pedro Ramalhete <pramalhe@gmail.com>
 *   Pascal Felber <pascal.felber@unine.ch>
 *
 * This work is published under the MIT license. See LICENSE.txt
 */
#include "RedoTimed.hpp"


namespace redotimed {

// Global with the 'main' size. Used by pload()
uint64_t g_main_size = 0;
// Global with the 'main' addr. Used by pload()
uint8_t* g_main_addr = 0;
uint8_t* g_main_addr_end;
uint8_t* g_region_end;

// Counter of nested write transactions
thread_local int64_t tl_nested_write_trans = 0;
// Counter of nested read-only transactions
thread_local int64_t tl_nested_read_trans = 0;
// Global instance
RedoTimed gRedo {};

//thread_local void* st;
//thread_local uint64_t tl_cx_size = 0;

// Log of the deferred PWBs
thread_local void* tl_pwb_log[PWB_LOG_SIZE];
//thread_local int tl_pwb_idx = 0;

thread_local varLocal tlocal;

} // End of cxpuc namespace
