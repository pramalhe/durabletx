#!/bin/sh

for i in \
pcaption.gp \
pq-ll.gp \
pq-fat.gp \
pstack-ll.gp \
pstack-fat.gp \
pset-tree-1m.gp \
pset-hash-1m.gp \
pset-btree-1m.gp \
pset-hashfixed-1m.gp \
pset-ziptree-1m.gp \
pset-ravl-1m.gp \
psps-integer.gp \
;
do
  echo "Processing:" $i
  gnuplot $i
  epstopdf `basename $i .gp`.eps
  rm `basename $i .gp`.eps
done
