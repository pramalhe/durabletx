#!/bin/sh

for i in \
pcaption.gp \
pq-ll-seq.gp \
pq-fat-seq.gp \
pset-ll-1k-seq.gp \
pset-ll-10k-seq.gp \
pset-tree-1m-seq.gp \
pset-btree-1m-seq.gp \
pset-hash-1m-seq.gp \
psps-integer-seq.gp \
psps-integer-seq-appendix.gp \
seqwrite-integer-seq.gp \
;
do
  echo "Processing:" $i
  gnuplot $i
  epstopdf `basename $i .gp`.eps
  rm `basename $i .gp`.eps
done
