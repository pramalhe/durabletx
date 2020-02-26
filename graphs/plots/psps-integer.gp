set term postscript color eps enhanced 22
set output "psps-integer.eps"

set size 0.95,1.10

X=0.1
W=0.26
M=0.025

load "styles.inc"

set tmargin 0
set bmargin 3

set multiplot layout 2,3

unset key

set grid ytics

set xtics ("" 1, "" 2, 4, "" 8, 16, 24, 32, "" 40) nomirror out offset -0.25,0.5
set label at screen 0.5,0.04 center "Number of threads (1,2,4,8,16,24,32,40)"
#set label at screen 0.5,1.075 center "SPS integer swap"

#set logscale x
set xrange [1:40]

# First row

set lmargin at screen X
set rmargin at screen X+W

#set ylabel offset 1,0 "Swaps ({/Symbol \264}10^6/s)"
set ylabel offset 2.5,0 "Tx ({/Symbol \264}10^6/s)"
set ytics 0.5 offset 0.5,0
set yrange [0:2.8]

set label at graph 0.5,1.075 center font "Helvetica-bold,18" "1 swaps/tx"

plot \
    '../data/castor/psps-integer-undologfc.txt'      using 1:($2/1e6) with linespoints notitle ls 1  lw 3 dt (1,1), \
    '../data/castor/psps-integer-redologfc.txt'      using 1:($2/1e6) with linespoints notitle ls 2  lw 3 dt (1,1), \
    '../data/castor/psps-integer-romlogfc.txt'       using 1:($2/1e6) with linespoints notitle ls 4  lw 3 dt (1,1), \
    '../data/castor/psps-integer-trinfc.txt'         using 1:($2/1e6) with linespoints notitle ls 6  lw 3 dt 1,     \
    '../data/castor/psps-integer-quadrafc.txt'       using 1:($2/1e6) with linespoints notitle ls 7  lw 3 dt 1,     \
    '../data/castor/psps-integer-trinvrfc.txt'       using 1:($2/1e6) with linespoints notitle ls 8  lw 3 dt 1,     \
    '../data/castor/psps-integer-quadravrfc.txt'     using 1:($2/1e6) with linespoints notitle ls 9  lw 3 dt 1,     \
    '../data/castor/psps-integer-trinvrtl2.txt'      using 1:($2/1e6) with linespoints notitle ls 10 lw 3 dt 1
#    '../data/castor/psps-integer-duovrfc.txt'        using 1:($2/1e6) with linespoints notitle ls 13 lw 3 dt 1,     \

unset ylabel
set ytics format ""

set lmargin at screen X+(W+M)
set rmargin at screen X+(W+M)+W

unset label
set ytics 0.5 offset 0.5,0
set yrange [0:2.8]
set style textbox opaque noborder fillcolor rgb "white"
set label at first 1,2.8 front boxed left offset -0.5,0 "2.8"
set label at graph 0.5,1.075 center font "Helvetica-bold,18" "4 swaps/tx"

plot \
    '../data/castor/psps-integer-undologfc.txt'      using 1:($3/1e6) with linespoints notitle ls 1  lw 3 dt (1,1), \
    '../data/castor/psps-integer-redologfc.txt'      using 1:($3/1e6) with linespoints notitle ls 2  lw 3 dt (1,1), \
    '../data/castor/psps-integer-romlogfc.txt'       using 1:($3/1e6) with linespoints notitle ls 4  lw 3 dt (1,1), \
    '../data/castor/psps-integer-trinfc.txt'         using 1:($3/1e6) with linespoints notitle ls 6  lw 3 dt 1,     \
    '../data/castor/psps-integer-quadrafc.txt'       using 1:($3/1e6) with linespoints notitle ls 7  lw 3 dt 1,     \
    '../data/castor/psps-integer-trinvrfc.txt'       using 1:($3/1e6) with linespoints notitle ls 8  lw 3 dt 1,     \
    '../data/castor/psps-integer-quadravrfc.txt'     using 1:($3/1e6) with linespoints notitle ls 9  lw 3 dt 1,     \
    '../data/castor/psps-integer-trinvrtl2.txt'      using 1:($3/1e6) with linespoints notitle ls 10 lw 3 dt 1
