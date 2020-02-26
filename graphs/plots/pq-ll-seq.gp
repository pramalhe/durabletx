set term postscript color eps enhanced 22
set output "pq-ll-seq.eps"

set size 0.62,0.56

X=0.1
W=0.45
M=0.01

load "styles.inc"

set tmargin 11.0
set bmargin 2.5

set grid ytics

set ylabel offset 2.0,0 "Speedup (vs. Undo)"
#set label at screen 0.3,0.03 center "Persistent Queue"

set yrange[0:7]
set ytics 1 nomirror offset 0.5,0

set noxtics
set style histogram clustered gap 0
set style data histogram
set style fill solid border 0
set boxwidth 1 absolute
set offset -1,1,0,0

set multiplot layout 1,2

set lmargin at screen X
set rmargin at screen X+W

#set label at graph 0.5,1.1 center "Queue"
set key at graph 0.0,1.15 Left left reverse invert samplen 0.5 font ",16"

undolog = `cat '../data/castor/pq-ll-enq-deq-seq-undologfc.txt' | tail -1 `
set label sprintf("%d",undolog) at 1.5,1.1 font ",12" rotate by 90

plot \
    newhistogram \
    "Undo" font ",14" offset 1,-1, \
    '../data/castor/pq-ll-enq-deq-seq-undologfc.txt' using ($1/undolog):xtic(1) notitle @pat1, \
    newhistogram \
    "Redo" font ",14" offset 1,-1, \
    '../data/castor/pq-ll-enq-deq-seq-redologfc.txt' using ($1/undolog):xtic(1) notitle @pat2, \
    newhistogram \
    "Rom" font ",14" offset 1,-1, \
    '../data/castor/pq-ll-enq-deq-seq-romlogfc.txt' using ($1/undolog):xtic(1) notitle @pat4, \
    newhistogram \
    "Trin" font ",14" offset 1,-1, \
    '../data/castor/pq-ll-enq-deq-seq-trinfc.txt' using ($1/undolog):xtic(1) notitle @pat6, \
    newhistogram \
    "Quad" font ",14" offset 1,-1, \
    '../data/castor/pq-ll-enq-deq-seq-quadrafc.txt' using ($1/undolog):xtic(1) notitle @pat7, \
    newhistogram \
    "TrinVR" font ",14" offset 1,-1, \
    '../data/castor/pq-ll-enq-deq-seq-trinvrfc.txt' using ($1/undolog):xtic(1) notitle @pat8, \
    newhistogram \
    "QuadVR" font ",14" offset 1,-1, \
    '../data/castor/pq-ll-enq-deq-seq-quadravrfc.txt' using ($1/undolog):xtic(1) notitle @pat9

unset tics
unset border
unset xlabel
unset ylabel
unset label

set key at screen 1.02,0.47 samplen 2.0 font ",18" opaque noinvert width 1
plot [][0:1] \
    2 with linespoints title 'QuadraVR'  ls 9, \
    2 with linespoints title 'TrinityVR' ls 8, \
    2 with linespoints title 'Quadra'    ls 7, \
    2 with linespoints title 'Trinity'   ls 6, \
    2 with linespoints title 'Rom'       ls 4, \
    2 with linespoints title 'Redo'      ls 2, \
    2 with linespoints title 'Undo'      ls 1

unset multiplot
