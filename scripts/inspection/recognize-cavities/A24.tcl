source $env(ASI_TEST_SCRIPTS)/inspection/recognize-cavities/__begin

# Set working variables.
set datafile  public/cad/blends/0077_isolated_blends_test_28.brep
set maxSize   0
set refFids { 105 }

__recognize-cavities
