# Efficient Algorithms for Persistent Transactional Memory

This repository contains several PTMs and the TrinityDB K/V store.

This is joint work by Pedro Ramalhete, Andreia Correia and Pascal Felber.


## PTMs
This repository contains the source code and paper for multiple STMs capable of providing durable transactions in Persistent Memory (PM):

    ptms/quadra/QuadraFC.hpp        Quadra (1 fence)
    ptms/quadra/QuadraVRFC.hpp      Quadra with Volatile Replica (1 fence)
    ptms/trinity/TrinityFC.hpp      Trinity (2 fences)
    ptms/trinity/TrinityVRFC.hpp    Trinity with Volatile Replica (2 fences)
    ptms/trinity/TrinityVRTL2.hpp   Trinity with Volatile Replica and TL2
    ptms/romuluslog/RomLogFC.hpp    RomulusLog (4 fences)
    
PTMs whose name ends in "FC" use Flat Combining for concurrency. PTMs whose name ends in "TL2" use our own implementation of the TL2 concurrency control for concurrency.
The VR variants have a volatile replica of the user data and they also support any user type, while the others are meant for 64bit sized user types only.
For simplicity of usage, each PTM is implemented as a single C++ header file.
All these PTMS provide durable linearizable (ACID) transactions.

If you don't know what to chose, then take TrinityVRTL2, which can execute disjoint transactions and has fast durability.


## Building
To build, go into the graphs/ folder, modify the Makefile to have the path of where your Optane PM device is located and then type make:
    
    cd graphs/
    make -j20

If you're on an ubuntu system you may need some packages:

    sudo apt-get update
    sudo apt-get install g++-9 gcc-9 python make


#### Do I need PMDK to build?
Yes if you want to compare with PMDK, but No if you only want to build and run our PTMs.
We have also included updated versions of OneFile and Romulus in the repository.
If you don't have PMDK installed then the simplest way to build is to open up the Makefile and comment out the two targets named 'bin-pmdk'.

#### I don't have any Optane Persistent Memory, can I run on DRAM just to experiment?
Yes you can, this is the default when you run make.

#### How do I specify a different memory mapped file for the persistent region?
Edit the Makefile and replace the filename in PM\_FILE\_NAME with a the path you want: 

	-DPM_FILE_NAME="\"/mnt/pmem0/durable\""

#### How do I create a larger mapped file/region of persistent memory?
Edit the Makefile and increase the default size of 4 GB to something else 

	-DPM_REGION_SIZE=4*1024*1024*1024ULL

and make sure to enable flushing to PM by passing the option

    -DPWB_IS_CLWB


## Running the benchmarks

We have two kinds of benchmarks, 'run-seq.py' for sequential data structures and transactions (non concurrent), and 'run-conc.py' for concurrent data structures (which use fully ACID transactions). 

    ./run-seq.py
    ./run-conc.py


## Using one of these PTMs in your application

If all you want to do is use one of these PTMs in your application, then don't even bother compiling, just take the header file of the corresponding PTM and include it in your code.
If you don't know what to chose, then take ptms/trinity/TrinityVRTL2.hpp, which can execute disjoint transactions and has fast durability.

Transactions need to be passed over in a lambda, for example:

    #include <ptms/trinity/TrinityVRTL2.hpp>
    using namespace trinityvrtl2;
    struct Foo {
        persist<int> a;
        persist<int> b;
	};
	
	updateTx([&] () {
	    Foo* foo = tmNew<Foo>();
	    foo->a = 1;
       foo->b = 2;
    });    


## Paper
The paper describing the Trinity and Quadra algorithms will appear in PPoPP 2021, "Efficient Algorithms for Persistent Transactional Memory"
