# Durable Disjoint Transactions for Persistent Memory.

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
    ptms/romuluslog/RomLog2FFC.hpp  RomulusLog 2 Fences (2 fences)
    ptms/undolog/UndoLogFC.hpp      Undo-Log durability
    ptms/undolog/UndoLogTSFC.hpp    Undo-Log durability with one fence per modification
    
PTMs whose name ends in "FC" use Flat Combining for concurrency. PTMs whose name ends in "TL2" use the TL2 concurrency control for concurrency.
The VR variants have a volatile replica of the user data and they also support any user type, while the others are meant for 64bit sized user types only.

## Building
To build, go into the graphs/ folder, modify the Makefile to have the path of where your Optane PM device is located and then type make:
    
    cd graphs/
    make -j20


## Paper
The paper describing all these algorithms can be obtained at xarchives.