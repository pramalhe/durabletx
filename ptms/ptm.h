#ifndef _PTM_INCLUDE_H_
#define _PTM_INCLUDE_H_
// We define here all the conditionals for the PTMs we want to use
// We could do all of this stuff with C++ templatization, but it's just simpler to use C macros. Both options are ugly.


#if defined USE_ROMULUS_LOG
#include "romuluslog/RomulusLog.hpp"
#define PTM_UPDATE_TX      romuluslog::RomulusLog::write_transaction
#define PTM_READ_TX        romuluslog::RomulusLog::read_transaction
#define PTM_NEW            romuluslog::RomulusLog::alloc
#define PTM_DELETE         romuluslog::RomulusLog::free
#define PTM_GET_ROOT       romuluslog::RomulusLog::get_object
#define PTM_PUT_ROOT       romuluslog::RomulusLog::put_object
#define PTM_TYPE           persist
#define PTM_NAME           romuluslog::RomulusLog::className

#elif defined USE_UNDO_LOG_FC
#include "undolog/UndoLogFC.hpp"
#define PTM_CLASS          undologfc::UndoLog
#define PTM_UPDATE_TX      undologfc::UndoLog::updateTx
#define PTM_READ_TX        undologfc::UndoLog::readTx
#define PTM_NEW            undologfc::UndoLog::tmNew
#define PTM_DELETE         undologfc::UndoLog::tmDelete
#define PTM_MALLOC         undologfc::UndoLog::tmMalloc
#define PTM_FREE           undologfc::UndoLog::tmFree
#define PTM_GET_ROOT       undologfc::UndoLog::get_object
#define PTM_PUT_ROOT       undologfc::UndoLog::put_object
#define PTM_TYPE           undologfc::persist
#define PTM_PERSIST        undologfc::persist
#define PTM_NAME           undologfc::UndoLog::className
#define PTM_FILEXT         "undologfc"

#elif defined USE_UNDO_LOG_SEQ_FC
#include "undolog/UndoLogSeqFC.hpp"
#define PTM_CLASS          undologseqfc::UndoLog
#define PTM_UPDATE_TX      undologseqfc::UndoLog::updateTx
#define PTM_READ_TX        undologseqfc::UndoLog::readTx
#define PTM_NEW            undologseqfc::UndoLog::tmNew
#define PTM_DELETE         undologseqfc::UndoLog::tmDelete
#define PTM_MALLOC         undologseqfc::UndoLog::tmMalloc
#define PTM_FREE           undologseqfc::UndoLog::tmFree
#define PTM_GET_ROOT       undologseqfc::UndoLog::get_object
#define PTM_PUT_ROOT       undologseqfc::UndoLog::put_object
#define PTM_TYPE           undologseqfc::persist
#define PTM_PERSIST        undologseqfc::persist
#define PTM_NAME           undologseqfc::UndoLog::className
#define PTM_FILEXT         "undologseqfc"

#elif defined USE_PMDK
#include "pmdk/PMDKTM.hpp"
#define PTM_CLASS          pmdk::PMDKTM
#define PTM_UPDATE_TX      pmdk::PMDKTM::updateTx
#define PTM_READ_TX        pmdk::PMDKTM::readTx
#define PTM_NEW            pmdk::PMDKTM::tmNew
#define PTM_DELETE         pmdk::PMDKTM::tmDelete
#define PTM_MALLOC         pmdk::PMDKTM::tmMalloc
#define PTM_FREE           pmdk::PMDKTM::tmFree
#define PTM_GET_ROOT       pmdk::PMDKTM::get_object
#define PTM_PUT_ROOT       pmdk::PMDKTM::put_object
#define PTM_TYPE           pmdk::persist
#define PTM_PERSIST        pmdk::persist
#define PTM_NAME           pmdk::PMDKTM::className
#define PTM_FILEXT         "pmdk"

#elif defined USE_REDO_LOG_FC
#include "redolog/RedoLogFC.hpp"
#define PTM_CLASS          redologfc::RedoLog
#define PTM_UPDATE_TX      redologfc::RedoLog::updateTx
#define PTM_READ_TX        redologfc::RedoLog::readTx
#define PTM_NEW            redologfc::RedoLog::tmNew
#define PTM_DELETE         redologfc::RedoLog::tmDelete
#define PTM_MALLOC         redologfc::RedoLog::tmMalloc
#define PTM_FREE           redologfc::RedoLog::tmFree
#define PTM_GET_ROOT       redologfc::RedoLog::get_object
#define PTM_PUT_ROOT       redologfc::RedoLog::put_object
#define PTM_TYPE           redologfc::persist
#define PTM_PERSIST        redologfc::persist
#define PTM_NAME           redologfc::RedoLog::className
#define PTM_FILEXT         "redologfc"


