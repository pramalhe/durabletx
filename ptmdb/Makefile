CXX=g++-9
BIN=bin
# For testing on laptop
#CXXFLAGS= -std=c++14 -g -O2 -DPWB_IS_NOP
# for using on Optane DC PM
CXXFLAGS = -std=c++14 -g -O2 -DPWB_IS_CLWB -DPM_REGION_SIZE=32*1024*1024*1024ULL -DPM_FILE_NAME="\"/mnt/pmem0/durable\""

# Possible options for PWB are:
# -DPWB_IS_CLFLUSH		pwb is a CLFLUSH and pfence/psync are nops      (Broadwell)
# -DPWB_IS_CLFLUSHOPT	pwb is a CLFLUSHOPT and pfence/psync are SFENCE (Kaby Lake) 
# -DPWB_IS_CLWB			pwb is a CLWB and pfence/psync are SFENCE       (Sky Lake SP, or Canon Lake SP and beyond)
# -DPWB_IS_NOP			pwb/pfence/psync are nops. Used for shared memory persistence

all: \
	persistencyclean \
	bin/db_bench_trinvrfc \
	bin/db_bench_trinvrtl2 \
	bin/ycsb_trinvrfc \
	bin/ycsb_trinvrtl2 \
	bin/ycsb_redodb \
	bin/ycsb_pmemkv \
	bin/ycsb_generator \

MYDEPS = \
	../ptms/trinity/TrinityVRFC.hpp \
	../ptms/trinity/TrinityVRTL2.hpp \
	ptmcpp/TrinityVRFC.cpp \
	ptmcpp/TrinityVRTL2.cpp \
	db.h \
	iterator.h \
	options.h \
	slice.h \
	status.h \
	TMHashMap.hpp \
	TMHashMapIterator.h \
	TMBTreeMap.hpp \
	TMBTreeMapIterator.h \
	db_impl.cc \
	db_impl.h \
	status.cc \
	db_bench.cc \
	ptmdb.h \
	util/random.h \
	util/histogram.cc \
	util/histogram.h \
	util/mutexlock.h \
	util/testutil.cc \
	port/port_posix.cc \
	write_batch.cc \
	write_batch.h \
	

SRCS = \
	db_impl.cc \
	status.cc \
	port/port_posix.cc \
	util/histogram.cc \
	util/env_posix.cc \
	util/testutil.cc \
	write_batch.cc \
	

REDODB_SRCS = \
	otherdb/redodb/db_impl.cc \
	otherdb/redodb/status.cc \
	otherdb/redodb/port/port_posix.cc \
	otherdb/redodb/util/histogram.cc \
	otherdb/redodb/util/env_posix.cc \
	otherdb/redodb/util/testutil.cc \
	otherdb/redodb/write_batch.cc \
	otherdb/redodb/common/ThreadRegistry.cpp \
	otherdb/redodb/ptms/redoopt/RedoOpt.cpp \

#
# unit tests
#
bin/ut2_romlog: $(MYDEPS) $(DSDEPS) examples/ut2.cpp
	$(CXX) $(CXXFLAGS) -DUSE_ROMULUS_LOG ut2.cpp $(SRCS) $(ROMLOG_SRCS) -I. -Iutil -Iport -o bin/ut2_romlog -lpthread

bin/ut2_trinvrfc: $(MYDEPS) $(DSDEPS) examples/ut2.cpp
	$(CXX) $(CXXFLAGS) -DUSE_TRINITY_VR_FC ptmcpp/TrinityVRFC.cpp examples/ut2.cpp $(SRCS) -I. -Iutil -Iport -o bin/ut2_trinvrfc -lpthread

bin/ut2_trinvrtl2: $(MYDEPS) $(DSDEPS) examples/ut2.cpp
	$(CXX) $(CXXFLAGS) -DUSE_TRINITY_VR_TL2 ptmcpp/TrinityVRTL2.cpp examples/ut2.cpp $(SRCS) -I. -Iutil -Iport -o bin/ut2_trinvrtl2 -lpthread



