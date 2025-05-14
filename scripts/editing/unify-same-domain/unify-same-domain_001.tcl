# Set working variables.
set datafile public/cad/ANC101.stp

# Read input geometry.
set datadir $env(ASI_TEST_DATA)
clear
load-step $datadir/$datafile

test-check-part-shape

print-summary

test-check-number-shape-entities -vertex 131 -edge 198 -wire 117 -face 87 -shell 1 -solid 1 -compsolid 0 -compound 1

test-check-shape-aabb-dim -xDim 426.25000010000002 -yDim 201.50000019999999 -zDim 190.54547119979679 -tol 1.0e-4

maximize-faces

test-check-solids-volumes 1.0e-4 8182330.6183503876

test-check-number-shape-entities -vertex 131 -edge 197 -wire 116 -face 86 -shell 1 -solid 1 -compsolid 0 -compound 1

test-check-shape-aabb-dim -xDim 426.25000010000002 -yDim 201.50000019999999 -zDim 190.54547119979679 -tol 1.0e-4

print-summary
