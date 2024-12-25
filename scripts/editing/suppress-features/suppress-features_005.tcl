# Set working variables.
set datafile     public/cad/aec/cube_with_holes.brep
set ref_ncomp    0
set ref_ncompso  0
set ref_nso      1
set ref_nshe     1
set ref_nf       97
set ref_nw       110
set ref_ne       282
set ref_nv       188

# Read input geometry.
set datadir $env(ASI_TEST_DATA)
load-part $datadir/$datafile
fit
#
if { [check-validity] != 1 } {
  error "Initial part is not valid."
}
#
print-summary
#
set initialToler [get-tolerance]

# Initialize naming service.
init-naming

# Apply feature suppression.
suppress-features {*}[get-face-ids -names face_72 face_73 face_74 face_75]
#
if { [check-validity] != 1 } {
  error "Final part is not valid."
}

# Verify face IDs vs names (to check that naming service works well).
if { [get-face-name -fid 96] != "face_76" } {
  error "Unexpected face name."
}
#
if { [get-face-name -fid 58] != "face_71" } {
  error "Unexpected face name."
}
#
if { [get-face-name -fid 21] != "face_101" } {
  error "Unexpected face name."
}
# Verify cardinal numbers.
get-summary ncomp ncompso nso nshe nf nw ne nv
#
if { $ncomp != $ref_ncomp } {
  error "Unexpected number of compounds."
}
if { $ncompso != $ref_ncompso } {
  error "Unexpected number of compsolids."
}
if { $nso != $ref_nso } {
  error "Unexpected number of solids."
}
if { $nshe != $ref_nshe } {
  error "Unexpected number of shells."
}
if { $nf != $ref_nf } {
  error "Unexpected number of faces."
}
if { $nw != $ref_nw } {
  error "Unexpected number of wires."
}
if { $ne != $ref_ne } {
  error "Unexpected number of edges."
}
if { $nv != $ref_nv } {
  error "Unexpected number of vertices."
}

# Check that tolernace has not significantly degraded.
set finalToler [get-tolerance]
puts "Final tolerance ($finalToler) vs initial tolerance ($initialToler)"
#
if { [expr $finalToler - $initialToler] > 1e-3 } {
  error "Significant tolerance degradation."
}

# Check orientations of vertices.
if { [check-vertices-ori] != 1 } {
  error "Some edges have non-distinguishable orientations of vertices."
}

# Check contours of faces.
if { [check-contours] != 1 } {
  error "Some faces have open contours."
}