#    '../data/castor/psps-integer-duovrfc.txt'        using 1:($3/1e6) with linespoints notitle ls 13 lw 3 dt 1,     \

set lmargin at screen X+2*(W+M)
set rmargin at screen X+2*(W+M)+W

unset label
set ytics 0.5 offset 0.5,0
set yrange [0:2.8]
set style textbox opaque noborder fillcolor rgb "white"
set label at first 1,2.8 front boxed left offset -0.5,0 "2.8"
set label at graph 0.5,1.075 center font "Helvetica-bold,18" "8 swaps/tx"

plot \
    '../data/castor/psps-integer-undologfc.txt'      using 1:($4/1e6) with linespoints notitle ls 1  lw 3 dt (1,1), \
    '../data/castor/psps-integer-redologfc.txt'      using 1:($4/1e6) with linespoints notitle ls 2  lw 3 dt (1,1), \
    '../data/castor/psps-integer-romlogfc.txt'       using 1:($4/1e6) with linespoints notitle ls 4  lw 3 dt (1,1), \
    '../data/castor/psps-integer-trinfc.txt'         using 1:($4/1e6) with linespoints notitle ls 6  lw 3 dt 1,     \
    '../data/castor/psps-integer-quadrafc.txt'       using 1:($4/1e6) with linespoints notitle ls 7  lw 3 dt 1,     \
    '../data/castor/psps-integer-trinvrfc.txt'       using 1:($4/1e6) with linespoints notitle ls 8  lw 3 dt 1,     \
    '../data/castor/psps-integer-quadravrfc.txt'     using 1:($4/1e6) with linespoints notitle ls 9  lw 3 dt 1,     \
    '../data/castor/psps-integer-trinvrtl2.txt'      using 1:($4/1e6) with linespoints notitle ls 10 lw 3 dt 1
#    '../data/castor/psps-integer-duovrfc.txt'        using 1:($4/1e6) with linespoints notitle ls 13 lw 3 dt 1,     \

# Second row

set lmargin at screen X
set rmargin at screen X+W

#set ylabel offset 2.0,0 "Swaps ({/Symbol \264}10^6/s)"
set ylabel offset 1.5,0 "Tx ({/Symbol \264}10^6/s)"
set ytics 0.5 offset 0.5,0
set format y "{%g}"
set yrange [0:2.8]

unset label
set label at graph 0.5,1.075 center font "Helvetica-bold,18" "16 swaps/tx"

plot \
    '../data/castor/psps-integer-undologfc.txt'      using 1:($5/1e6) with linespoints notitle ls 1  lw 3 dt (1,1), \
    '../data/castor/psps-integer-redologfc.txt'      using 1:($5/1e6) with linespoints notitle ls 2  lw 3 dt (1,1), \
    '../data/castor/psps-integer-romlogfc.txt'       using 1:($5/1e6) with linespoints notitle ls 4  lw 3 dt (1,1), \
    '../data/castor/psps-integer-trinfc.txt'         using 1:($5/1e6) with linespoints notitle ls 6  lw 3 dt 1,     \
    '../data/castor/psps-integer-quadrafc.txt'       using 1:($5/1e6) with linespoints notitle ls 7  lw 3 dt 1,     \
    '../data/castor/psps-integer-trinvrfc.txt'       using 1:($5/1e6) with linespoints notitle ls 8  lw 3 dt 1,     \
    '../data/castor/psps-integer-quadravrfc.txt'     using 1:($5/1e6) with linespoints notitle ls 9  lw 3 dt 1,     \
    '../data/castor/psps-integer-trinvrtl2.txt'      using 1:($5/1e6) with linespoints notitle ls 10 lw 3 dt 1
