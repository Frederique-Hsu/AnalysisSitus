# Set working variables.
set datafile private/unify-same-domain/UnifySameDomain1.stp

# Read input geometry.
set datadir $env(ASI_TEST_DATA)
clear
load-step $datadir/$datafile

test-check-part-shape

print-summary

sew -toler 0.001

test-check-number-shape-entities -vertex 454 -edge 456 -wire 26 -face 2 -shell 1 -solid 0 -compsolid 0 -compound 0

test-check-shape-aabb-dim -xDim 25.400000199999997 -yDim 6.0448502503609998 -zDim 25.400000199999997 -tol 1.0e-4

maximize-faces

test-check-number-shape-entities -vertex 454 -edge 455 -wire 25 -face 1 -shell 1 -solid 0 -compsolid 0 -compound 0

test-check-shape-aabb-dim -xDim 25.400000199999997 -yDim 6.0448502503609998 -zDim 25.400000199999997 -tol 1.0e-4

print-summary
