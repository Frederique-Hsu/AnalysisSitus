# Set working variables.
set datafile private/gordon/loft2.stp

# Read input geometry.
set datadir $env(ASI_TEST_DATA)
clear; load-part $datadir/$datafile; fit
fit

# Concat curves
concat-curves-and-check -edges 102 103