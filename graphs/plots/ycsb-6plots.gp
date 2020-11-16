set term postscript color eps enhanced 22                                                       
set output "ycsb-6plots.eps"                                                               

set size 0.95,1.12

X=0.1
W=0.26
M=0.025

load "styles.inc"

set tmargin 0
set bmargin 3

set multiplot layout 2,3

unset key

set grid ytics

set xtics ("" 1, "" 2, 4, 8, 16, 24, 32, 40) nomirror out offset -0.25,0.5
set label at screen 0.5,0.04 center "Number of threads"        
set label at screen 0.5,1.09 center "K/V stores"


#
# First row of plots
#

set lmargin at screen X
set rmargin at screen X+W

set ylabel offset 1.0,0 "Operations ({/Symbol \264}10^6/s)"
set ytics 1 offset 0.5,0                                 
set yrange [0:9]                                         

set key at graph 0.99,0.99 samplen 1.5
set label at graph 0.5,1.075 center font "Helvetica-bold,18" "YCSB-A (1M keys)"
plot \
    '../data/castor/ycsb-1m-trinvrfc.txt'  using 1:($2/1e6)  with linespoints notitle ls 8  lw 4 dt 1,     \
    '../data/castor/ycsb-1m-trinvrtl2.txt' using 1:($2/1e6)  with linespoints notitle ls 10 lw 4 dt 1,     \
    '../data/castor/ycsb-1m-pronto.txt'    using 1:($2/1e6)  with linespoints notitle ls 11 lw 3 dt (1,1), \
    '../data/castor/ycsb-1m-redodb.txt'    using 1:($2/1e6)  with linespoints notitle ls 12 lw 3 dt (1,1), \
    '../data/castor/ycsb-1m-rocksdb.txt'   using 1:($2/1e6)  with linespoints notitle ls 2  lw 3 dt (1,1), \
    '../data/castor/ycsb-1m-pmemkv.txt'    using 1:($2/1e6)  with linespoints notitle ls 1  lw 3 dt (1,1)

unset ylabel
set ytics format ""

set lmargin at screen X+(W+M)
set rmargin at screen X+(W+M)+W

unset label
set label at graph 0.5,1.075 center font "Helvetica-bold,18" "YCSB-A (10M keys)"
plot \
    '../data/castor/ycsb-10m-trinvrfc.txt'  using 1:($2/1e6)  with linespoints notitle ls 8  lw 4 dt 1,     \
    '../data/castor/ycsb-10m-trinvrtl2.txt' using 1:($2/1e6)  with linespoints notitle ls 10 lw 4 dt 1,     \
    '../data/castor/ycsb-10m-redodb.txt'    using 1:($2/1e6)  with linespoints notitle ls 12 lw 3 dt (1,1), \
    '../data/castor/ycsb-10m-rocksdb.txt'   using 1:($2/1e6)  with linespoints notitle ls 2  lw 3 dt (1,1), \
    '../data/castor/ycsb-10m-pmemkv.txt'    using 1:($2/1e6)  with linespoints notitle ls 1  lw 3 dt (1,1)

set lmargin at screen X+2*(W+M)
set rmargin at screen X+2*(W+M)+W

unset label
set label at graph 0.5,1.075 center font "Helvetica-bold,18" "YCSB-A (100M keys)"
plot \
    '../data/castor/ycsb-100m-trinvrfc.txt'  using 1:($2/1e6)  with linespoints notitle ls 8  lw 4 dt 1,     \
    '../data/castor/ycsb-100m-trinvrtl2.txt' using 1:($2/1e6)  with linespoints notitle ls 10 lw 4 dt 1,     \
    '../data/castor/ycsb-100m-rocksdb.txt'   using 1:($2/1e6)  with linespoints notitle ls 2  lw 3 dt (1,1), \
    '../data/castor/ycsb-100m-pmemkv.txt'    using 1:($2/1e6)  with linespoints notitle ls 1  lw 3 dt (1,1)



#
# Second row of plots
#

set lmargin at screen X
set rmargin at screen X+W

set ylabel offset 0.5,0 "Operations ({/Symbol \264}10^6/s)"
set ytics 5 offset 0.5,0                                   
set ytics format "%g"                                      
set yrange [0:30]                                           

