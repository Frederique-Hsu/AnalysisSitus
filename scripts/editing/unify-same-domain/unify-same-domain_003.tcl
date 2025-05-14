# Set working variables.
set datafile private/unify-same-domain/UnifySameDomain2.stp

# Read input geometry.
set datadir $env(ASI_TEST_DATA)
clear
load-step $datadir/$datafile

test-check-part-shape

print-summary

sew -toler 0.001

test-check-number-shape-entities -vertex 944 -edge 1414 -wire 526 -face 498 -shell 1 -solid 0 -compsolid 0 -compound 0

test-check-shape-aabb-dim -xDim 25.400000200000001 -yDim 6.4943251565984763 -zDim 25.400000199999997 -tol 1.0e-4

catch { maximize-faces }

test-check-number-shape-entities -vertex 944 -edge 1414 -wire 526 -face 498 -shell 1 -solid 0 -compsolid 0 -compound 0

test-check-shape-aabb-dim -xDim 25.400000200000001 -yDim 6.4943251565984763 -zDim 25.400000199999997 -tol 1.0e-4

print-summary
