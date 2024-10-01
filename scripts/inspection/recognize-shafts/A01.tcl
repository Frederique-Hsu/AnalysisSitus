source $env(ASI_TEST_SCRIPTS)/inspection/recognize-shafts/__begin

# Set working variables.
set datafile public/cad/ANC101.stp
set radius 1e10
set refFids { 70 }

__recognize-shafts