#    '../data/castor/psps-integer-duovrfc.txt'        using 1:($5/1e6) with linespoints notitle ls 13 lw 3 dt 1,     \

unset ylabel
set ytics format ""

set lmargin at screen X+(W+M)
set rmargin at screen X+(W+M)+W

unset label
set ytics 0.5 offset 0.5,0
set yrange [0:2.8]
set style textbox opaque noborder fillcolor rgb "white"
set label at first 2,2.8 front boxed left offset -1.5,0 "2.8"
set label at graph 0.5,1.075 center font "Helvetica-bold,18" "32 swaps/tx"

plot \
    '../data/castor/psps-integer-undologfc.txt'      using 1:($6/1e6) with linespoints notitle ls 1  lw 3 dt (1,1), \
    '../data/castor/psps-integer-redologfc.txt'      using 1:($6/1e6) with linespoints notitle ls 2  lw 3 dt (1,1), \
    '../data/castor/psps-integer-romlogfc.txt'       using 1:($6/1e6) with linespoints notitle ls 4  lw 3 dt (1,1), \
    '../data/castor/psps-integer-trinfc.txt'         using 1:($6/1e6) with linespoints notitle ls 6  lw 3 dt 1,     \
    '../data/castor/psps-integer-quadrafc.txt'       using 1:($6/1e6) with linespoints notitle ls 7  lw 3 dt 1,     \
    '../data/castor/psps-integer-trinvrfc.txt'       using 1:($6/1e6) with linespoints notitle ls 8  lw 3 dt 1,     \
    '../data/castor/psps-integer-quadravrfc.txt'     using 1:($6/1e6) with linespoints notitle ls 9  lw 3 dt 1,     \
    '../data/castor/psps-integer-trinvrtl2.txt'      using 1:($6/1e6) with linespoints notitle ls 10 lw 3 dt 1
#    '../data/castor/psps-integer-duovrfc.txt'        using 1:($6/1e6) with linespoints notitle ls 13 lw 3 dt 1,     \

set lmargin at screen X+2*(W+M)
set rmargin at screen X+2*(W+M)+W

unset label
set ytics 0.5 offset 0.5,0
set yrange [0:2.8]
set style textbox opaque noborder fillcolor rgb "white"
set label at first 2,2.8 front boxed left offset -1.5,0 "2.8"
set label at graph 0.5,1.075 center font "Helvetica-bold,18" "64 swaps/tx"

plot \
    '../data/castor/psps-integer-undologfc.txt'      using 1:($7/1e6) with linespoints notitle ls 1  lw 3 dt (1,1), \
    '../data/castor/psps-integer-redologfc.txt'      using 1:($7/1e6) with linespoints notitle ls 2  lw 3 dt (1,1), \
    '../data/castor/psps-integer-romlogfc.txt'       using 1:($7/1e6) with linespoints notitle ls 4  lw 3 dt (1,1), \
    '../data/castor/psps-integer-trinfc.txt'         using 1:($7/1e6) with linespoints notitle ls 6  lw 3 dt 1,     \
    '../data/castor/psps-integer-quadrafc.txt'       using 1:($7/1e6) with linespoints notitle ls 7  lw 3 dt 1,     \
    '../data/castor/psps-integer-trinvrfc.txt'       using 1:($7/1e6) with linespoints notitle ls 8  lw 3 dt 1,     \
    '../data/castor/psps-integer-quadravrfc.txt'     using 1:($7/1e6) with linespoints notitle ls 9  lw 3 dt 1,     \
    '../data/castor/psps-integer-trinvrtl2.txt'      using 1:($7/1e6) with linespoints notitle ls 10 lw 3 dt 1
#    '../data/castor/psps-integer-duovrfc.txt'        using 1:($7/1e6) with linespoints notitle ls 13 lw 3 dt 1,     \

    
unset tics
unset border
unset xlabel
unset ylabel
unset label
	
unset multiplot   
    
