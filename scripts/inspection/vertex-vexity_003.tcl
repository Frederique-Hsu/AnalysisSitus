# Set working variables.
set datafile public/cad/situ/vexity/vvexity-01.brep

# Read input geometry.
set datadir $env(ASI_TEST_DATA)
load-brep $datadir/$datafile
fit

set ref_convex 0
set ref_concave 3
set ref_smooth 100

# Check vexity.
check-vertex-vexity -fid 1 convex concave smooth

# Verify result.
if { $convex != $ref_convex } {
  error "Unexpected number of convex vertices ($ref_convex expected while $convex returned)."
}
if { $concave != $ref_concave } {
  error "Unexpected number of concave vertices ($ref_concave expected while $concave returned)."
}
if { $smooth != $ref_smooth } {
  error "Unexpected number of smooth vertices ($ref_smooth expected while $smooth returned)."
}
