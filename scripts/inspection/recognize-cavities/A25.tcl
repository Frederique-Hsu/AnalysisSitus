source $env(ASI_TEST_SCRIPTS)/inspection/recognize-cavities/__begin

# Set working variables.
set datafile  public/cad/blends/0079_level_grabcad-rana.step
set maxSize   0
set refFids { 45 46 60 61 }

__recognize-cavities
