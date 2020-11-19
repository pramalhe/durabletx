#ifndef _PTM_DB_INCLUDE_H_
#define _PTM_DB_INCLUDE_H_
// We define here all the conditionals for the PTMs we want to use in our DB.
// We could do all of this stuff with C++ templatization, but it's just simpler to use C macros. Both options are ugly.

#define INCLUDED_FROM_MULTIPLE_CPP
#include "../ptms/ptm.h"

/*
#if defined USE_ROMULUS_LOG
#include "../ptms/romuluslog/RomulusLog.hpp"
#define PTM_UPDATE_TX      romuluslog::RomulusLog::write_transaction
#define PTM_READ_TX        romuluslog::RomulusLog::read_transaction
#define PTM_ALLOC          romuluslog::RomulusLog::alloc
#define PTM_FREE           romuluslog::RomulusLog::free
#define PTM_NEW            romuluslog::RomulusLog::alloc
#define PTM_DELETE         romuluslog::RomulusLog::free
#define PTM_GET_ROOT       romuluslog::RomulusLog::get_object
#define PTM_PUT_ROOT       romuluslog::RomulusLog::put_object
#define TM_ALLOC           romuluslog::RomulusLog::alloc
#define TM_FREE            romuluslog::RomulusLog::free
#define TM_PMALLOC         romuluslog::RomulusLog::pmalloc
#define TM_PFREE           romuluslog::RomulusLog::pfree
#define TM_TYPE            romuluslog::persist
#define TM_NAME            romuluslog::RomulusLog::className
#define PTM_FLUSH          romuluslog::RomulusLog::log_flush_range


#elif defined USE_TRINITY_VR_FC
#include "../ptms/trinity/TrinityVRFC.hpp"
using namespace trinityvrfc;
#define PTM_UPDATE_TX      Trinity::updateTx
#define PTM_READ_TX        Trinity::readTx
#define PTM_ALLOC          Trinity::tmNew
#define PTM_FREE           Trinity::tmDelete
#define PTM_NEW            Trinity::tmNew
#define PTM_DELETE         Trinity::tmDelete
#define PTM_GET_ROOT       Trinity::get_object
#define PTM_PUT_ROOT       Trinity::put_object
#define TM_PMALLOC         Trinity::pmalloc
#define TM_PFREE           Trinity::pfree
#define TM_TYPE            persist
#define TM_NAME            Trinity::className
#define PTM_FLUSH          doNothingPTMFlush


#elif defined USE_TRINITY_VR_TL2
#include "../ptms/trinity/TrinityVRTL2.hpp"
using namespace trinityvrtl2;
#define PTM_UPDATE_TX      Trinity::updateTx
#define PTM_READ_TX        Trinity::readTx
#define PTM_ALLOC          Trinity::tmNew
#define PTM_FREE           Trinity::tmDelete
#define PTM_NEW            Trinity::tmNew
#define PTM_DELETE         Trinity::tmDelete
#define PTM_GET_ROOT       Trinity::get_object
#define PTM_PUT_ROOT       Trinity::put_object
#define TM_PMALLOC         Trinity::pmalloc
#define TM_PFREE           Trinity::pfree
#define TM_TYPE            persist
#define TM_NAME            Trinity::className
#define PTM_FLUSH          doNothingPTMFlush

#elif PMDK_PTM
#include "pmdk/PMDKTM.hpp"
#define TM_WRITE_TRANSACTION   pmdk::PMDKTM::write_transaction
#define TM_READ_TRANSACTION    pmdk::PMDKTM::read_transaction
#define TM_ALLOC               pmdk::PMDKTM::alloc
#define TM_FREE                pmdk::PMDKTM::free
#define TM_PMALLOC             pmdk::PMDKTM::pmalloc
#define TM_PFREE               pmdk::PMDKTM::pfree
#define TM_TYPE                pmdk::persist
#define TM_NAME                pmdk::PMDKTM::className

#else
#error "PTM macros were undefined. You need a -DUSE_SOMETHING in the Makefile"
#endif
*/


#endif  // _PTM_DB_INCLUDE_H_