#elif defined USE_QUADRA_FC
#include "quadra/QuadraFC.hpp"
#define PTM_CLASS          quadrafc::Quadra
#define PTM_UPDATE_TX      quadrafc::Quadra::updateTx
#define PTM_READ_TX        quadrafc::Quadra::readTx
#define PTM_NEW            quadrafc::Quadra::tmNew
#define PTM_DELETE         quadrafc::Quadra::tmDelete
#define PTM_MALLOC         quadrafc::Quadra::tmMalloc
#define PTM_FREE           quadrafc::Quadra::tmFree
#define PTM_GET_ROOT       quadrafc::Quadra::get_object
#define PTM_PUT_ROOT       quadrafc::Quadra::put_object
#define PTM_TYPE           quadrafc::persist
#define PTM_PERSIST        quadrafc::persist
#define PTM_NAME           quadrafc::Quadra::className
#define PTM_FILEXT         "quadrafc"

#elif defined USE_QUADRA_VR_FC
#include "quadra/QuadraVRFC.hpp"
#define PTM_CLASS          quadravrfc::Quadra
#define PTM_UPDATE_TX      quadravrfc::Quadra::updateTx
#define PTM_READ_TX        quadravrfc::Quadra::readTx
#define PTM_NEW            quadravrfc::Quadra::tmNew
#define PTM_DELETE         quadravrfc::Quadra::tmDelete
#define PTM_MALLOC         quadravrfc::Quadra::tmMalloc
#define PTM_FREE           quadravrfc::Quadra::tmFree
#define PTM_GET_ROOT       quadravrfc::Quadra::get_object
#define PTM_PUT_ROOT       quadravrfc::Quadra::put_object
#define PTM_TYPE           quadravrfc::persist
#define PTM_PERSIST        quadravrfc::persist
#define PTM_NAME           quadravrfc::Quadra::className
#define PTM_FILEXT         "quadravrfc"


#elif defined USE_TRINITY_FC
#include "trinity/TrinityFC.hpp"
#define PTM_CLASS          trinityfc::Trinity
#define PTM_UPDATE_TX      trinityfc::Trinity::updateTx
#define PTM_READ_TX        trinityfc::Trinity::readTx
#define PTM_NEW            trinityfc::Trinity::tmNew
#define PTM_DELETE         trinityfc::Trinity::tmDelete
#define PTM_MALLOC         trinityfc::Trinity::tmMalloc
#define PTM_FREE           trinityfc::Trinity::tmFree
#define PTM_GET_ROOT       trinityfc::Trinity::get_object
#define PTM_PUT_ROOT       trinityfc::Trinity::put_object
#define PTM_TYPE           trinityfc::persist
#define PTM_PERSIST        trinityfc::persist
#define PTM_NAME           trinityfc::Trinity::className
#define PTM_FILEXT         "trinfc"

#elif defined USE_TRINITY_TL2
#include "trinity/TrinityTL2.hpp"
#define PTM_CLASS          trinitytl2::Trinity
#define PTM_UPDATE_TX      trinitytl2::Trinity::updateTx
#define PTM_READ_TX        trinitytl2::Trinity::readTx
#define PTM_NEW            trinitytl2::Trinity::tmNew
#define PTM_DELETE         trinitytl2::Trinity::tmDelete
#define PTM_MALLOC         trinitytl2::Trinity::tmMalloc
#define PTM_FREE           trinitytl2::Trinity::tmFree
#define PTM_GET_ROOT       trinitytl2::Trinity::get_object
#define PTM_PUT_ROOT       trinitytl2::Trinity::put_object
#define PTM_TYPE           trinitytl2::persist
#define PTM_PERSIST        trinitytl2::persist
#define PTM_NAME           trinitytl2::Trinity::className
#define PTM_FILEXT         "trintl2"

#elif defined USE_TRINITY_VR_FC
#include "trinity/TrinityVRFC.hpp"
#define PTM_CLASS          trinityvrfc::Trinity
#define PTM_UPDATE_TX      trinityvrfc::Trinity::updateTx
#define PTM_READ_TX        trinityvrfc::Trinity::readTx
#define PTM_NEW            trinityvrfc::Trinity::tmNew
#define PTM_DELETE         trinityvrfc::Trinity::tmDelete
#define PTM_MALLOC         trinityvrfc::Trinity::tmMalloc
#define PTM_FREE           trinityvrfc::Trinity::tmFree
#define PTM_GET_ROOT       trinityvrfc::Trinity::get_object
#define PTM_PUT_ROOT       trinityvrfc::Trinity::put_object
#define PTM_TYPE           trinityvrfc::persist
#define PTM_PERSIST        trinityvrfc::persist
#define PTM_BASE           trinityvrfc::tmbase
#define PTM_NAME           trinityvrfc::Trinity::className
#define PTM_FILEXT         "trinvrfc"
#define PTM_MEMCPY         trinityvrfc::Trinity::tmMemcpy
#define PTM_MEMCMP         trinityvrfc::Trinity::tmMemcmp
#define PTM_STRCMP         trinityvrfc::Trinity::tmStrcmp
#define PTM_MEMSET         trinityvrfc::Trinity::tmMemset
#define PTM_STRLEN         trinityvrfc::Trinity::tmStrlen
#define PTM_MEMCPY         trinityvrfc::Trinity::tmMemcpy



