set term postscript color eps enhanced 22
set output "seqwrite-integer-seq.eps"

set size 0.95,0.6

X=0.08
W=0.28
M=0.01

load "styles.inc"

set tmargin 10.8
set bmargin 2.5

set grid ytics mytics

set ylabel offset 2.4,0 "Sequential Writes/{/Symbol m}s"
# set format y "10^{%T}"
set label at screen 0.50,0.03 center "Writes/TX {/=16 (1,2,4,8,16,32,64,128,256,512)}"

set xtics (1, 2, 4, 8, 16, "" 32, 64, "" 128, 256, "" 512) font ",16" nomirror out offset -0.1,0.6
set ytics 1 offset 0.5,0
set mytics 1

set multiplot layout 1,5

set logscale x
set yrange [0:8]
set xrange [1:512]
set lmargin at screen X
set rmargin at screen X+W

set label at graph 0.5,1.1 center "clwb + sfence"
set key at graph 1.0,0.225 samplen 2

plot \
    '../data/castor/seqwrite-integer-seq-undologfc-clwb.txt'    using 1:($2/1E6) with linespoints notitle ls 1 lw 3 dt (1,1), \
    '../data/castor/seqwrite-integer-seq-redologfc-clwb.txt'    using 1:($2/1E6) with linespoints notitle ls 2 lw 3 dt (1,1), \
    '../data/castor/seqwrite-integer-seq-romlogfc-clwb.txt'     using 1:($2/1E6) with linespoints notitle ls 4 lw 3 dt (1,1), \
    '../data/castor/seqwrite-integer-seq-trinfc-clwb.txt'       using 1:($2/1E6) with linespoints notitle ls 6, \
    '../data/castor/seqwrite-integer-seq-quadrafc-clwb.txt'     using 1:($2/1E6) with linespoints notitle ls 7

unset ylabel
set ytics format ""

set lmargin at screen X+(W+M)
set rmargin at screen X+(W+M)+W

unset label
set label at graph 0.5,1.1 center "clflushopt + sfence"
set key at graph 1.0,0.25 samplen 2

plot \
    '../data/castor/seqwrite-integer-seq-undologfc-clflushopt.txt'    using 1:($2/1E6) with linespoints notitle ls 1 lw 3 dt (1,1), \
    '../data/castor/seqwrite-integer-seq-redologfc-clflushopt.txt'    using 1:($2/1E6) with linespoints notitle ls 2 lw 3 dt (1,1), \
    '../data/castor/seqwrite-integer-seq-romlogfc-clflushopt.txt'     using 1:($2/1E6) with linespoints notitle ls 4 lw 3 dt (1,1), \
    '../data/castor/seqwrite-integer-seq-trinfc-clflushopt.txt'       using 1:($2/1E6) with linespoints notitle ls 6, \
    '../data/castor/seqwrite-integer-seq-quadrafc-clflushopt.txt'     using 1:($2/1E6) with linespoints notitle ls 7

unset xlabel

#X=0.12
#set ytics 1 offset 0.5,0 format "% h"
#set yrange [0:3]

set lmargin at screen X+2*(W+M)
set rmargin at screen X+2*(W+M)+W

unset label
set label at graph 0.5,1.1 center "clflush"
set key at graph 1.0,0.5 samplen 2

plot \
    '../data/castor/seqwrite-integer-seq-undologfc-clflush.txt'    using 1:($2/1E6) with linespoints notitle ls 1 lw 3 dt (1,1), \
    '../data/castor/seqwrite-integer-seq-redologfc-clflush.txt'    using 1:($2/1E6) with linespoints notitle ls 2 lw 3 dt (1,1), \
    '../data/castor/seqwrite-integer-seq-romlogfc-clflush.txt'     using 1:($2/1E6) with linespoints notitle ls 4 lw 3 dt (1,1), \
    '../data/castor/seqwrite-integer-seq-trinfc-clflush.txt'       using 1:($2/1E6) with linespoints notitle ls 6, \
    '../data/castor/seqwrite-integer-seq-quadrafc-clflush.txt'     using 1:($2/1E6) with linespoints notitle ls 7


unset tics
unset border
unset xlabel
unset ylabel
unset label

set key at screen 0.86,0.50 samplen 2.0 font ",18" noinvert width 1
plot [][0:1] \
    2 with linespoints title 'Undo'    ls 1, \
    2 with linespoints title 'Redo'    ls 2, \
    2 with linespoints title 'Rom'     ls 4, \
    2 with linespoints title 'Trinity' ls 6, \
    2 with linespoints title 'Quadra'  ls 7

unset multiplot
