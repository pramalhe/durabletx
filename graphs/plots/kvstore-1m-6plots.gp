set term postscript color eps enhanced 22                                                       
set output "kvstore-1m-6plots.eps"                                                               

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
set label at screen 0.5,1.09 center "K/V store with 10^{6} keys"


#
# First row of plots
#

set lmargin at screen X
set rmargin at screen X+W

set ylabel offset 2.5,0 "Operations ({/Symbol \264}10^6/s)"
set ytics 0.5 offset 0.5,0                                 
set yrange [0:2.5]                                         

set key at graph 0.99,0.99 samplen 1.5
set label at graph 0.5,1.075 center font "Helvetica-bold,18" "fillseq"
plot \
    '../data/castor/db-1m-trinvrfc.txt'  using 1:($1/$4)  with linespoints notitle ls 8  lw 4 dt 1,     \
    '../data/castor/db-1m-trinvrtl2.txt' using 1:($1/$4)  with linespoints notitle ls 10 lw 4 dt 1,     \
    '../data/castor/db-1m-rocksdb.txt'   using 1:($1/$4)  with linespoints notitle ls 2  lw 4 dt (1,1), \
    '../data/castor/db-1m-pmemkv.txt'    using 1:($1/$4)  with linespoints notitle ls 1  lw 4 dt (1,1)

unset ylabel
set ytics format ""

set lmargin at screen X+(W+M)
set rmargin at screen X+(W+M)+W

unset label
set label at graph 0.5,1.075 center font "Helvetica-bold,18" "fillrandom"
plot \
    '../data/castor/db-1m-trinvrfc.txt'  using 1:($1/$2)  with linespoints notitle ls 8  lw 4 dt 1,     \
    '../data/castor/db-1m-trinvrtl2.txt' using 1:($1/$2)  with linespoints notitle ls 10 lw 4 dt 1,     \
    '../data/castor/db-1m-rocksdb.txt'   using 1:($1/$2)  with linespoints notitle ls 2  lw 4 dt (1,1), \
    '../data/castor/db-1m-pmemkv.txt'    using 1:($1/$2)  with linespoints notitle ls 1  lw 4 dt (1,1)

set lmargin at screen X+2*(W+M)
set rmargin at screen X+2*(W+M)+W

unset label
set label at graph 0.5,1.075 center font "Helvetica-bold,18" "overwrite"
plot \
    '../data/castor/db-1m-trinvrfc.txt'  using 1:($1/$3)  with linespoints notitle ls 8  lw 4 dt 1,     \
    '../data/castor/db-1m-trinvrtl2.txt' using 1:($1/$3)  with linespoints notitle ls 10 lw 4 dt 1,     \
    '../data/castor/db-1m-rocksdb.txt'   using 1:($1/$3)  with linespoints notitle ls 2  lw 4 dt (1,1), \
    '../data/castor/db-1m-pmemkv.txt'    using 1:($1/$3)  with linespoints notitle ls 1  lw 4 dt (1,1)



#
# Second row of plots
#

set lmargin at screen X
set rmargin at screen X+W

set ylabel offset 0.5,0 "Operations ({/Symbol \264}10^6/s)"
set ytics 5 offset 0.5,0                                   
set ytics format "%g"                                      
set yrange [0:20]                                           

unset label
set label at graph 0.5,1.075 center font "Helvetica-bold,18" "readrandom"

plot \
    '../data/castor/db-1m-trinvrfc.txt'  using 1:($1/$5)  with linespoints notitle ls 8  lw 4 dt 1,     \
    '../data/castor/db-1m-trinvrtl2.txt' using 1:($1/$5)  with linespoints notitle ls 10 lw 4 dt 1,     \
    '../data/castor/db-1m-rocksdb.txt'   using 1:($1/$5)  with linespoints notitle ls 2  lw 4 dt (1,1), \
    '../data/castor/db-1m-pmemkv.txt'    using 1:($1/$5)  with linespoints notitle ls 1  lw 4 dt (1,1)

unset ylabel
set ytics format ""

set lmargin at screen X+(W+M)
set rmargin at screen X+(W+M)+W

unset label
set label at graph 0.5,1.075 center font "Helvetica-bold,18" "readwhilewriting"

plot \
    '../data/castor/db-1m-trinvrfc.txt'  using 1:($1/$6)  with linespoints notitle ls 8  lw 4 dt 1,     \
    '../data/castor/db-1m-trinvrtl2.txt' using 1:($1/$6)  with linespoints notitle ls 10 lw 4 dt 1,     \
    '../data/castor/db-1m-rocksdb.txt'   using 1:($1/$6)  with linespoints notitle ls 2  lw 4 dt (1,1), \
    '../data/castor/db-1m-pmemkv.txt'    using 1:($1/$6)  with linespoints notitle ls 1  lw 4 dt (1,1)

set lmargin at screen X+2*(W+M)
set rmargin at screen X+2*(W+M)+W

unset label
set ytics 20 offset 0.5,0
set yrange [0:180]
set style textbox opaque noborder fillcolor rgb "white"
set label at first 1,180 front boxed left offset -0.5,0 "180"
set label at graph 0.5,1.075 center font "Helvetica-bold,18" "fillseekseq"

plot \
    '../data/castor/db-1m-trinvrfc.txt'  using 1:($1/$7)  with linespoints notitle ls 8  lw 4 dt 1,     \
    '../data/castor/db-1m-trinvrtl2.txt' using 1:($1/$7)  with linespoints notitle ls 10 lw 4 dt 1,     \
    '../data/castor/db-1m-rocksdb.txt'   using 1:($1/$7)  with linespoints notitle ls 2  lw 4 dt (1,1), \
    '../data/castor/db-1m-pmemkv.txt'    using 1:($1/$7)  with linespoints notitle ls 1  lw 4 dt (1,1)

unset tics
unset border
unset xlabel
unset ylabel
unset label

#set key at screen 0.64,0.32 samplen 2.0 bottom font "Helvetica Condensed"
set key at screen 0.36,0.87 samplen 2.0 bottom font "Helvetica Condensed" spacing 1.3
plot [][0:1] \
    2 with linespoints title 'TrinityDB (TL2)'       ls 10 lw 4 dt 1,     \
    2 with linespoints title 'TrinityDB (FC)'        ls 8  lw 4 dt 1

set key at screen 0.65,0.87 samplen 2.0 bottom font "Helvetica Condensed" spacing 1.3
plot [][0:1] \
    2 with linespoints title 'RocksDB'               ls 2  lw 4 dt (1,1), \
    2 with linespoints title 'pmemkv'                ls 1  lw 4 dt (1,1)

