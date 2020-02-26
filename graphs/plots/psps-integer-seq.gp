set term postscript color eps enhanced 22
set output "psps-integer-seq.eps"

set size 0.66,0.56

X=0.07
W=0.36
M=0.01

load "styles.inc"

set tmargin 11.0
set bmargin 2.5

set grid ytics

set ylabel offset 1.6,0 "Swaps/{/Symbol m}s"
#set label at screen 0.30,0.03 center "Swaps/TX"
set label at screen 0.25,0.03 center "Swaps/TX"

set xtics (1, 2, 4, 8, 16, 32, 64, 128, 256, 512) font "Helvetica-Narrow,18" nomirror out offset -0.1,0.6
set ytics 1 offset 0.5,0
set mytics 5

set multiplot layout 1,2

set logscale x
set yrange [0:3]
set xrange [1:512]
set lmargin at screen X
set rmargin at screen X+W


plot \
    '../data/castor/psps-integer-seq-undologfc.txt'    using 1:($2/1E6) with linespoints notitle ls 1 lw 3 dt (1,1), \
    '../data/castor/psps-integer-seq-redologfc.txt'    using 1:($2/1E6) with linespoints notitle ls 2 lw 3 dt (1,1), \
    '../data/castor/psps-integer-seq-romlogfc.txt'     using 1:($2/1E6) with linespoints notitle ls 4 lw 3 dt (1,1), \
    '../data/castor/psps-integer-seq-trinfc.txt'       using 1:($2/1E6) with linespoints notitle ls 6, \
    '../data/castor/psps-integer-seq-quadrafc.txt'     using 1:($2/1E6) with linespoints notitle ls 7, \
    '../data/castor/psps-integer-seq-trinvrfc.txt'     using 1:($2/1E6) with linespoints notitle ls 8, \
    '../data/castor/psps-integer-seq-quadravrfc.txt'   using 1:($2/1E6) with linespoints notitle ls 9

unset tics
unset border
unset xlabel
unset ylabel
unset label

set key at screen 0.67,0.47 samplen 3.0 font ",18" opaque noinvert width 1
plot [][0:1] \
    2 with linespoints title 'QuadraVR'  ls 9, \
    2 with linespoints title 'TrinityVR' ls 8, \
    2 with linespoints title 'Quadra'    ls 7, \
    2 with linespoints title 'Trinity'   ls 6, \
    2 with linespoints title 'Rom'       ls 4, \
    2 with linespoints title 'Redo'      ls 2, \
    2 with linespoints title 'Undo'      ls 1

unset multiplot
