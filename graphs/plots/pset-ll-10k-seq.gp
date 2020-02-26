set term postscript color eps enhanced 22
set output "pset-ll-10k-seq.eps"

set size 1.0,0.6

X=0.10
W=0.29
M=0.01

load "styles.inc"

set tmargin 11.0
set bmargin 2.5

set grid ytics

set ylabel offset 2.0,0 "Speedup (vs. Undo)"
#set label at screen 0.5,0.03 center "Persistent Sequential Linked List {10^4} keys (clwb + sfence)"

set yrange[0:1.4]
set ytics 1 nomirror offset 0.5,0
set mytics 2

set noxtics
set style histogram clustered gap 0
set style data histogram
set style fill solid border 0
set boxwidth 1 absolute
set offset -1,1,0,0

set multiplot layout 1,3

set lmargin at screen X
set rmargin at screen X+W

set label at graph 0.5,1.1 center "100\% updates"
set key at graph 0.0,1.15 Left left reverse invert samplen 0.5 font ",16"

undolog = `cat '../data/castor/pset-ll-10k-seq-undologfc.txt' | tail -1 | awk '{print $1}' `
set label sprintf("%d",undolog) at 1.5,1.1 font ",14" rotate by 90

plot \
    newhistogram \
    "Undo" font ",14" offset 1,-1, \
    '../data/castor/pset-ll-10k-seq-undologfc.txt' using ($1/undolog):xtic(1) notitle @pat1, \
    newhistogram \
    "" font ",14" offset 1,-1, \
    '../data/castor/pset-ll-10k-seq-redologfc.txt' using ($1/undolog):xtic(1) notitle @pat2, \
    newhistogram \
    "Rom" font ",14" offset 1,-1, \
    '../data/castor/pset-ll-10k-seq-romlogfc.txt' using ($1/undolog):xtic(1) notitle @pat4, \
    newhistogram \
    "Trin" font ",14" offset 1,-1, \
    '../data/castor/pset-ll-10k-seq-trinfc.txt' using ($1/undolog):xtic(1) notitle @pat6, \
    newhistogram \
    "" font ",14" offset 1,-1, \
    '../data/castor/pset-ll-10k-seq-quadrafc.txt' using ($1/undolog):xtic(1) notitle @pat7, \
    newhistogram \
    "TrinVR" font ",14" offset 1,-1, \
    '../data/castor/pset-ll-10k-seq-trinvrfc.txt' using ($1/undolog):xtic(1) notitle @pat8, \
    newhistogram \
    "" font ",14" offset 1,-1, \
    '../data/castor/pset-ll-10k-seq-quadravrfc.txt' using ($1/undolog):xtic(1) notitle @pat9



unset ylabel
set ytics format ""

set lmargin at screen X+(W+M)
set rmargin at screen X+(W+M)+W

unset label
set label at graph 0.5,1.1 center "10\% updates"

undolog = `cat '../data/castor/pset-ll-10k-seq-undologfc.txt' | tail -1 | awk '{print $2}'`
set label sprintf("%d",undolog) at 1.5,1.1 font ",14" rotate by 90

plot \
    newhistogram \
    "" font ",14" offset 1,-1, \
    '../data/castor/pset-ll-10k-seq-undologfc.txt' using ($2/undolog):xtic(1) notitle @pat1, \
    newhistogram \
    "Redo" font ",14" offset 1,-1, \
    '../data/castor/pset-ll-10k-seq-redologfc.txt' using ($2/undolog):xtic(1) notitle @pat2, \
    newhistogram \
    "" font ",14" offset 1,-1, \
    '../data/castor/pset-ll-10k-seq-romlogfc.txt' using ($2/undolog):xtic(1) notitle @pat4, \
    newhistogram \
    "" font ",14" offset 1,-1, \
    '../data/castor/pset-ll-10k-seq-trinfc.txt' using ($2/undolog):xtic(1) notitle @pat6, \
    newhistogram \
    "Quad" font ",14" offset 1,-1, \
    '../data/castor/pset-ll-10k-seq-quadrafc.txt' using ($2/undolog):xtic(1) notitle @pat7, \
    newhistogram \
    "" font ",14" offset 1,-1, \
    '../data/castor/pset-ll-10k-seq-trinvrfc.txt' using ($2/undolog):xtic(1) notitle @pat8, \
    newhistogram \
    "QuadVR" font ",14" offset 1,-1, \
    '../data/castor/pset-ll-10k-seq-quadravrfc.txt' using ($2/undolog):xtic(1) notitle @pat9
unset xlabel

set lmargin at screen X+2*(W+M)
set rmargin at screen X+2*(W+M)+W

unset label
set label at graph 0.5,1.1 center "1\% updates"

undolog = `cat '../data/castor/pset-ll-10k-seq-undologfc.txt' | tail -1 | awk '{print $3}' `
set label sprintf("%d",undolog) at 1.5,1.1 font ",14" rotate by 90

plot \
    newhistogram \
    "" font ",14" offset 1,-1, \
    '../data/castor/pset-ll-10k-seq-undologfc.txt' using ($3/undolog):xtic(1) notitle @pat1, \
    newhistogram \
    "Redo" font ",14" offset 1,-1, \
    '../data/castor/pset-ll-10k-seq-redologfc.txt' using ($3/undolog):xtic(1) notitle @pat2, \
    newhistogram \
    "" font ",14" offset 1,-1, \
    '../data/castor/pset-ll-10k-seq-romlogfc.txt' using ($3/undolog):xtic(1) notitle @pat4, \
    newhistogram \
    "Trin" font ",14" offset 1,-1, \
    '../data/castor/pset-ll-10k-seq-trinfc.txt' using ($3/undolog):xtic(1) notitle @pat6, \
    newhistogram \
    "" font ",14" offset 1,-1, \
    '../data/castor/pset-ll-10k-seq-quadrafc.txt' using ($3/undolog):xtic(1) notitle @pat7, \
    newhistogram \
    "TrinVR" font ",14" offset 1,-1, \
    '../data/castor/pset-ll-10k-seq-trinvrfc.txt' using ($3/undolog):xtic(1) notitle @pat8, \
    newhistogram \
    "" font ",14" offset 1,-1, \
    '../data/castor/pset-ll-10k-seq-quadravrfc.txt' using ($3/undolog):xtic(1) notitle @pat9
	
unset tics
unset border
unset xlabel
unset ylabel
unset label

#set key at screen 1.02,0.47 samplen 2.0 font ",16" opaque noinvert width 1
#plot [][0:1] \
#    2 with linespoints title 'Undo'      ls 1, \
#    2 with linespoints title 'Redo'      ls 2, \
#    2 with linespoints title 'Rom'       ls 4, \
#    2 with linespoints title 'Trinity'   ls 6, \
#    2 with linespoints title 'Quadra'    ls 7, \
#    2 with linespoints title 'TrinityVR' ls 8, \
#    2 with linespoints title 'QuadraVR'  ls 9

unset multiplot
