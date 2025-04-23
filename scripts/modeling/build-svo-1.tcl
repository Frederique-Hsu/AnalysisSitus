set datafile public/cad/ANC101.stp

# Read input geometry.
set datadir $env(ASI_TEST_DATA)
clear
load-step $datadir/$datafile

# Configure DDF.
set-param 0:2:3:1 19 1
ddf-build-svo -min 10 -max 1500 -prec 1 -cube -owner 0:2:3:1
set-param 0:2:23:1 26 3
set-param 0:2:23:1 27 1
ddf-dump-vtu 0:2:23:1 $datadir/public/cad/ANC101.vtu