#elif defined USE_TRINITY_VR_TL2
#include "trinity/TrinityVRTL2.hpp"
#define PTM_CLASS          trinityvrtl2::Trinity
#define PTM_UPDATE_TX      trinityvrtl2::Trinity::updateTx
#define PTM_READ_TX        trinityvrtl2::Trinity::readTx
#define PTM_NEW            trinityvrtl2::Trinity::tmNew
#define PTM_DELETE         trinityvrtl2::Trinity::tmDelete
#define PTM_MALLOC         trinityvrtl2::Trinity::tmMalloc
#define PTM_FREE           trinityvrtl2::Trinity::tmFree
#define PTM_GET_ROOT       trinityvrtl2::Trinity::get_object
#define PTM_PUT_ROOT       trinityvrtl2::Trinity::put_object
#define PTM_TYPE           trinityvrtl2::persist
#define PTM_PERSIST        trinityvrtl2::persist
#define PTM_BASE           trinityvrtl2::tmbase
#define PTM_NAME           trinityvrtl2::Trinity::className
#define PTM_FILEXT         "trinvrtl2"
#define PTM_MEMCPY         trinityvrtl2::Trinity::tmMemcpy
#define PTM_MEMCMP         trinityvrtl2::Trinity::tmMemcmp
#define PTM_STRCMP         trinityvrtl2::Trinity::tmStrcmp
#define PTM_MEMSET         trinityvrtl2::Trinity::tmMemset
#define PTM_STRLEN         trinityvrtl2::Trinity::tmStrlen
#define PTM_MEMCPY         trinityvrtl2::Trinity::tmMemcpy

#elif defined USE_TRINITY_VR_TL2_PL   // Persistent Locks
#include "trinity/TrinityVRTL2PL.hpp"
#define PTM_CLASS          trinityvrtl2pl::Trinity
#define PTM_UPDATE_TX      trinityvrtl2pl::Trinity::updateTx
#define PTM_READ_TX        trinityvrtl2pl::Trinity::readTx
#define PTM_NEW            trinityvrtl2pl::Trinity::tmNew
#define PTM_DELETE         trinityvrtl2pl::Trinity::tmDelete
#define PTM_MALLOC         trinityvrtl2pl::Trinity::tmMalloc
#define PTM_FREE           trinityvrtl2pl::Trinity::tmFree
#define PTM_GET_ROOT       trinityvrtl2pl::Trinity::get_object
#define PTM_PUT_ROOT       trinityvrtl2pl::Trinity::put_object
#define PTM_TYPE           trinityvrtl2pl::persist
#define PTM_PERSIST        trinityvrtl2pl::persist
#define PTM_NAME           trinityvrtl2pl::Trinity::className
#define PTM_FILEXT         "trinvrtl2pl"

#elif defined USE_DUO_VR_FC
#include "duo/DuoVRFC.hpp"
#define PTM_CLASS          duovrfc::Duo
#define PTM_UPDATE_TX      duovrfc::Duo::updateTx
#define PTM_READ_TX        duovrfc::Duo::readTx
#define PTM_NEW            duovrfc::Duo::tmNew
#define PTM_DELETE         duovrfc::Duo::tmDelete
#define PTM_MALLOC         duovrfc::Duo::tmMalloc
#define PTM_FREE           duovrfc::Duo::tmFree
#define PTM_GET_ROOT       duovrfc::Duo::get_object
#define PTM_PUT_ROOT       duovrfc::Duo::put_object
#define PTM_TYPE           duovrfc::persist
#define PTM_PERSIST        duovrfc::persist
#define PTM_BASE           duovrfc::tmbase
#define PTM_NAME           duovrfc::Duo::className
#define PTM_FILEXT         "duovrfc"
#define PTM_MEMCPY         duovrfc::Duo::tmMemcpy
#define PTM_MEMCMP         duovrfc::Duo::tmMemcmp
#define PTM_STRCMP         duovrfc::Duo::tmStrcmp
#define PTM_MEMSET         duovrfc::Duo::tmMemset
#define PTM_STRLEN         duovrfc::Duo::tmStrlen
#define PTM_MEMCPY         duovrfc::Duo::tmMemcpy