# Recovery
bin/recover_rocksdb: examples/recover_rocksdb.cpp
	$(CXX) $(CXXFLAGS) examples/recover_rocksdb.cpp -I~/rocksdb/include -o bin/recover_rocksdb -L~/rocksdb/ -lrocksdb -lpthread -lrt -lsnappy -lgflags -lz -lbz2 -llz4 -lzstd -lnuma -ltbb

bin/recover_redodb: examples/recover_redodb.cpp
	$(CXX) $(CXXFLAGS) -DUSE_REDOOPT examples/recover_redodb.cpp $(SRCS) $(REDOOPT_SRCS) -I. -Iutil -Iport -o bin/db_bench_redoopt -lpthread -o bin/recover_redodb


#
# Benchmarks based on leveldb
#
bin/db_bench_trinvrfc: $(MYDEPS) $(DSDEPS) db_bench.cc 
	$(CXX) $(CXXFLAGS) -DUSE_TRINITY_VR_FC ptmcpp/TrinityVRFC.cpp db_bench.cc $(SRCS) -I. -Iutil -Iport -o bin/db_bench_trinvrfc -lpthread

bin/db_bench_trinvrtl2: $(MYDEPS) $(DSDEPS) db_bench.cc 
	$(CXX) $(CXXFLAGS) -DUSE_TRINITY_VR_TL2 ptmcpp/TrinityVRTL2.cpp db_bench.cc $(SRCS) -I. -Iutil -Iport -o bin/db_bench_trinvrtl2 -lpthread

#
# YCSB taken from PRONTO and https://github.com/YuNanlong/YCSB-CPP
#
bin/ycsb_generator: $(MYDEPS) $(DSDEPS) ycsb/ycsb_generator.cpp ycsb/generator.h ycsb/workload.h ycsb/zipf.h
	$(CXX) $(CXXFLAGS) ycsb/ycsb_generator.cpp -o bin/ycsb_generator -lpthread

bin/ycsb_trinvrfc: $(MYDEPS) $(DSDEPS) ycsb/ycsb.cpp
	$(CXX) $(CXXFLAGS) -DUSE_TRINITY_VR_FC ptmcpp/TrinityVRFC.cpp ycsb/ycsb.cpp $(SRCS) -I. -Iutil -Iport -o bin/ycsb_trinvrfc -lpthread
	
bin/ycsb_trinvrtl2: $(MYDEPS) $(DSDEPS) ycsb/ycsb.cpp
	$(CXX) $(CXXFLAGS) -DUSE_TRINITY_VR_TL2 ptmcpp/TrinityVRTL2.cpp ycsb/ycsb.cpp $(SRCS) -I. -Iutil -Iport -o bin/ycsb_trinvrtl2 -lpthread
	
bin/ycsb_redodb: $(MYDEPS) $(DSDEPS) ycsb/ycsb.cpp
	$(CXX) $(CXXFLAGS) -DUSE_REDOOPT ycsb/ycsb.cpp $(REDODB_SRCS) -Iotherdb/redodb/ -Iotherdb/redodb/util -Iotherdb/redodb/port -o bin/ycsb_redodb -lpthread
	
bin/ycsb_pmemkv: $(MYDEPS) $(DSDEPS) ycsb/ycsb.cpp
	$(CXX) $(CXXFLAGS) -DUSE_PMEMKV ycsb/ycsb.cpp -o bin/ycsb_pmemkv -lpthread -lpmemkv


persistencyclean:
	rm -f /dev/shm/*_shared
	rm -f /dev/shm/*_shared_main
	rm -f /mnt/pmem0/durable
	rm -rf /mnt/pmem0/rocksdb/
	rm -f /mnt/pmem0/pmemkv


# We also clean the persistency files so that on a new build we get a clean slate
clean: persistencyclean
	rm -f bin/*
		
