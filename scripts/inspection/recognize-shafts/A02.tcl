source $env(ASI_TEST_SCRIPTS)/inspection/recognize-shafts/__begin

# Set working variables.
set datafile public/cad/cylinder.stp
set radius 1e10
set refFids { 1 }

__recognize-shafts
