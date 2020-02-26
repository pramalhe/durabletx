set term postscript color eps enhanced 22
set output "pq-ll.eps"

set size 0.76,0.6

S=0.05
X=0.06
W=0.395
M=0.075

load "styles.inc"

set tmargin 10.5
set bmargin 3

# We can fit a second graph if need be (remove S "hack")
set multiplot layout 1,2

unset key

set grid ytics

set xtics ("" 1, "" 2, 4, 8, 16, 24, 32, 40) nomirror out offset -0.25,0.5
set label at screen 0.32,0.04 center "Number of threads"

#set logscale x
set xrange [1:40]

# First row

set lmargin at screen S+X
set rmargin at screen S+X+W

set ylabel offset 2.0,0 "Tx ({/Symbol \264}10^6/s)"
set ytics 0.2 offset 0.5,0
set yrange [0:1.4]

#set label at graph 0.5,1.075 center font "Helvetica-bold,18" "Linked list-based queue"
set key top left

#set label at graph 0.85,0.44 center font "Helvetica,18" "FHMP"
#set arrow from graph 0.85,0.40 to graph 0.85,0.30 size screen 0.015,25 lw 3

plot \
    '../data/castor/pq-ll-undologfc.txt'      using 1:($2/1e6) with linespoints notitle ls 1  lw 3 dt (1,1), \
    '../data/castor/pq-ll-redologfc.txt'      using 1:($2/1e6) with linespoints notitle ls 2  lw 3 dt (1,1), \
    '../data/castor/pq-ll-romlogfc.txt'       using 1:($2/1e6) with linespoints notitle ls 4  lw 3 dt (1,1), \
    '../data/castor/pq-ll-quadravrfc.txt'     using 1:($2/1e6) with linespoints notitle ls 9  lw 3 dt 1,     \
    '../data/castor/pq-ll-trinvrfc.txt'       using 1:($2/1e6) with linespoints notitle ls 8  lw 3 dt 1,     \
    '../data/castor/pq-ll-trinfc.txt'         using 1:($2/1e6) with linespoints notitle ls 6  lw 3 dt 1,     \
    '../data/castor/pq-ll-quadrafc.txt'       using 1:($2/1e6) with linespoints notitle ls 7  lw 3 dt 1,     \
    '../data/castor/pq-ll-trinvrtl2.txt'      using 1:($2/1e6) with linespoints notitle ls 10 lw 3 dt 1
#    '../data/castor/pq-ll-duovrfc.txt'        using 1:($2/1e6) with linespoints notitle ls 13 lw 3 dt 1,     \
    
unset tics
unset border
unset xlabel
unset ylabel
unset label

set key at screen 0.52,0.47 samplen 3.0 font ",18" noinvert width 1
plot [][0:1] \
    2 with linespoints title 'TrinVR-TL2'  ls 10, \
    2 with linespoints title 'QuadVR-FC'   ls 9,  \
    2 with linespoints title 'TrinVR-FC'   ls 8,  \
    2 with linespoints title 'Quad-FC'     ls 7,  \
    2 with linespoints title 'Trin-FC'     ls 6,  \
    2 with linespoints title 'Rom-FC'      ls 4,  \
    2 with linespoints title 'Redo-FC'     ls 2,  \
    2 with linespoints title 'Undo-FC'     ls 1
	
unset multiplot