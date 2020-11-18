# Efficient Algorithms for Persistent Transactional Memory

This repository contains several PTMs.

This is joint work by Pedro Ramalhete, Andreia Correia and Pascal Felber.

## PTMs
This repository contains the source code and paper for multiple STMs capable of providing durable transactions in Persistent Memory (PM):

    ptms/quadra/QuadraFC.hpp        Quadra (1 fence)
    ptms/quadra/QuadraVRFC.hpp      Quadra with Volatile Replica (1 fence)
    ptms/trinity/TrinityFC.hpp      Trinity (2 fences)
    ptms/trinity/TrinityVRFC.hpp    Trinity with Volatile Replica(2 fences)
    ptms/redolog/RedoLogFC.hpp      Redo-Log (4 fences)
    ptms/romuluslog/RomLogFC.hpp    RomulusLog (4 fences)
    ptms/undolog/UndoLogFC.hpp      Undo-Log durability
    
PTMs whose name ends in "FC" use Flat Combining for concurrency. PTMs whose name ends in "TL2" use the TL2 concurrency control for concurrency.
The VR variants have a volatile replica of the user data and they also support any user type, while the others are meant for 64bit sized user types only.

## Building
To build, go into the graphs/ folder, modify the Makefile to have the path of where your Optane PM device is located and then type make:
    
    cd graphs/
    make -j20
	./run-seq.py
	./run-conc.py

### Do I need PMDK to build?
Yes if you want to compare with PMDK, but No if you only want to build and run our PTMs.
The simplest way is to open up the Makefile and comment out the two targets named 'bin-pmdk'.

### I don't have any Optane Persistent Memory, can I run on DRAM just to experiment?
Yes you can, this is the default when you run make.

### How do I specify a different memory mapped file for the persistent region?
Edit the Makefile and replace the filename in PM\_FILE\_NAME with a the path you want: 
	-DPM_FILE_NAME="\"/mnt/pmem0/durable\""

### How do I create a larger mapped file/region of persistent memory?
Edit the Makefile and increase the default size of 4 GB to something else 
	-DPM_REGION_SIZE=4*1024*1024*1024ULL


## Paper
The paper describing these algorithms will appear in PPoPP 2021, "Efficient Algorithms for Persistent Transactional Memory"
