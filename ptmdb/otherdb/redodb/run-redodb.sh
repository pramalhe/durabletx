#!/bin/sh

make persistencyclean
sleep 2
echo threads=1
bin/db_bench_redoopt --benchmarks=fillrandom,overwrite,fillseq,readrandom,readwhilewriting --num=10000000 --threads=1
make persistencyclean
sleep 2
echo threads=2
bin/db_bench_redoopt --benchmarks=fillrandom,overwrite,fillseq,readrandom,readwhilewriting --num=10000000 --threads=2
make persistencyclean
sleep 2
echo threads=4
bin/db_bench_redoopt --benchmarks=fillrandom,overwrite,fillseq,readrandom,readwhilewriting --num=10000000 --threads=4
make persistencyclean
sleep 2
echo threads=8
bin/db_bench_redoopt --benchmarks=fillrandom,overwrite,fillseq,readrandom,readwhilewriting --num=10000000 --threads=8
make persistencyclean
sleep 2
echo threads=16
bin/db_bench_redoopt --benchmarks=fillrandom,overwrite,fillseq,readrandom,readwhilewriting --num=10000000 --threads=16
make persistencyclean
sleep 2
echo threads=24
bin/db_bench_redoopt --benchmarks=fillrandom,overwrite,fillseq,readrandom,readwhilewriting --num=10000000 --threads=24
make persistencyclean
sleep 2
echo threads=32
bin/db_bench_redoopt --benchmarks=fillrandom,overwrite,fillseq,readrandom,readwhilewriting --num=10000000 --threads=32
make persistencyclean
sleep 2
echo threads=40
bin/db_bench_redoopt --benchmarks=fillrandom,overwrite,fillseq,readrandom,readwhilewriting --num=10000000 --threads=40
make persistencyclean
