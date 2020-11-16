set term postscript color eps enhanced 22
set output "ycsb-1m.eps"

set size 0.95,0.55

X=0.08
W=0.26
M=0.06

load "styles.inc"

set tmargin 11.5
set bmargin 2.5

set grid ytics

set ylabel offset 1.5,0 "Operations ({/Symbol \264}10^6/s)" font "Helvetica Condensed"
#set ylabel offset 2.0,0 "{/Symbol m}s/operation"
#set format y "10^{%T}"
set xtics ("" 1, "" 2, 4, 8, 16, "" 20, 24, 32, 40) nomirror out offset -0.25,0.5 font "Helvetica Condensed"
set xlabel offset 0,0.75 "Number of threads" font "Helvetica Condensed"

set multiplot layout 1,3

set yrange [0:4]
set ytics 1 offset 0.6,0 font "Helvetica Condensed"
set lmargin at screen X
set rmargin at screen X+W

set label at graph 0.5,1.075 center font "Helvetica-bold,18" "YCSB-A"
unset key

plot \
    '../data/castor/ycsb-1m-trinvrfc.txt'  using 1:($2/1e6)  with linespoints notitle ls 8  lw 4 dt 1,     \
    '../data/castor/ycsb-1m-trinvrtl2.txt' using 1:($2/1e6)  with linespoints notitle ls 10 lw 4 dt 1,     \
    '../data/castor/ycsb-1m-rocksdb.txt'   using 1:($2/1e6)  with linespoints notitle ls 2  lw 3 dt (1,1), \
    '../data/castor/ycsb-1m-pmemkv.txt'    using 1:($2/1e6)  with linespoints notitle ls 1  lw 3 dt (1,1), \
    '../data/castor/ycsb-1m-redodb.txt'    using 1:($2/1e6)  with linespoints notitle ls 12 lw 3 dt (1,1), \
    '../data/castor/ycsb-1m-pronto.txt'    using 1:($2/1e6)  with linespoints notitle ls 11 lw 3 dt (1,1)
	
unset ylabel
#set ytics format "%g"

set yrange [0:20]
set ytics 5 offset 0.6,0 font "Helvetica Condensed"
set lmargin at screen X+(W+M)
set rmargin at screen X+(W+M)+W

unset label
set label at graph 0.5,1.075 center font "Helvetica-bold,18" "YCSB-B"

plot \
    '../data/castor/ycsb-1m-trinvrfc.txt'  using 1:($3/1e6)  with linespoints notitle ls 8  lw 4 dt 1,     \
    '../data/castor/ycsb-1m-trinvrtl2.txt' using 1:($3/1e6)  with linespoints notitle ls 10 lw 4 dt 1,     \
    '../data/castor/ycsb-1m-rocksdb.txt'   using 1:($3/1e6)  with linespoints notitle ls 2  lw 3 dt (1,1), \
    '../data/castor/ycsb-1m-pmemkv.txt'    using 1:($3/1e6)  with linespoints notitle ls 1  lw 3 dt (1,1), \
    '../data/castor/ycsb-1m-redodb.txt'    using 1:($3/1e6)  with linespoints notitle ls 12 lw 3 dt (1,1), \
    '../data/castor/ycsb-1m-pronto.txt'    using 1:($3/1e6)  with linespoints notitle ls 11 lw 3 dt (1,1)

unset ylabel
#set ytics format ""
unset tics
unset border
unset xlabel
unset ylabel
unset label

set key at screen 0.94,0.20 samplen 2.0 bottom font "Helvetica Condensed"
plot [][0:1] \
    2 with linespoints title 'TrinityDB (TL2)'       ls 10 lw 4 dt 1,     \
    2 with linespoints title 'TrinityDB (FC)'        ls 9  lw 4 dt 1,     \
    2 with linespoints title 'RocksDB'               ls 2  lw 4 dt (1,1), \
    2 with linespoints title 'pmemkv'                ls 1  lw 4 dt (1,1), \
    2 with linespoints title 'RedoDB'                ls 12 lw 2 dt (1,1), \
    2 with linespoints title 'Pronto (sync)'         ls 11 lw 2 dt (1,1)

unset multiplot
