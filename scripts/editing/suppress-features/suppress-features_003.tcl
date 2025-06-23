# Set working variables.
set datafile     public/cad/20221121_154647_982.step
set ref_ncomp    0
set ref_ncompso  0
set ref_nso      1
set ref_nshe     1
set ref_nf       31
set ref_nw       35
set ref_ne       89
set ref_nv       60

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
suppress-features {*}[get-face-ids -names face_2 face_5 face_10]
#
if { [check-validity] != 1 } {
  error "Final part is not valid."
}

# Verify face IDs vs names (to check that naming service works well).
if { [get-face-name -fid 17] != "face_13" } {
  error "Unexpected face name."
}
#
if { [get-face-name -fid 4] != "face_1" } {
  error "Unexpected face name."
}
#
if { [get-face-name -fid 10] != "face_9" } {
  error "Unexpected face name."
}
#
if { [get-face-name -fid 2] != "face_11" } {
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
