CXX = g++-9
#CXXFLAGS = -std=c++14 -g -O2 -DPWB_IS_CLFLUSHOPT
# Use these options for Optane DC PM:
CXXFLAGS = -std=c++14 -g -O2 -DPWB_IS_CLWB -DPM_REGION_SIZE=4*1024*1024*1024ULL -DPM_FILE_NAME="\"/mnt/pmem0/durable\""    
#CXXFLAGS += -fuse-ld=gold  -fsanitize=address
# Possible options for PWB are:
# -DPWB_IS_CLFLUSH		pwb is a CLFLUSH and pfence/psync are nops
# -DPWB_IS_CLFLUSHOPT	pwb is a CLFLUSHOPT and pfence/psync are SFENCE 
# -DPWB_IS_CLWB			pwb is a CLWB and pfence/psync are SFENCE
# -DPWB_IS_NOP			pwb/pfence/psync are nops

INCLUDES = -I../

# This library is needed for PMDK
PMDKLIBS=-I../../libpmemobj-cpp/include/ -L/usr/local/lib -lpmemobj

# We make different binaries, one for every different PTM, so that we can re-use the same mmap region.
# PTMs for which we want to build concurrent benchmarks
CONC_PTM_NAMES := \
	bin-trinityfc \
	bin-quadrafc \
	bin-trinityvrfc \
	bin-quadravrfc \
	bin-trinitytl2 \
	bin-trinityvrtl2 \
	bin-romlogfc \
	bin-oflf \
	bin-pmdk \
	

# The following binaries will be built for the PTMs selected above	
BINARIES_SEQ = \
	$(subst bin-, bin/pq-ll-,                $(CONC_PTM_NAMES)) \
	$(subst bin-, bin/pset-tree-1m-,         $(CONC_PTM_NAMES)) \
	$(subst bin-, bin/pset-btree-1m-,        $(CONC_PTM_NAMES)) \
	$(subst bin-, bin/pset-ziptree-1m-,      $(CONC_PTM_NAMES)) \

	
#
# Dependencies for each PTM
#
COMMON_DEP = \
	../common/ThreadRegistry.hpp \
	../common/EsLoco.hpp \
	../common/StrongTryRIRWLock.hpp \

PQUEUES_DEP = \
	../pdatastructures/pqueues/MichaelScottQueue.hpp \
	../pdatastructures/pqueues/PFriedmanQueue.hpp \
	../pdatastructures/pqueues/PMDKLinkedListQueue.hpp \
	../pdatastructures/pqueues/PMichaelScottQueue.hpp \
	../pdatastructures/TMLinkedListQueue.hpp \
	../pdatastructures/pqueues/POFLFLinkedListQueue.hpp \
	../pdatastructures/pqueues/POFWFLinkedListQueue.hpp \
	../pdatastructures/pqueues/RomLogLinkedListQueue.hpp \
	../pdatastructures/pqueues/RomLRLinkedListQueue.hpp \
	
ROMULUS_LIB_SRC = \
	../common/ThreadRegistry.cpp \
	../ptms/romuluslog/malloc.cpp \
	../ptms/romuluslog/RomulusLog.cpp \
	../ptms/romuluslr/malloc.cpp \
	../ptms/romuluslr/RomulusLR.cpp \
	
ROMULUS_LIB_DEP = \
	$(ROMULUS_LIB_SRC) \
	../ptms/romuluslog/RomulusLog.hpp \
	../ptms/romuluslr/RomulusLR.hpp \

all: $(BINARIES_SEQ) persistencyclean

.PHONY : clean persistencyclean