unset label
set label at graph 0.5,1.075 center font "Helvetica-bold,18" "YCSB-B (1M keys)"

plot \
    '../data/castor/ycsb-1m-trinvrfc.txt'  using 1:($3/1e6)  with linespoints notitle ls 8  lw 4 dt 1,     \
    '../data/castor/ycsb-1m-trinvrtl2.txt' using 1:($3/1e6)  with linespoints notitle ls 10 lw 4 dt 1,     \
    '../data/castor/ycsb-1m-pronto.txt'    using 1:($3/1e6)  with linespoints notitle ls 11 lw 3 dt (1,1), \
    '../data/castor/ycsb-1m-redodb.txt'    using 1:($3/1e6)  with linespoints notitle ls 12 lw 3 dt (1,1), \
    '../data/castor/ycsb-1m-rocksdb.txt'   using 1:($3/1e6)  with linespoints notitle ls 2  lw 3 dt (1,1), \
    '../data/castor/ycsb-1m-pmemkv.txt'    using 1:($3/1e6)  with linespoints notitle ls 1  lw 3 dt (1,1)

unset ylabel
set ytics format ""

set lmargin at screen X+(W+M)
set rmargin at screen X+(W+M)+W

unset label
set label at graph 0.5,1.075 center font "Helvetica-bold,18" "YCSB-B (10M keys)"

plot \
    '../data/castor/ycsb-10m-trinvrfc.txt'  using 1:($3/1e6)  with linespoints notitle ls 8  lw 4 dt 1,     \
    '../data/castor/ycsb-10m-trinvrtl2.txt' using 1:($3/1e6)  with linespoints notitle ls 10 lw 4 dt 1,     \
    '../data/castor/ycsb-10m-redodb.txt'    using 1:($3/1e6)  with linespoints notitle ls 12 lw 3 dt (1,1), \
    '../data/castor/ycsb-10m-rocksdb.txt'   using 1:($3/1e6)  with linespoints notitle ls 2  lw 3 dt (1,1), \
    '../data/castor/ycsb-10m-pmemkv.txt'    using 1:($3/1e6)  with linespoints notitle ls 1  lw 3 dt (1,1)

set lmargin at screen X+2*(W+M)
set rmargin at screen X+2*(W+M)+W

unset label
#set ytics 20 offset 0.5,0
#set yrange [0:30]
#set style textbox opaque noborder fillcolor rgb "white"
#set label at first 1,180 front boxed left offset -0.5,0 "180"
set label at graph 0.5,1.075 center font "Helvetica-bold,18" "YCSB-B (100M keys)"

plot \
    '../data/castor/ycsb-100m-trinvrfc.txt'  using 1:($3/1e6)  with linespoints notitle ls 8  lw 4 dt 1,     \
    '../data/castor/ycsb-100m-trinvrtl2.txt' using 1:($3/1e6)  with linespoints notitle ls 10 lw 4 dt 1,     \
    '../data/castor/ycsb-100m-rocksdb.txt'   using 1:($3/1e6)  with linespoints notitle ls 2  lw 3 dt (1,1), \
    '../data/castor/ycsb-100m-pmemkv.txt'    using 1:($3/1e6)  with linespoints notitle ls 1  lw 3 dt (1,1)

unset tics
unset border
unset xlabel
unset ylabel
unset label

set key at screen 0.36,0.82 samplen 2.0 bottom font "Helvetica Condensed" spacing 1.3
plot [][0:1] \
    2 with linespoints title 'TrinityDB (TL2)'       ls 10 lw 4 dt 1,     \
    2 with linespoints title 'TrinityDB (FC)'        ls 8  lw 4 dt 1,     \
    2 with linespoints title 'RedoDB'                ls 12 lw 3 dt (1,1)

set key at screen 0.35,0.32 samplen 2.0 bottom font "Helvetica Condensed" spacing 1.3
plot [][0:1] \
    2 with linespoints title 'pronto (sync)'         ls 11 lw 3 dt (1,1), \
    2 with linespoints title 'RocksDB'               ls 2  lw 3 dt (1,1), \
    2 with linespoints title 'pmemkv'                ls 1  lw 3 dt (1,1)

