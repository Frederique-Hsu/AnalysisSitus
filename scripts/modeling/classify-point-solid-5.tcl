clear
set datafile public/cad/ANC101.stp

# Read input geometry.
set datadir $env(ASI_TEST_DATA)
clear
load-step $datadir/$datafile

test-build-aabb-points "boxPoints" 100 100 100

test-classify-point-solid "boxPoints" "boxPointsOut" 1 1

test-number-of-points "boxPointsOut" 485213