clean: persistencyclean
	rm -f bin/*
	rm -f lib/*

persistencyclean:
	rm -f /mnt/pmem0/durable
	rm -f /dev/shm/*_shared
	rm -f /dev/shm/*_shared_main
	rm -f /dev/shm/*_shared_back
	rm -f /dev/shm/psegments/*
	rm -f /dev/shm/pmdk_shared
	rm -f /dev/shm/pmdk_shared_friedman	




#
#
# Multithreaded benchmarks
#
#

#
# Multithreaded SPS
#
bin/psps-integer-quadrafc: psps-integer.cpp PBenchmarkSPS.hpp ../ptms/quadra/QuadraFC.hpp
	$(CXX) $(CXXFLAGS) -DUSE_QUADRA_FC $(INCLUDES) psps-integer.cpp -o bin/psps-integer-quadrafc -lpthread

bin/psps-integer-quadravrfc: psps-integer.cpp PBenchmarkSPS.hpp ../ptms/quadra/QuadraVRFC.hpp
	$(CXX) $(CXXFLAGS) -DUSE_QUADRA_VR_FC $(INCLUDES) psps-integer.cpp -o bin/psps-integer-quadravrfc -lpthread

bin/psps-integer-trinityfc: psps-integer.cpp PBenchmarkSPS.hpp ../ptms/trinity/TrinityFC.hpp
	$(CXX) $(CXXFLAGS) -DUSE_TRINITY_FC $(INCLUDES) psps-integer.cpp -o bin/psps-integer-trinityfc -lpthread

bin/psps-integer-trinityvrfc: psps-integer.cpp PBenchmarkSPS.hpp ../ptms/trinity/TrinityVRFC.hpp
	$(CXX) $(CXXFLAGS) -DUSE_TRINITY_VR_FC $(INCLUDES) psps-integer.cpp -o bin/psps-integer-trinityvrfc -lpthread

bin/psps-integer-trinitytl2: psps-integer.cpp PBenchmarkSPS.hpp ../ptms/trinity/TrinityTL2.hpp
	$(CXX) $(CXXFLAGS) -DUSE_TRINITY_TL2 $(INCLUDES) psps-integer.cpp -o bin/psps-integer-trinitytl2 -lpthread

bin/psps-integer-trinityvrtl2: psps-integer.cpp PBenchmarkSPS.hpp ../ptms/trinity/TrinityVRTL2.hpp
	$(CXX) $(CXXFLAGS) -DUSE_TRINITY_VR_TL2 $(INCLUDES) psps-integer.cpp -o bin/psps-integer-trinityvrtl2 -lpthread

bin/psps-integer-trinityvrtl2pl: psps-integer.cpp PBenchmarkSPS.hpp ../ptms/trinity/TrinityVRTL2PL.hpp
	$(CXX) $(CXXFLAGS) -DUSE_TRINITY_VR_TL2_PL $(INCLUDES) psps-integer.cpp -o bin/psps-integer-trinityvrtl2pl -lpthread

bin/psps-integer-duovrfc: psps-integer.cpp PBenchmarkSPS.hpp ../ptms/duo/DuoVRFC.hpp
	$(CXX) $(CXXFLAGS) -DUSE_DUO_VR_FC $(INCLUDES) psps-integer.cpp -o bin/psps-integer-duovrfc -lpthread

bin/psps-integer-undologfc: psps-integer.cpp PBenchmarkSPS.hpp ../ptms/undolog/UndoLogFC.hpp
	$(CXX) $(CXXFLAGS) -DUSE_UNDO_LOG_FC $(INCLUDES) psps-integer.cpp -o bin/psps-integer-undologfc -lpthread

bin/psps-integer-redologfc: psps-integer.cpp PBenchmarkSPS.hpp ../ptms/redolog/RedoLogFC.hpp
	$(CXX) $(CXXFLAGS) -DUSE_REDO_LOG_FC $(INCLUDES) psps-integer.cpp -o bin/psps-integer-redologfc -lpthread

bin/psps-integer-romlogfc: psps-integer.cpp PBenchmarkSPS.hpp ../ptms/romuluslog/RomLogFC.hpp
	$(CXX) $(CXXFLAGS) -DUSE_ROM_LOG_FC $(INCLUDES) psps-integer.cpp -o bin/psps-integer-romlogfc -lpthread

bin/psps-integer-pmdk: psps-integer.cpp PBenchmarkSPS.hpp ../ptms/pmdk/PMDKTM.hpp
	$(CXX) $(CXXFLAGS) -DUSE_PMDK $(INCLUDES) psps-integer.cpp -o bin/psps-integer-pmdk -lpthread -lpmemobj
	
bin/psps-integer-oflf: psps-integer.cpp PBenchmarkSPS.hpp ../ptms/onefile/OneFilePTMLF.hpp
	$(CXX) $(CXXFLAGS) -DUSE_OFLF $(INCLUDES) psps-integer.cpp -o bin/psps-integer-oflf -lpthread

#
# Multithreaded queue
#
bin/pq-ll-romlogfc: pq-ll.cpp PBenchmarkQueues.hpp ../pdatastructures/queues/TMLinkedListQueueByRef.hpp ../ptms/romuluslog/RomLogFC.hpp
	$(CXX) $(CXXFLAGS) -DUSE_ROM_LOG_FC $(INCLUDES) pq-ll.cpp -o bin/pq-ll-romlogfc -lpthread

bin/pq-ll-undologfc: pq-ll.cpp PBenchmarkQueues.hpp ../pdatastructures/queues/TMLinkedListQueueByRef.hpp ../ptms/undolog/UndoLogFC.hpp
	$(CXX) $(CXXFLAGS) -DUSE_UNDO_LOG_FC $(INCLUDES) pq-ll.cpp -o bin/pq-ll-undologfc -lpthread

bin/pq-ll-redologfc: pq-ll.cpp PBenchmarkQueues.hpp ../pdatastructures/queues/TMLinkedListQueueByRef.hpp ../ptms/redolog/RedoLogFC.hpp
	$(CXX) $(CXXFLAGS) -DUSE_REDO_LOG_FC $(INCLUDES) pq-ll.cpp -o bin/pq-ll-redologfc -lpthread

bin/pq-ll-trinityfc: pq-ll.cpp PBenchmarkQueues.hpp ../pdatastructures/queues/TMLinkedListQueueByRef.hpp ../ptms/trinity/TrinityFC.hpp
	$(CXX) $(CXXFLAGS) -DUSE_TRINITY_FC $(INCLUDES) pq-ll.cpp -o bin/pq-ll-trinityfc -lpthread

bin/pq-ll-trinityvrfc: pq-ll.cpp PBenchmarkQueues.hpp ../pdatastructures/queues/TMLinkedListQueueByRef.hpp ../ptms/trinity/TrinityVRFC.hpp
	$(CXX) $(CXXFLAGS) -DUSE_TRINITY_VR_FC $(INCLUDES) pq-ll.cpp -o bin/pq-ll-trinityvrfc -lpthread

bin/pq-ll-trinitytl2: pq-ll.cpp PBenchmarkQueues.hpp ../pdatastructures/queues/TMLinkedListQueueByRef.hpp ../ptms/trinity/TrinityTL2.hpp
	$(CXX) $(CXXFLAGS) -DUSE_TRINITY_TL2 $(INCLUDES) pq-ll.cpp -o bin/pq-ll-trinitytl2 -lpthread

bin/pq-ll-trinityvrtl2: pq-ll.cpp PBenchmarkQueues.hpp ../pdatastructures/queues/TMLinkedListQueueByRef.hpp ../ptms/trinity/TrinityVRTL2.hpp
	$(CXX) $(CXXFLAGS) -DUSE_TRINITY_VR_TL2 $(INCLUDES) pq-ll.cpp -o bin/pq-ll-trinityvrtl2 -lpthread

bin/pq-ll-trinityvrtl2pl: pq-ll.cpp PBenchmarkQueues.hpp ../pdatastructures/queues/TMLinkedListQueueByRef.hpp ../ptms/trinity/TrinityVRTL2PL.hpp
	$(CXX) $(CXXFLAGS) -DUSE_TRINITY_VR_TL2 $(INCLUDES) pq-ll.cpp -o bin/pq-ll-trinityvrtl2pl -lpthread

bin/pq-ll-quadrafc: pq-ll.cpp PBenchmarkQueues.hpp ../pdatastructures/queues/TMLinkedListQueueByRef.hpp ../ptms/quadra/QuadraFC.hpp
	$(CXX) $(CXXFLAGS) -DUSE_QUADRA_FC $(INCLUDES) pq-ll.cpp -o bin/pq-ll-quadrafc -lpthread

bin/pq-ll-quadravrfc: pq-ll.cpp PBenchmarkQueues.hpp ../pdatastructures/queues/TMLinkedListQueueByRef.hpp ../ptms/quadra/QuadraVRFC.hpp
	$(CXX) $(CXXFLAGS) -DUSE_QUADRA_VR_FC $(INCLUDES) pq-ll.cpp -o bin/pq-ll-quadravrfc -lpthread

bin/pq-ll-duovrfc: pq-ll.cpp PBenchmarkQueues.hpp ../pdatastructures/queues/TMLinkedListQueueByRef.hpp ../ptms/duo/DuoVRFC.hpp
	$(CXX) $(CXXFLAGS) -DUSE_DUO_VR_FC $(INCLUDES) pq-ll.cpp -o bin/pq-ll-duovrfc -lpthread

bin/pq-ll-pmdk: pq-ll.cpp PBenchmarkQueues.hpp ../pdatastructures/queues/TMLinkedListQueueByRef.hpp ../ptms/pmdk/PMDKTM.hpp
	$(CXX) $(CXXFLAGS) -DUSE_PMDK $(INCLUDES) pq-ll.cpp -o bin/pq-ll-pmdk -lpthread -lpmemobj

bin/pq-ll-oflf: pq-ll.cpp PBenchmarkQueues.hpp ../pdatastructures/queues/TMLinkedListQueueByRef.hpp ../ptms/onefile/OneFilePTMLF.hpp
	$(CXX) $(CXXFLAGS) -DUSE_OFLF $(INCLUDES) pq-ll.cpp -o bin/pq-ll-oflf -lpthread

bin/pq-ll-friedman: pq-ll.cpp PBenchmarkQueues.hpp ../pdatastructures/pqueues/PFriedmanQueue.hpp
	$(CXX) $(CXXFLAGS) -DUSE_FRIEDMAN $(INCLUDES) pq-ll.cpp -o bin/pq-ll-friedman -lpthread $(PMDKLIBS)



#
# Multithreaded fat-node queue
#
bin/pq-fat-romlogfc: pq-fat.cpp PBenchmarkQueues.hpp ../pdatastructures/queues/TMLinkedListFatQueueByRef.hpp ../ptms/romuluslog/RomLogFC.hpp
	$(CXX) $(CXXFLAGS) -DUSE_ROM_LOG_FC $(INCLUDES) pq-fat.cpp -o bin/pq-fat-romlogfc -lpthread

bin/pq-fat-undologfc: pq-fat.cpp PBenchmarkQueues.hpp ../pdatastructures/queues/TMLinkedListFatQueueByRef.hpp ../ptms/undolog/UndoLogFC.hpp
	$(CXX) $(CXXFLAGS) -DUSE_UNDO_LOG_FC $(INCLUDES) pq-fat.cpp -o bin/pq-fat-undologfc -lpthread

bin/pq-fat-redologfc: pq-fat.cpp PBenchmarkQueues.hpp ../pdatastructures/queues/TMLinkedListFatQueueByRef.hpp ../ptms/redolog/RedoLogFC.hpp
	$(CXX) $(CXXFLAGS) -DUSE_REDO_LOG_FC $(INCLUDES) pq-fat.cpp -o bin/pq-fat-redologfc -lpthread

bin/pq-fat-trinityfc: pq-fat.cpp PBenchmarkQueues.hpp ../pdatastructures/queues/TMLinkedListFatQueueByRef.hpp ../ptms/trinity/TrinityFC.hpp
	$(CXX) $(CXXFLAGS) -DUSE_TRINITY_FC $(INCLUDES) pq-fat.cpp -o bin/pq-fat-trinityfc -lpthread

bin/pq-fat-trinityvrfc: pq-fat.cpp PBenchmarkQueues.hpp ../pdatastructures/queues/TMLinkedListFatQueueByRef.hpp ../ptms/trinity/TrinityVRFC.hpp
	$(CXX) $(CXXFLAGS) -DUSE_TRINITY_VR_FC $(INCLUDES) pq-fat.cpp -o bin/pq-fat-trinityvrfc -lpthread

bin/pq-fat-trinitytl2: pq-fat.cpp PBenchmarkQueues.hpp ../pdatastructures/queues/TMLinkedListFatQueueByRef.hpp ../ptms/trinity/TrinityTL2.hpp
	$(CXX) $(CXXFLAGS) -DUSE_TRINITY_TL2 $(INCLUDES) pq-fat.cpp -o bin/pq-fat-trinitytl2 -lpthread

bin/pq-fat-trinityvrtl2: pq-fat.cpp PBenchmarkQueues.hpp ../pdatastructures/queues/TMLinkedListFatQueueByRef.hpp ../ptms/trinity/TrinityVRTL2.hpp
	$(CXX) $(CXXFLAGS) -DUSE_TRINITY_VR_TL2 $(INCLUDES) pq-fat.cpp -o bin/pq-fat-trinityvrtl2 -lpthread

bin/pq-fat-trinityvrtl2pl: pq-fat.cpp PBenchmarkQueues.hpp ../pdatastructures/queues/TMLinkedListFatQueueByRef.hpp ../ptms/trinity/TrinityVRTL2PL.hpp
	$(CXX) $(CXXFLAGS) -DUSE_TRINITY_VR_TL2 $(INCLUDES) pq-fat.cpp -o bin/pq-fat-trinityvrtl2pl -lpthread

bin/pq-fat-quadrafc: pq-fat.cpp PBenchmarkQueues.hpp ../pdatastructures/queues/TMLinkedListFatQueueByRef.hpp ../ptms/quadra/QuadraFC.hpp
	$(CXX) $(CXXFLAGS) -DUSE_QUADRA_FC $(INCLUDES) pq-fat.cpp -o bin/pq-fat-quadrafc -lpthread

bin/pq-fat-quadravrfc: pq-fat.cpp PBenchmarkQueues.hpp ../pdatastructures/queues/TMLinkedListFatQueueByRef.hpp ../ptms/quadra/QuadraVRFC.hpp
	$(CXX) $(CXXFLAGS) -DUSE_QUADRA_VR_FC $(INCLUDES) pq-fat.cpp -o bin/pq-fat-quadravrfc -lpthread

bin/pq-fat-duovrfc: pq-fat.cpp PBenchmarkQueues.hpp ../pdatastructures/queues/TMLinkedListFatQueueByRef.hpp ../ptms/duo/DuoVRFC.hpp
	$(CXX) $(CXXFLAGS) -DUSE_DUO_VR_FC $(INCLUDES) pq-fat.cpp -o bin/pq-fat-duovrfc -lpthread

bin/pq-fat-pmdk: pq-fat.cpp PBenchmarkQueues.hpp ../pdatastructures/queues/TMLinkedListFatQueueByRef.hpp ../ptms/pmdk/PMDKTM.hpp
	$(CXX) $(CXXFLAGS) -DUSE_PMDK $(INCLUDES) pq-fat.cpp -o bin/pq-fat-ll-pmdk -lpthread -lpmemobj

bin/pq-fat-oflf: pq-fat.cpp PBenchmarkQueues.hpp ../pdatastructures/queues/TMLinkedListFatQueueByRef.hpp ../ptms/onefile/OneFilePTMLF.hpp
	$(CXX) $(CXXFLAGS) -DUSE_OFLF $(INCLUDES) pq-fat.cpp -o bin/pq-fat-ll-oflf -lpthread -lpmemobj

bin/pq-fat-friedman: pq-fat.cpp PBenchmarkQueues.hpp ../pdatastructures/pqueues/PFriedmanQueue.hpp
	$(CXX) $(CXXFLAGS) -DUSE_FRIEDMAN $(INCLUDES) pq-fat.cpp -o bin/pq-fat-friedman -lpthread $(PMDKLIBS)


#
# Multithreaded stack (linked list)
#
bin/pstack-ll-romlogfc: pstack-ll.cpp PBenchmarkStacks.hpp ../pdatastructures/stacks/TMLinkedListStackByRef.hpp ../ptms/romuluslog/RomLogFC.hpp
	$(CXX) $(CXXFLAGS) -DUSE_ROM_LOG_FC $(INCLUDES) pstack-ll.cpp -o bin/pstack-ll-romlogfc -lpthread

bin/pstack-ll-undologfc: pstack-ll.cpp PBenchmarkStacks.hpp ../pdatastructures/stacks/TMLinkedListStackByRef.hpp ../ptms/undolog/UndoLogFC.hpp
	$(CXX) $(CXXFLAGS) -DUSE_UNDO_LOG_FC $(INCLUDES) pstack-ll.cpp -o bin/pstack-ll-undologfc -lpthread

bin/pstack-ll-redologfc: pstack-ll.cpp PBenchmarkStacks.hpp ../pdatastructures/stacks/TMLinkedListStackByRef.hpp ../ptms/redolog/RedoLogFC.hpp
	$(CXX) $(CXXFLAGS) -DUSE_REDO_LOG_FC $(INCLUDES) pstack-ll.cpp -o bin/pstack-ll-redologfc -lpthread

bin/pstack-ll-trinityfc: pstack-ll.cpp PBenchmarkStacks.hpp ../pdatastructures/stacks/TMLinkedListStackByRef.hpp ../ptms/trinity/TrinityFC.hpp
	$(CXX) $(CXXFLAGS) -DUSE_TRINITY_FC $(INCLUDES) pstack-ll.cpp -o bin/pstack-ll-trinityfc -lpthread

bin/pstack-ll-trinityvrfc: pstack-ll.cpp PBenchmarkStacks.hpp ../pdatastructures/stacks/TMLinkedListStackByRef.hpp ../ptms/trinity/TrinityVRFC.hpp
	$(CXX) $(CXXFLAGS) -DUSE_TRINITY_VR_FC $(INCLUDES) pstack-ll.cpp -o bin/pstack-ll-trinityvrfc -lpthread

bin/pstack-ll-trinitytl2: pstack-ll.cpp PBenchmarkStacks.hpp ../pdatastructures/stacks/TMLinkedListStackByRef.hpp ../ptms/trinity/TrinityTL2.hpp
	$(CXX) $(CXXFLAGS) -DUSE_TRINITY_TL2 $(INCLUDES) pstack-ll.cpp -o bin/pstack-ll-trinitytl2 -lpthread

bin/pstack-ll-trinityvrtl2: pstack-ll.cpp PBenchmarkStacks.hpp ../pdatastructures/stacks/TMLinkedListStackByRef.hpp ../ptms/trinity/TrinityVRTL2.hpp
	$(CXX) $(CXXFLAGS) -DUSE_TRINITY_VR_TL2 $(INCLUDES) pstack-ll.cpp -o bin/pstack-ll-trinityvrtl2 -lpthread

bin/pstack-ll-trinityvrtl2pl: pstack-ll.cpp PBenchmarkStacks.hpp ../pdatastructures/stacks/TMLinkedListStackByRef.hpp ../ptms/trinity/TrinityVRTL2PL.hpp
	$(CXX) $(CXXFLAGS) -DUSE_TRINITY_VR_TL2 $(INCLUDES) pstack-ll.cpp -o bin/pstack-ll-trinityvrtl2pl -lpthread

bin/pstack-ll-quadrafc: pstack-ll.cpp PBenchmarkStacks.hpp ../pdatastructures/stacks/TMLinkedListStackByRef.hpp ../ptms/quadra/QuadraFC.hpp
	$(CXX) $(CXXFLAGS) -DUSE_QUADRA_FC $(INCLUDES) pstack-ll.cpp -o bin/pstack-ll-quadrafc -lpthread

bin/pstack-ll-quadravrfc: pstack-ll.cpp PBenchmarkStacks.hpp ../pdatastructures/stacks/TMLinkedListStackByRef.hpp ../ptms/quadra/QuadraVRFC.hpp
	$(CXX) $(CXXFLAGS) -DUSE_QUADRA_VR_FC $(INCLUDES) pstack-ll.cpp -o bin/pstack-ll-quadravrfc -lpthread

bin/pstack-ll-duovrfc: pstack-ll.cpp PBenchmarkStacks.hpp ../pdatastructures/stacks/TMLinkedListStackByRef.hpp ../ptms/duo/DuoVRFC.hpp
	$(CXX) $(CXXFLAGS) -DUSE_DUO_VR_FC $(INCLUDES) pstack-ll.cpp -o bin/pstack-ll-duovrfc -lpthread

bin/pstack-ll-pmdk: pstack-ll.cpp PBenchmarkStacks.hpp ../pdatastructures/stacks/TMLinkedListStackByRef.hpp ../ptms/pmdk/PMDKTM.hpp
	$(CXX) $(CXXFLAGS) -DUSE_PMDK $(INCLUDES) pstack-ll.cpp -o bin/pstack-ll-pmdk -lpthread -lpmemobj

bin/pstack-ll-oflf: pstack-ll.cpp PBenchmarkStacks.hpp ../pdatastructures/stacks/TMLinkedListStackByRef.hpp ../ptms/onefile/OneFilePTMLF.hpp
	$(CXX) $(CXXFLAGS) -DUSE_OFLF $(INCLUDES) pstack-ll.cpp -o bin/pstack-ll-oflf -lpthread


#
# Multithreaded stack (fat nodes)
#
bin/pstack-fat-romlogfc: pstack-fat.cpp PBenchmarkStacks.hpp ../pdatastructures/stacks/TMLinkedListFatStackByRef.hpp ../ptms/romuluslog/RomLogFC.hpp
	$(CXX) $(CXXFLAGS) -DUSE_ROM_LOG_FC $(INCLUDES) pstack-fat.cpp -o bin/pstack-fat-romlogfc -lpthread

bin/pstack-fat-undologfc: pstack-fat.cpp PBenchmarkStacks.hpp ../pdatastructures/stacks/TMLinkedListFatStackByRef.hpp ../ptms/undolog/UndoLogFC.hpp
	$(CXX) $(CXXFLAGS) -DUSE_UNDO_LOG_FC $(INCLUDES) pstack-fat.cpp -o bin/pstack-fat-undologfc -lpthread

bin/pstack-fat-redologfc: pstack-fat.cpp PBenchmarkStacks.hpp ../pdatastructures/stacks/TMLinkedListFatStackByRef.hpp ../ptms/redolog/RedoLogFC.hpp
	$(CXX) $(CXXFLAGS) -DUSE_REDO_LOG_FC $(INCLUDES) pstack-fat.cpp -o bin/pstack-fat-redologfc -lpthread

bin/pstack-fat-trinityfc: pstack-fat.cpp PBenchmarkStacks.hpp ../pdatastructures/stacks/TMLinkedListFatStackByRef.hpp ../ptms/trinity/TrinityFC.hpp
	$(CXX) $(CXXFLAGS) -DUSE_TRINITY_FC $(INCLUDES) pstack-fat.cpp -o bin/pstack-fat-trinityfc -lpthread

bin/pstack-fat-trinityvrfc: pstack-fat.cpp PBenchmarkStacks.hpp ../pdatastructures/stacks/TMLinkedListFatStackByRef.hpp ../ptms/trinity/TrinityVRFC.hpp
	$(CXX) $(CXXFLAGS) -DUSE_TRINITY_VR_FC $(INCLUDES) pstack-fat.cpp -o bin/pstack-fat-trinityvrfc -lpthread

bin/pstack-fat-trinitytl2: pstack-fat.cpp PBenchmarkStacks.hpp ../pdatastructures/stacks/TMLinkedListFatStackByRef.hpp ../ptms/trinity/TrinityTL2.hpp
	$(CXX) $(CXXFLAGS) -DUSE_TRINITY_TL2 $(INCLUDES) pstack-fat.cpp -o bin/pstack-fat-trinitytl2 -lpthread

bin/pstack-fat-trinityvrtl2: pstack-fat.cpp PBenchmarkStacks.hpp ../pdatastructures/stacks/TMLinkedListFatStackByRef.hpp ../ptms/trinity/TrinityVRTL2.hpp
	$(CXX) $(CXXFLAGS) -DUSE_TRINITY_VR_TL2 $(INCLUDES) pstack-fat.cpp -o bin/pstack-fat-trinityvrtl2 -lpthread

bin/pstack-fat-trinityvrtl2pl: pstack-fat.cpp PBenchmarkStacks.hpp ../pdatastructures/stacks/TMLinkedListFatStackByRef.hpp ../ptms/trinity/TrinityVRTL2PL.hpp
	$(CXX) $(CXXFLAGS) -DUSE_TRINITY_VR_TL2 $(INCLUDES) pstack-fat.cpp -o bin/pstack-fat-trinityvrtl2pl -lpthread

bin/pstack-fat-quadrafc: pstack-fat.cpp PBenchmarkStacks.hpp ../pdatastructures/stacks/TMLinkedListFatStackByRef.hpp ../ptms/quadra/QuadraFC.hpp
	$(CXX) $(CXXFLAGS) -DUSE_QUADRA_FC $(INCLUDES) pstack-fat.cpp -o bin/pstack-fat-quadrafc -lpthread

bin/pstack-fat-quadravrfc: pstack-fat.cpp PBenchmarkStacks.hpp ../pdatastructures/stacks/TMLinkedListFatStackByRef.hpp ../ptms/quadra/QuadraVRFC.hpp
	$(CXX) $(CXXFLAGS) -DUSE_QUADRA_VR_FC $(INCLUDES) pstack-fat.cpp -o bin/pstack-fat-quadravrfc -lpthread

bin/pstack-fat-duovrfc: pstack-fat.cpp PBenchmarkStacks.hpp ../pdatastructures/stacks/TMLinkedListFatStackByRef.hpp ../ptms/duo/DuoVRFC.hpp
	$(CXX) $(CXXFLAGS) -DUSE_DUO_VR_FC $(INCLUDES) pstack-fat.cpp -o bin/pstack-fat-duovrfc -lpthread

bin/pstack-fat-pmdk: pstack-fat.cpp PBenchmarkStacks.hpp ../pdatastructures/stacks/TMLinkedListFatStackByRef.hpp ../ptms/pmdk/PMDKTM.hpp
	$(CXX) $(CXXFLAGS) -DUSE_PMDK $(INCLUDES) pstack-fat.cpp -o bin/pstack-fat-pmdk -lpthread -lpmemobj

bin/pstack-fat-oflf: pstack-fat.cpp PBenchmarkStacks.hpp ../pdatastructures/stacks/TMLinkedListFatStackByRef.hpp ../ptms/onefile/OneFilePTMLF.hpp
	$(CXX) $(CXXFLAGS) -DUSE_OFLF $(INCLUDES) pstack-fat.cpp -o bin/pstack-fat-oflf -lpthread -lpmemobj


#
# Multithreaded tree 1M
#
bin/pset-tree-1m-romlogfc: pset-tree-1m.cpp PBenchmarkSets.hpp ../ptms/romuluslog/RomLogFC.hpp
	$(CXX) $(CXXFLAGS) -DUSE_ROM_LOG_FC $(INCLUDES) pset-tree-1m.cpp -o bin/pset-tree-1m-romlogfc -lpthread

bin/pset-tree-1m-undologfc: pset-tree-1m.cpp PBenchmarkSets.hpp ../ptms/undolog/UndoLogFC.hpp
	$(CXX) $(CXXFLAGS) -DUSE_UNDO_LOG_FC $(INCLUDES) pset-tree-1m.cpp -o bin/pset-tree-1m-undologfc -lpthread

bin/pset-tree-1m-redologfc: pset-tree-1m.cpp PBenchmarkSets.hpp ../ptms/redolog/RedoLogFC.hpp
	$(CXX) $(CXXFLAGS) -DUSE_REDO_LOG_FC $(INCLUDES) pset-tree-1m.cpp -o bin/pset-tree-1m-redologfc -lpthread

bin/pset-tree-1m-trinityfc: pset-tree-1m.cpp PBenchmarkSets.hpp ../ptms/trinity/TrinityFC.hpp
	$(CXX) $(CXXFLAGS) -DUSE_TRINITY_FC $(INCLUDES) pset-tree-1m.cpp -o bin/pset-tree-1m-trinityfc -lpthread

bin/pset-tree-1m-trinityvrfc: pset-tree-1m.cpp PBenchmarkSets.hpp ../ptms/trinity/TrinityVRFC.hpp
	$(CXX) $(CXXFLAGS) -DUSE_TRINITY_VR_FC $(INCLUDES) pset-tree-1m.cpp -o bin/pset-tree-1m-trinityvrfc -lpthread

bin/pset-tree-1m-trinitytl2: pset-tree-1m.cpp PBenchmarkSets.hpp ../ptms/trinity/TrinityTL2.hpp
	$(CXX) $(CXXFLAGS) -DUSE_TRINITY_TL2 $(INCLUDES) pset-tree-1m.cpp -o bin/pset-tree-1m-trinitytl2 -lpthread

bin/pset-tree-1m-trinityvrtl2: pset-tree-1m.cpp PBenchmarkSets.hpp ../ptms/trinity/TrinityVRTL2.hpp
	$(CXX) $(CXXFLAGS) -DUSE_TRINITY_VR_TL2 $(INCLUDES) pset-tree-1m.cpp -o bin/pset-tree-1m-trinityvrtl2 -lpthread

bin/pset-tree-1m-trinityvrtl2pl: pset-tree-1m.cpp PBenchmarkSets.hpp ../ptms/trinity/TrinityVRTL2PL.hpp
	$(CXX) $(CXXFLAGS) -DUSE_TRINITY_VR_TL2_PL $(INCLUDES) pset-tree-1m.cpp -o bin/pset-tree-1m-trinityvrtl2pl -lpthread

bin/pset-tree-1m-quadrafc: pset-tree-1m.cpp PBenchmarkSets.hpp ../ptms/quadra/QuadraFC.hpp
	$(CXX) $(CXXFLAGS) -DUSE_QUADRA_FC $(INCLUDES) pset-tree-1m.cpp -o bin/pset-tree-1m-quadrafc -lpthread

bin/pset-tree-1m-quadravrfc: pset-tree-1m.cpp PBenchmarkSets.hpp ../ptms/quadra/QuadraVRFC.hpp
	$(CXX) $(CXXFLAGS) -DUSE_QUADRA_VR_FC $(INCLUDES) pset-tree-1m.cpp -o bin/pset-tree-1m-quadravrfc -lpthread

bin/pset-tree-1m-duovrfc: pset-tree-1m.cpp PBenchmarkSets.hpp ../ptms/duo/DuoVRFC.hpp
	$(CXX) $(CXXFLAGS) -DUSE_DUO_VR_FC $(INCLUDES) pset-tree-1m.cpp -o bin/pset-tree-1m-duovrfc -lpthread

bin/pset-tree-1m-pmdk: pset-tree-1m.cpp PBenchmarkSets.hpp ../ptms/pmdk/PMDKTM.hpp
	$(CXX) $(CXXFLAGS) -DUSE_PMDK $(INCLUDES) pset-tree-1m.cpp -o bin/pset-tree-1m-pmdk -lpthread -lpmemobj

bin/pset-tree-1m-oflf: pset-tree-1m.cpp PBenchmarkSets.hpp ../ptms/onefile/OneFilePTMLF.hpp
	$(CXX) $(CXXFLAGS) -DUSE_OFLF $(INCLUDES) pset-tree-1m.cpp -o bin/pset-tree-1m-oflf -lpthread

# These are just to cross-check results. They're not built by default
bin/pset-tree-1m-romlr: pset-tree-1m.cpp PBenchmarkSets.hpp ../pdatastructures/TMRedBlackTree.hpp lib/libromulus.a 
	$(CXX) $(CXXFLAGS) -DUSE_ROMLR $(INCLUDES) pset-tree-1m.cpp -o bin/pset-tree-1m-romlr -lpthread lib/libromulus.a

bin/pset-tree-1m-romlog: pset-tree-1m.cpp PBenchmarkSets.hpp ../pdatastructures/TMRedBlackTree.hpp lib/libromulus.a 
	$(CXX) $(CXXFLAGS) -DUSE_ROMLOG $(INCLUDES) pset-tree-1m.cpp -o bin/pset-tree-1m-romlog -lpthread lib/libromulus.a



#
# Multithreaded (resizable) hashmap 1M
#
bin/pset-hash-1m-romlogfc: pset-hash-1m.cpp PBenchmarkSets.hpp ../ptms/romuluslog/RomLogFC.hpp
	$(CXX) $(CXXFLAGS) -DUSE_ROM_LOG_FC $(INCLUDES) pset-hash-1m.cpp -o bin/pset-hash-1m-romlogfc -lpthread

bin/pset-hash-1m-undologfc: pset-hash-1m.cpp PBenchmarkSets.hpp ../ptms/undolog/UndoLogFC.hpp
	$(CXX) $(CXXFLAGS) -DUSE_UNDO_LOG_FC $(INCLUDES) pset-hash-1m.cpp -o bin/pset-hash-1m-undologfc -lpthread

bin/pset-hash-1m-redologfc: pset-hash-1m.cpp PBenchmarkSets.hpp ../ptms/redolog/RedoLogFC.hpp
	$(CXX) $(CXXFLAGS) -DUSE_REDO_LOG_FC $(INCLUDES) pset-hash-1m.cpp -o bin/pset-hash-1m-redologfc -lpthread

bin/pset-hash-1m-trinityfc: pset-hash-1m.cpp PBenchmarkSets.hpp ../ptms/trinity/TrinityFC.hpp
	$(CXX) $(CXXFLAGS) -DUSE_TRINITY_FC $(INCLUDES) pset-hash-1m.cpp -o bin/pset-hash-1m-trinityfc -lpthread

bin/pset-hash-1m-trinityvrfc: pset-hash-1m.cpp PBenchmarkSets.hpp ../ptms/trinity/TrinityVRFC.hpp
	$(CXX) $(CXXFLAGS) -DUSE_TRINITY_VR_FC $(INCLUDES) pset-hash-1m.cpp -o bin/pset-hash-1m-trinityvrfc -lpthread

bin/pset-hash-1m-trinitytl2: pset-hash-1m.cpp PBenchmarkSets.hpp ../ptms/trinity/TrinityTL2.hpp
	$(CXX) $(CXXFLAGS) -DUSE_TRINITY_TL2 $(INCLUDES) pset-hash-1m.cpp -o bin/pset-hash-1m-trinitytl2 -lpthread

bin/pset-hash-1m-trinityvrtl2: pset-hash-1m.cpp PBenchmarkSets.hpp ../ptms/trinity/TrinityVRTL2.hpp
	$(CXX) $(CXXFLAGS) -DUSE_TRINITY_VR_TL2 $(INCLUDES) pset-hash-1m.cpp -o bin/pset-hash-1m-trinityvrtl2 -lpthread

bin/pset-hash-1m-trinityvrtl2pl: pset-hash-1m.cpp PBenchmarkSets.hpp ../ptms/trinity/TrinityVRTL2PL.hpp
	$(CXX) $(CXXFLAGS) -DUSE_TRINITY_VR_TL2_PL $(INCLUDES) pset-hash-1m.cpp -o bin/pset-hash-1m-trinityvrtl2pl -lpthread

bin/pset-hash-1m-quadrafc: pset-hash-1m.cpp PBenchmarkSets.hpp ../ptms/quadra/QuadraFC.hpp
	$(CXX) $(CXXFLAGS) -DUSE_QUADRA_FC $(INCLUDES) pset-hash-1m.cpp -o bin/pset-hash-1m-quadrafc -lpthread

bin/pset-hash-1m-quadravrfc: pset-hash-1m.cpp PBenchmarkSets.hpp ../ptms/quadra/QuadraVRFC.hpp
	$(CXX) $(CXXFLAGS) -DUSE_QUADRA_VR_FC $(INCLUDES) pset-hash-1m.cpp -o bin/pset-hash-1m-quadravrfc -lpthread

bin/pset-hash-1m-duovrfc: pset-hash-1m.cpp PBenchmarkSets.hpp ../ptms/duo/DuoVRFC.hpp
	$(CXX) $(CXXFLAGS) -DUSE_DUO_VR_FC $(INCLUDES) pset-hash-1m.cpp -o bin/pset-hash-1m-duovrfc -lpthread

bin/pset-hash-1m-pmdk: pset-hash-1m.cpp PBenchmarkSets.hpp ../ptms/pmdk/PMDKTM.hpp
	$(CXX) $(CXXFLAGS) -DUSE_PMDK $(INCLUDES) pset-hash-1m.cpp -o bin/pset-hash-1m-pmdk -lpthread -lpmemobj

bin/pset-hash-1m-oflf: pset-hash-1m.cpp PBenchmarkSets.hpp ../ptms/onefile/OneFilePTMLF.hpp
	$(CXX) $(CXXFLAGS) -DUSE_OFLF $(INCLUDES) pset-hash-1m.cpp -o bin/pset-hash-1m-oflf -lpthread


#
# Multithreaded (fixed-size) hashmap 1M
#
bin/pset-hashfixed-1m-romlogfc: pset-hashfixed-1m.cpp PBenchmarkSets.hpp ../ptms/romuluslog/RomLogFC.hpp
	$(CXX) $(CXXFLAGS) -DUSE_ROM_LOG_FC $(INCLUDES) pset-hashfixed-1m.cpp -o bin/pset-hashfixed-1m-romlogfc -lpthread

bin/pset-hashfixed-1m-undologfc: pset-hashfixed-1m.cpp PBenchmarkSets.hpp ../ptms/undolog/UndoLogFC.hpp
	$(CXX) $(CXXFLAGS) -DUSE_UNDO_LOG_FC $(INCLUDES) pset-hashfixed-1m.cpp -o bin/pset-hashfixed-1m-undologfc -lpthread

bin/pset-hashfixed-1m-redologfc: pset-hashfixed-1m.cpp PBenchmarkSets.hpp ../ptms/redolog/RedoLogFC.hpp
	$(CXX) $(CXXFLAGS) -DUSE_REDO_LOG_FC $(INCLUDES) pset-hashfixed-1m.cpp -o bin/pset-hashfixed-1m-redologfc -lpthread

bin/pset-hashfixed-1m-trinityfc: pset-hashfixed-1m.cpp PBenchmarkSets.hpp ../ptms/trinity/TrinityFC.hpp
	$(CXX) $(CXXFLAGS) -DUSE_TRINITY_FC $(INCLUDES) pset-hashfixed-1m.cpp -o bin/pset-hashfixed-1m-trinityfc -lpthread

bin/pset-hashfixed-1m-trinityvrfc: pset-hashfixed-1m.cpp PBenchmarkSets.hpp ../ptms/trinity/TrinityVRFC.hpp
	$(CXX) $(CXXFLAGS) -DUSE_TRINITY_VR_FC $(INCLUDES) pset-hashfixed-1m.cpp -o bin/pset-hashfixed-1m-trinityvrfc -lpthread

bin/pset-hashfixed-1m-trinitytl2: pset-hashfixed-1m.cpp PBenchmarkSets.hpp ../ptms/trinity/TrinityTL2.hpp
	$(CXX) $(CXXFLAGS) -DUSE_TRINITY_TL2 $(INCLUDES) pset-hashfixed-1m.cpp -o bin/pset-hashfixed-1m-trinitytl2 -lpthread

bin/pset-hashfixed-1m-trinityvrtl2: pset-hashfixed-1m.cpp PBenchmarkSets.hpp ../ptms/trinity/TrinityVRTL2.hpp
	$(CXX) $(CXXFLAGS) -DUSE_TRINITY_VR_TL2 $(INCLUDES) pset-hashfixed-1m.cpp -o bin/pset-hashfixed-1m-trinityvrtl2 -lpthread

bin/pset-hashfixed-1m-trinityvrtl2pl: pset-hashfixed-1m.cpp PBenchmarkSets.hpp ../ptms/trinity/TrinityVRTL2PL.hpp
	$(CXX) $(CXXFLAGS) -DUSE_TRINITY_VR_TL2_PL $(INCLUDES) pset-hashfixed-1m.cpp -o bin/pset-hashfixed-1m-trinityvrtl2pl -lpthread

bin/pset-hashfixed-1m-quadrafc: pset-hashfixed-1m.cpp PBenchmarkSets.hpp ../ptms/quadra/QuadraFC.hpp
	$(CXX) $(CXXFLAGS) -DUSE_QUADRA_FC $(INCLUDES) pset-hashfixed-1m.cpp -o bin/pset-hashfixed-1m-quadrafc -lpthread

bin/pset-hashfixed-1m-quadravrfc: pset-hashfixed-1m.cpp PBenchmarkSets.hpp ../ptms/quadra/QuadraVRFC.hpp
	$(CXX) $(CXXFLAGS) -DUSE_QUADRA_VR_FC $(INCLUDES) pset-hashfixed-1m.cpp -o bin/pset-hashfixed-1m-quadravrfc -lpthread

bin/pset-hashfixed-1m-duovrfc: pset-hashfixed-1m.cpp PBenchmarkSets.hpp ../ptms/duo/DuoVRFC.hpp
	$(CXX) $(CXXFLAGS) -DUSE_DUO_VR_FC $(INCLUDES) pset-hashfixed-1m.cpp -o bin/pset-hashfixed-1m-duovrfc -lpthread

bin/pset-hashfixed-1m-pmdk: pset-hashfixed-1m.cpp PBenchmarkSets.hpp ../ptms/pmdk/PMDKTM.hpp
	$(CXX) $(CXXFLAGS) -DUSE_PMDK $(INCLUDES) pset-hashfixed-1m.cpp -o bin/pset-hashfixed-1m-pmdk -lpthread -lpmemobj

bin/pset-hashfixed-1m-oflf: pset-hashfixed-1m.cpp PBenchmarkSets.hpp ../ptms/onefile/OneFilePTMLF.hpp
	$(CXX) $(CXXFLAGS) -DUSE_OFLF $(INCLUDES) pset-hashfixed-1m.cpp -o bin/pset-hashfixed-1m-oflf -lpthread


#
# Multithreaded B-tree 1M
#
bin/pset-btree-1m-romlogfc: pset-btree-1m.cpp PBenchmarkSets.hpp ../ptms/romuluslog/RomLogFC.hpp
	$(CXX) $(CXXFLAGS) -DUSE_ROM_LOG_FC $(INCLUDES) pset-btree-1m.cpp -o bin/pset-btree-1m-romlogfc -lpthread

bin/pset-btree-1m-undologfc: pset-btree-1m.cpp PBenchmarkSets.hpp ../ptms/undolog/UndoLogFC.hpp
	$(CXX) $(CXXFLAGS) -DUSE_UNDO_LOG_FC $(INCLUDES) pset-btree-1m.cpp -o bin/pset-btree-1m-undologfc -lpthread

bin/pset-btree-1m-redologfc: pset-btree-1m.cpp PBenchmarkSets.hpp ../ptms/redolog/RedoLogFC.hpp
	$(CXX) $(CXXFLAGS) -DUSE_REDO_LOG_FC $(INCLUDES) pset-btree-1m.cpp -o bin/pset-btree-1m-redologfc -lpthread

bin/pset-btree-1m-trinityfc: pset-btree-1m.cpp PBenchmarkSets.hpp ../ptms/trinity/TrinityFC.hpp
	$(CXX) $(CXXFLAGS) -DUSE_TRINITY_FC $(INCLUDES) pset-btree-1m.cpp -o bin/pset-btree-1m-trinityfc -lpthread

bin/pset-btree-1m-trinityvrfc: pset-btree-1m.cpp PBenchmarkSets.hpp ../ptms/trinity/TrinityVRFC.hpp
	$(CXX) $(CXXFLAGS) -DUSE_TRINITY_VR_FC $(INCLUDES) pset-btree-1m.cpp -o bin/pset-btree-1m-trinityvrfc -lpthread

bin/pset-btree-1m-trinitytl2: pset-btree-1m.cpp PBenchmarkSets.hpp ../ptms/trinity/TrinityTL2.hpp
	$(CXX) $(CXXFLAGS) -DUSE_TRINITY_TL2 $(INCLUDES) pset-btree-1m.cpp -o bin/pset-btree-1m-trinitytl2 -lpthread

bin/pset-btree-1m-trinityvrtl2: pset-btree-1m.cpp PBenchmarkSets.hpp ../ptms/trinity/TrinityVRTL2.hpp
	$(CXX) $(CXXFLAGS) -DUSE_TRINITY_VR_TL2 $(INCLUDES) pset-btree-1m.cpp -o bin/pset-btree-1m-trinityvrtl2 -lpthread

bin/pset-btree-1m-trinityvrtl2pl: pset-btree-1m.cpp PBenchmarkSets.hpp ../ptms/trinity/TrinityVRTL2PL.hpp
	$(CXX) $(CXXFLAGS) -DUSE_TRINITY_VR_TL2_PL $(INCLUDES) pset-btree-1m.cpp -o bin/pset-btree-1m-trinityvrtl2pl -lpthread

bin/pset-btree-1m-quadrafc: pset-btree-1m.cpp PBenchmarkSets.hpp ../ptms/quadra/QuadraFC.hpp
	$(CXX) $(CXXFLAGS) -DUSE_QUADRA_FC $(INCLUDES) pset-btree-1m.cpp -o bin/pset-btree-1m-quadrafc -lpthread

bin/pset-btree-1m-quadravrfc: pset-btree-1m.cpp PBenchmarkSets.hpp ../ptms/quadra/QuadraVRFC.hpp
	$(CXX) $(CXXFLAGS) -DUSE_QUADRA_VR_FC $(INCLUDES) pset-btree-1m.cpp -o bin/pset-btree-1m-quadravrfc -lpthread

bin/pset-btree-1m-duovrfc: pset-btree-1m.cpp PBenchmarkSets.hpp ../ptms/duo/DuoVRFC.hpp
	$(CXX) $(CXXFLAGS) -DUSE_DUO_VR_FC $(INCLUDES) pset-btree-1m.cpp -o bin/pset-btree-1m-duovrfc -lpthread

bin/pset-btree-1m-pmdk: pset-btree-1m.cpp PBenchmarkSets.hpp ../ptms/pmdk/PMDKTM.hpp
	$(CXX) $(CXXFLAGS) -DUSE_PMDK $(INCLUDES) pset-btree-1m.cpp -o bin/pset-btree-1m-pmdk -lpthread -lpmemobj

bin/pset-btree-1m-oflf: pset-btree-1m.cpp PBenchmarkSets.hpp ../ptms/onefile/OneFilePTMLF.hpp
	$(CXX) $(CXXFLAGS) -DUSE_OFLF $(INCLUDES) pset-btree-1m.cpp -o bin/pset-btree-1m-oflf -lpthread

	
#
# Multithreaded Zip Tree Set 1M
#
bin/pset-ziptree-1m-romlogfc: pset-ziptree-1m.cpp PBenchmarkSets.hpp ../pdatastructures/TMZipTreeSet.hpp ../ptms/romuluslog/RomLogFC.hpp
	$(CXX) $(CXXFLAGS) -DUSE_ROM_LOG_FC $(INCLUDES) pset-ziptree-1m.cpp -o bin/pset-ziptree-1m-romlogfc -lpthread

bin/pset-ziptree-1m-undologfc: pset-ziptree-1m.cpp PBenchmarkSets.hpp ../pdatastructures/TMZipTreeSet.hpp ../ptms/undolog/UndoLogFC.hpp
	$(CXX) $(CXXFLAGS) -DUSE_UNDO_LOG_FC $(INCLUDES) pset-ziptree-1m.cpp -o bin/pset-ziptree-1m-undologfc -lpthread

bin/pset-ziptree-1m-redologfc: pset-ziptree-1m.cpp PBenchmarkSets.hpp ../pdatastructures/TMZipTreeSet.hpp ../ptms/redolog/RedoLogFC.hpp
	$(CXX) $(CXXFLAGS) -DUSE_REDO_LOG_FC $(INCLUDES) pset-ziptree-1m.cpp -o bin/pset-ziptree-1m-redologfc -lpthread

bin/pset-ziptree-1m-trinityfc: pset-ziptree-1m.cpp PBenchmarkSets.hpp ../pdatastructures/TMZipTreeSet.hpp ../ptms/trinity/TrinityFC.hpp
	$(CXX) $(CXXFLAGS) -DUSE_TRINITY_FC $(INCLUDES) pset-ziptree-1m.cpp -o bin/pset-ziptree-1m-trinityfc -lpthread

bin/pset-ziptree-1m-trinityvrfc: pset-ziptree-1m.cpp PBenchmarkSets.hpp ../pdatastructures/TMZipTreeSet.hpp ../ptms/trinity/TrinityVRFC.hpp
	$(CXX) $(CXXFLAGS) -DUSE_TRINITY_VR_FC $(INCLUDES) pset-ziptree-1m.cpp -o bin/pset-ziptree-1m-trinityvrfc -lpthread

bin/pset-ziptree-1m-trinitytl2: pset-ziptree-1m.cpp PBenchmarkSets.hpp ../pdatastructures/TMZipTreeSet.hpp ../ptms/trinity/TrinityTL2.hpp
	$(CXX) $(CXXFLAGS) -DUSE_TRINITY_TL2 $(INCLUDES) pset-ziptree-1m.cpp -o bin/pset-ziptree-1m-trinitytl2 -lpthread

bin/pset-ziptree-1m-trinityvrtl2: pset-ziptree-1m.cpp PBenchmarkSets.hpp ../pdatastructures/TMZipTreeSet.hpp ../ptms/trinity/TrinityVRTL2.hpp
	$(CXX) $(CXXFLAGS) -DUSE_TRINITY_VR_TL2 $(INCLUDES) pset-ziptree-1m.cpp -o bin/pset-ziptree-1m-trinityvrtl2 -lpthread

bin/pset-ziptree-1m-trinityvrtl2pl: pset-ziptree-1m.cpp PBenchmarkSets.hpp ../pdatastructures/TMZipTreeSet.hpp ../ptms/trinity/TrinityVRTL2PL.hpp
	$(CXX) $(CXXFLAGS) -DUSE_TRINITY_VR_TL2_PL $(INCLUDES) pset-ziptree-1m.cpp -o bin/pset-ziptree-1m-trinityvrtl2pl -lpthread

bin/pset-ziptree-1m-quadrafc: pset-ziptree-1m.cpp PBenchmarkSets.hpp ../pdatastructures/TMZipTreeSet.hpp ../ptms/quadra/QuadraFC.hpp
	$(CXX) $(CXXFLAGS) -DUSE_QUADRA_FC $(INCLUDES) pset-ziptree-1m.cpp -o bin/pset-ziptree-1m-quadrafc -lpthread

bin/pset-ziptree-1m-quadravrfc: pset-ziptree-1m.cpp PBenchmarkSets.hpp ../pdatastructures/TMZipTreeSet.hpp ../ptms/quadra/QuadraVRFC.hpp
	$(CXX) $(CXXFLAGS) -DUSE_QUADRA_VR_FC $(INCLUDES) pset-ziptree-1m.cpp -o bin/pset-ziptree-1m-quadravrfc -lpthread

bin/pset-ziptree-1m-duovrfc: pset-ziptree-1m.cpp PBenchmarkSets.hpp ../pdatastructures/TMZipTreeSet.hpp ../ptms/duo/DuoVRFC.hpp
	$(CXX) $(CXXFLAGS) -DUSE_DUO_VR_FC $(INCLUDES) pset-ziptree-1m.cpp -o bin/pset-ziptree-1m-duovrfc -lpthread

bin/pset-ziptree-1m-pmdk: pset-ziptree-1m.cpp PBenchmarkSets.hpp ../pdatastructures/TMZipTreeSet.hpp ../ptms/pmdk/PMDKTM.hpp
	$(CXX) $(CXXFLAGS) -DUSE_PMDK $(INCLUDES) pset-ziptree-1m.cpp -o bin/pset-ziptree-1m-pmdk -lpthread -lpmemobj

bin/pset-ziptree-1m-oflf: pset-ziptree-1m.cpp PBenchmarkSets.hpp ../pdatastructures/TMZipTreeSet.hpp ../ptms/onefile/OneFilePTMLF.hpp
	$(CXX) $(CXXFLAGS) -DUSE_OFLF $(INCLUDES) pset-ziptree-1m.cpp -o bin/pset-ziptree-1m-oflf -lpthread


#
# Multithreaded Relaxed AVL Tree Set 1M
#
bin/pset-ravl-1m-romlogfc: pset-ravl-1m.cpp PBenchmarkSets.hpp ../pdatastructures/TMRAVLSet.hpp ../ptms/romuluslog/RomLogFC.hpp
	$(CXX) $(CXXFLAGS) -DUSE_ROM_LOG_FC $(INCLUDES) pset-ravl-1m.cpp -o bin/pset-ravl-1m-romlogfc -lpthread

bin/pset-ravl-1m-undologfc: pset-ravl-1m.cpp PBenchmarkSets.hpp ../pdatastructures/TMRAVLSet.hpp ../ptms/undolog/UndoLogFC.hpp
	$(CXX) $(CXXFLAGS) -DUSE_UNDO_LOG_FC $(INCLUDES) pset-ravl-1m.cpp -o bin/pset-ravl-1m-undologfc -lpthread

bin/pset-ravl-1m-redologfc: pset-ravl-1m.cpp PBenchmarkSets.hpp ../pdatastructures/TMRAVLSet.hpp ../ptms/redolog/RedoLogFC.hpp
	$(CXX) $(CXXFLAGS) -DUSE_REDO_LOG_FC $(INCLUDES) pset-ravl-1m.cpp -o bin/pset-ravl-1m-redologfc -lpthread

bin/pset-ravl-1m-trinityfc: pset-ravl-1m.cpp PBenchmarkSets.hpp ../pdatastructures/TMRAVLSet.hpp ../ptms/trinity/TrinityFC.hpp
	$(CXX) $(CXXFLAGS) -DUSE_TRINITY_FC $(INCLUDES) pset-ravl-1m.cpp -o bin/pset-ravl-1m-trinityfc -lpthread

bin/pset-ravl-1m-trinityvrfc: pset-ravl-1m.cpp PBenchmarkSets.hpp ../pdatastructures/TMRAVLSet.hpp ../ptms/trinity/TrinityVRFC.hpp
	$(CXX) $(CXXFLAGS) -DUSE_TRINITY_VR_FC $(INCLUDES) pset-ravl-1m.cpp -o bin/pset-ravl-1m-trinityvrfc -lpthread

bin/pset-ravl-1m-trinitytl2: pset-ravl-1m.cpp PBenchmarkSets.hpp ../pdatastructures/TMRAVLSet.hpp ../ptms/trinity/TrinityTL2.hpp
	$(CXX) $(CXXFLAGS) -DUSE_TRINITY_TL2 $(INCLUDES) pset-ravl-1m.cpp -o bin/pset-ravl-1m-trinitytl2 -lpthread

bin/pset-ravl-1m-trinityvrtl2: pset-ravl-1m.cpp PBenchmarkSets.hpp ../pdatastructures/TMRAVLSet.hpp ../ptms/trinity/TrinityVRTL2.hpp
	$(CXX) $(CXXFLAGS) -DUSE_TRINITY_VR_TL2 $(INCLUDES) pset-ravl-1m.cpp -o bin/pset-ravl-1m-trinityvrtl2 -lpthread

bin/pset-ravl-1m-trinityvrtl2pl: pset-ravl-1m.cpp PBenchmarkSets.hpp ../pdatastructures/TMRAVLSet.hpp ../ptms/trinity/TrinityVRTL2PL.hpp
	$(CXX) $(CXXFLAGS) -DUSE_TRINITY_VR_TL2_PL $(INCLUDES) pset-ravl-1m.cpp -o bin/pset-ravl-1m-trinityvrtl2pl -lpthread

bin/pset-ravl-1m-quadrafc: pset-ravl-1m.cpp PBenchmarkSets.hpp ../pdatastructures/TMRAVLSet.hpp ../ptms/quadra/QuadraFC.hpp
	$(CXX) $(CXXFLAGS) -DUSE_QUADRA_FC $(INCLUDES) pset-ravl-1m.cpp -o bin/pset-ravl-1m-quadrafc -lpthread

bin/pset-ravl-1m-quadravrfc: pset-ravl-1m.cpp PBenchmarkSets.hpp ../pdatastructures/TMRAVLSet.hpp ../ptms/quadra/QuadraVRFC.hpp
	$(CXX) $(CXXFLAGS) -DUSE_QUADRA_VR_FC $(INCLUDES) pset-ravl-1m.cpp -o bin/pset-ravl-1m-quadravrfc -lpthread

bin/pset-ravl-1m-duovrfc: pset-ravl-1m.cpp PBenchmarkSets.hpp ../pdatastructures/TMRAVLSet.hpp ../ptms/duo/DuoVRFC.hpp
	$(CXX) $(CXXFLAGS) -DUSE_DUO_VR_FC $(INCLUDES) pset-ravl-1m.cpp -o bin/pset-ravl-1m-duovrfc -lpthread

bin/pset-ravl-1m-pmdk: pset-ravl-1m.cpp PBenchmarkSets.hpp ../pdatastructures/TMRAVLSet.hpp ../ptms/pmdk/PMDKTM.hpp
	$(CXX) $(CXXFLAGS) -DUSE_PMDK $(INCLUDES) pset-ravl-1m.cpp -o bin/pset-ravl-1m-pmdk -lpthread -lpmemobj

bin/pset-ravl-1m-oflf: pset-ravl-1m.cpp PBenchmarkSets.hpp ../pdatastructures/TMRAVLSet.hpp ../ptms/onefile/OneFilePTMLF.hpp
	$(CXX) $(CXXFLAGS) -DUSE_OFLF $(INCLUDES) pset-ravl-1m.cpp -o bin/pset-ravl-1m-oflf -lpthread

