set term postscript color eps enhanced 22
set output "pstack-fat.eps"

set size 0.75,0.6

S=0.05
X=0.1
W=0.375
M=0.075

load "styles.inc"

set tmargin 10.5
set bmargin 3

# We can fit a second graph if need be (remove S "hack")
set multiplot layout 1,2

unset key

set grid ytics

set xtics ("" 1, "" 2, 4, "" 8, 16, "" 24, 32, 40) nomirror out offset -0.25,0.5
set label at screen 0.36,0.04 center "Number of threads"

#set logscale x
set xrange [1:40]

# First row

set lmargin at screen S+X
set rmargin at screen S+X+W

set ylabel offset 1.5,0 "Operations ({/Symbol \264}10^6/s)"
set ytics 1 offset 0.5,0
set yrange [0:5.2]

#set label at graph 0.5,1.075 center font "Helvetica-bold,18" "Fat-node stacks"
set key top left

#set label at graph 0.85,0.44 center font "Helvetica,18" "FHMP"
#set arrow from graph 0.85,0.40 to graph 0.85,0.30 size screen 0.015,25 lw 3

plot \
    '../data/castor/pstack-fat-quadravrfc.txt'     using 1:($2/1e6) with linespoints notitle ls 7  lw 3 dt 1,     \
    '../data/castor/pstack-fat-trinvrfc.txt'       using 1:($2/1e6) with linespoints notitle ls 9  lw 3 dt 1,     \
    '../data/castor/pstack-fat-trinvrtl2.txt'      using 1:($2/1e6) with linespoints notitle ls 11 lw 3 dt 1,     \
    '../data/castor/pstack-fat-duovrfc.txt'        using 1:($2/1e6) with linespoints notitle ls 13 lw 3 dt 1,     \
    '../data/castor/pstack-fat-romlogfc.txt'       using 1:($2/1e6) with linespoints notitle ls 4  lw 3 dt (1,1), \
    '../data/castor/pstack-fat-redologfc.txt'      using 1:($2/1e6) with linespoints notitle ls 2  lw 3 dt (1,1), \
    '../data/castor/pstack-fat-undologfc.txt'      using 1:($2/1e6) with linespoints notitle ls 1  lw 3 dt (1,1)
#    '../data/castor/pstack-fat-trinvrlockstl2.txt' using 1:($2/1e6) with linespoints notitle ls 12 lw 3 dt 1
#    '../data/castor/pstack-fat-trintl2.txt'        using 1:($2/1e6) with linespoints notitle ls 10 lw 3 dt 1,     \
    '../data/castor/pstack-fat-trinfc.txt'         using 1:($2/1e6) with linespoints notitle ls 8  lw 3 dt 1,     \
    '../data/castor/pstack-fat-quadrafc.txt'       using 1:($2/1e6) with linespoints notitle ls 6  lw 3 dt 1,     \
    
unset tics
unset border
unset xlabel
unset ylabel
unset label

set key at screen 0.52,0.47 samplen 2.0 font ",18" noinvert width 1
plot [][0:1] \
    2 with linespoints title 'QuadVR-FC'   ls 7,  \
    2 with linespoints title 'TrinVR-FC'   ls 9,  \
    2 with linespoints title 'TrinVR-TL2'  ls 11, \
    2 with linespoints title 'DuoVR-FC'    ls 13, \
    2 with linespoints title 'Redo-FC'     ls 2,  \
    2 with linespoints title 'Rom-FC'      ls 4, \
    2 with linespoints title 'Undo-FC'     ls 1
    
#    2 with linespoints title 'TrinVRL-TL2' ls 12, \
#    2 with linespoints title 'TrinTL2'    ls 10, \
    2 with linespoints title 'TrinFC'     ls 8,  \
    2 with linespoints title 'QuadFC'     ls 6,  \
	
unset multiplot