set term postscript color eps enhanced 22
set output "pcaption.eps"

set size 0.95,0.15

load "styles.inc"

unset tics
unset border
unset xlabel
unset ylabel
unset label

set object 1 rectangle from screen 0.02,0.01 to screen 0.925,0.14 fillcolor rgb "white" dashtype (2,3) behind
set label at screen 0.5,0.11 center "{/Helvetica-bold Legend for non-volatile memory} (all graphs of {\247}5.2)"

set key at screen 0.9,0.07 samplen 1.5 maxrows 2 width 0.25
plot [][0:1] \
    2 with linespoints title 'Undo'    ls 1, \
    2 with linespoints title 'Redo'    ls 2, \
    2 with linespoints title 'Rom'     ls 4, \
    2 with linespoints title 'Trinity' ls 6, \
    2 with linespoints title 'Quadra'  ls 7
