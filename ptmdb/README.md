# Romulus DB #
A persistent in-memory key-value store for usage with NVRAM.
RomulusDB uses the same API as LevelDB.
At the core of RomulusDB is an Hash Map data structure protected with the RomulusLog persistent transactional memory (PTM).


## How to run ##
To execute the LevelDB benchmarks for RomulusDB, do this:
... 
    make db_bench
    ./db_bench
or
    ./db_bench --benchmarks=fillbatch,fillsync
    ./db_bench --threads=4
    ./db_bench --reads=10000000
or 
    ./bench.sh
...

To compare the results with LevelDB:
...
    cd ~/leveldb
    make
    out-static/db_bench
...

To run leveldb on a shared memory file system, do this:
...
    mkdir /dev/shn/leveldb
    ./db_bench --db=/dev/shm/leveldb
...


## Future possible improvements ##
- Add a node pool to TMHashMap;
- Add an intrusive doubly-linked list so that we can iterate forward and backward;