//
// Dual-Zone variants
//
#elif defined USE_DZ_V1
#include "dualzone/DualZoneV1.hpp"
#define PTM_CLASS          dzv1::DualZone
#define PTM_UPDATE_TX      dzv1::DualZone::updateTx
#define PTM_READ_TX        dzv1::DualZone::readTx
#define PTM_NEW            dzv1::DualZone::tmNew
#define PTM_DELETE         dzv1::DualZone::tmDelete
#define PTM_MALLOC         dzv1::DualZone::tmMalloc
#define PTM_FREE           dzv1::DualZone::tmFree
#define PTM_GET_ROOT       dzv1::DualZone::get_object
#define PTM_PUT_ROOT       dzv1::DualZone::put_object
#define PTM_TYPE           dzv1::persist
#define PTM_PERSIST        dzv1::persist
#define PTM_NAME           dzv1::DualZone::className
#define PTM_FILEXT         "dzv1"

#elif defined USE_DZ_V2
#include "dualzone/DualZoneV2.hpp"
#define PTM_CLASS          dzv2::DualZone
#define PTM_UPDATE_TX      dzv2::DualZone::updateTx
#define PTM_READ_TX        dzv2::DualZone::readTx
#define PTM_NEW            dzv2::DualZone::tmNew
#define PTM_DELETE         dzv2::DualZone::tmDelete
#define PTM_MALLOC         dzv2::DualZone::tmMalloc
#define PTM_FREE           dzv2::DualZone::tmFree
#define PTM_GET_ROOT       dzv2::DualZone::get_object
#define PTM_PUT_ROOT       dzv2::DualZone::put_object
#define PTM_TYPE           dzv2::persist
#define PTM_PERSIST        dzv2::persist
#define PTM_NAME           dzv2::DualZone::className
#define PTM_FILEXT         "dzv2"

#elif defined USE_DZ_V3
#include "dualzone/DualZoneV3.hpp"
#define PTM_CLASS          dzv3::DualZone
#define PTM_UPDATE_TX      dzv3::DualZone::updateTx
#define PTM_READ_TX        dzv3::DualZone::readTx
#define PTM_NEW            dzv3::DualZone::tmNew
#define PTM_DELETE         dzv3::DualZone::tmDelete
#define PTM_MALLOC         dzv3::DualZone::tmMalloc
#define PTM_FREE           dzv3::DualZone::tmFree
#define PTM_GET_ROOT       dzv3::DualZone::get_object
#define PTM_PUT_ROOT       dzv3::DualZone::put_object
#define PTM_TYPE           dzv3::persist
#define PTM_PERSIST        dzv3::persist
#define PTM_NAME           dzv3::DualZone::className
#define PTM_FILEXT         "dzv3"

#elif defined USE_DZ_V4
#include "dualzone/DualZoneV4.hpp"
#define PTM_CLASS          dzv4::DualZone
#define PTM_UPDATE_TX      dzv4::DualZone::updateTx
#define PTM_READ_TX        dzv4::DualZone::readTx
#define PTM_NEW            dzv4::DualZone::tmNew
#define PTM_DELETE         dzv4::DualZone::tmDelete
#define PTM_MALLOC         dzv4::DualZone::tmMalloc
#define PTM_FREE           dzv4::DualZone::tmFree
#define PTM_GET_ROOT       dzv4::DualZone::get_object
#define PTM_PUT_ROOT       dzv4::DualZone::put_object
#define PTM_TYPE           dzv4::persist
#define PTM_PERSIST        dzv4::persist
#define PTM_NAME           dzv4::DualZone::className
#define PTM_FILEXT         "dzv4"

#elif defined USE_OFLF
#include "onefile/OneFilePTMLF.hpp"
#define PTM_CLASS          poflf::OneFileLF
#define PTM_UPDATE_TX      poflf::OneFileLF::updateTx
#define PTM_READ_TX        poflf::OneFileLF::readTx
#define PTM_NEW            poflf::OneFileLF::tmNew
#define PTM_DELETE         poflf::OneFileLF::tmDelete
#define PTM_MALLOC         poflf::OneFileLF::tmMalloc
#define PTM_FREE           poflf::OneFileLF::tmFree
#define PTM_GET_ROOT       poflf::OneFileLF::get_object
#define PTM_PUT_ROOT       poflf::OneFileLF::put_object
#define PTM_TYPE           poflf::tmtype
#define PTM_PERSIST        poflf::tmtype
#define PTM_NAME           poflf::OneFileLF::className
#define PTM_FILEXT         "oflf"


#endif



#endif  // _PTM_INCLUDE_H_
