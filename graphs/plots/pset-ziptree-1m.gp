set term postscript color eps enhanced 22
set output "pset-ziptree-1m.eps"

set size 0.95,0.54

X=0.1
W=0.26
M=0.025

load "styles.inc"

set tmargin 11.8
set bmargin 2.5

set multiplot layout 1,3

unset key

set grid ytics

set xtics ("" 1, "" 2, 4, 8, 16, 32, 24, 40) nomirror out offset -0.25,0.5
set label at screen 0.5,0.03 center "Number of threads"
#set label at screen 0.5,0.57 center "Persistent Zip-Tree with 10^{6} keys"

set xrange [1:40]

# First row

set lmargin at screen X
set rmargin at screen X+W

set ylabel offset -0.5,0 "Tx ({/Symbol \264}10^6/s)"
set ytics 2 offset 0.5,0
set format y "%g"
set yrange [0:6]
set grid mytics

set label at graph 0.5,1.075 center font "Helvetica-bold,16" "in=50% de=50% lk=0%"

set key at graph 0.99,0.99 samplen 1.5

plot \
    '../data/castor/pset-ziptree-1m-pmdk.txt'           using 1:($2/1e6) with linespoints notitle ls 1  lw 3 dt (1,1), \
    '../data/castor/pset-ziptree-1m-oflf.txt'           using 1:($2/1e6) with linespoints notitle ls 2  lw 3 dt (1,1), \
    '../data/castor/pset-ziptree-1m-romlogfc.txt'       using 1:($2/1e6) with linespoints notitle ls 4  lw 3 dt (1,1), \
    '../data/castor/pset-ziptree-1m-trinfc.txt'         using 1:($2/1e6) with linespoints notitle ls 6  lw 3 dt 1,     \
    '../data/castor/pset-ziptree-1m-quadrafc.txt'       using 1:($2/1e6) with linespoints notitle ls 7  lw 3 dt 1,     \
    '../data/castor/pset-ziptree-1m-trinvrfc.txt'       using 1:($2/1e6) with linespoints notitle ls 8  lw 3 dt 1,     \
    '../data/castor/pset-ziptree-1m-quadravrfc.txt'     using 1:($2/1e6) with linespoints notitle ls 9  lw 3 dt 1,     \
    '../data/castor/pset-ziptree-1m-trinvrtl2.txt'      using 1:($2/1e6) with linespoints notitle ls 10 lw 3 dt 1
#    '../data/castor/pset-ziptree-1m-duovrfc.txt'        using 1:($2/1e6) with linespoints notitle ls 13 lw 3 dt 1,     \

set mytics 1
set grid nomytics

unset ylabel
set ytics format ""

set lmargin at screen X+(W+M)
set rmargin at screen X+(W+M)+W

unset label
set ytics 2 offset 0.5,0
set yrange [0:12]
set style textbox opaque noborder fillcolor rgb "white"
set label at first 1,12 front boxed left offset -0.5,0 "12"
set label at graph 0.5,1.075 center font "Helvetica-bold,16" "in=5% de=5% lk=90%"

plot \
    '../data/castor/pset-ziptree-1m-pmdk.txt'           using 1:($3/1e6) with linespoints notitle ls 1  lw 3 dt (1,1), \
    '../data/castor/pset-ziptree-1m-oflf.txt'           using 1:($3/1e6) with linespoints notitle ls 2  lw 3 dt (1,1), \
    '../data/castor/pset-ziptree-1m-romlogfc.txt'       using 1:($3/1e6) with linespoints notitle ls 4  lw 3 dt (1,1), \
    '../data/castor/pset-ziptree-1m-trinfc.txt'         using 1:($3/1e6) with linespoints notitle ls 6  lw 3 dt 1,     \
    '../data/castor/pset-ziptree-1m-quadrafc.txt'       using 1:($3/1e6) with linespoints notitle ls 7  lw 3 dt 1,     \
    '../data/castor/pset-ziptree-1m-trinvrfc.txt'       using 1:($3/1e6) with linespoints notitle ls 8  lw 3 dt 1,     \
    '../data/castor/pset-ziptree-1m-quadravrfc.txt'     using 1:($3/1e6) with linespoints notitle ls 9  lw 3 dt 1,     \
    '../data/castor/pset-ziptree-1m-trinvrtl2.txt'      using 1:($3/1e6) with linespoints notitle ls 10 lw 3 dt 1
#    '../data/castor/pset-ziptree-1m-duovrfc.txt'        using 1:($3/1e6) with linespoints notitle ls 13 lw 3 dt 1,     \


set lmargin at screen X+2*(W+M)
set rmargin at screen X+2*(W+M)+W

unset label
set ytics 2 offset 0.5,0
set yrange [0:16]
set style textbox opaque noborder fillcolor rgb "white"
set label at first 1,16 front boxed left offset -0.5,0 "16"
set label at graph 0.5,1.075 center font "Helvetica-bold,16" "in=.5% de=.5% lk=99%"

plot \
    '../data/castor/pset-ziptree-1m-pmdk.txt'           using 1:($4/1e6) with linespoints notitle ls 1  lw 3 dt (1,1), \
    '../data/castor/pset-ziptree-1m-oflf.txt'           using 1:($4/1e6) with linespoints notitle ls 2  lw 3 dt (1,1), \
    '../data/castor/pset-ziptree-1m-romlogfc.txt'       using 1:($4/1e6) with linespoints notitle ls 4  lw 3 dt (1,1), \
    '../data/castor/pset-ziptree-1m-trinfc.txt'         using 1:($4/1e6) with linespoints notitle ls 6  lw 3 dt 1,     \
    '../data/castor/pset-ziptree-1m-quadrafc.txt'       using 1:($4/1e6) with linespoints notitle ls 7  lw 3 dt 1,     \
    '../data/castor/pset-ziptree-1m-trinvrfc.txt'       using 1:($4/1e6) with linespoints notitle ls 8  lw 3 dt 1,     \
    '../data/castor/pset-ziptree-1m-quadravrfc.txt'     using 1:($4/1e6) with linespoints notitle ls 9  lw 3 dt 1,     \
    '../data/castor/pset-ziptree-1m-trinvrtl2.txt'      using 1:($4/1e6) with linespoints notitle ls 10 lw 3 dt 1
#    '../data/castor/pset-ziptree-1m-duovrfc.txt'        using 1:($4/1e6) with linespoints notitle ls 13 lw 3 dt 1,     \



unset tics
unset border
unset xlabel
unset ylabel
unset label
	
unset multiplot
    
