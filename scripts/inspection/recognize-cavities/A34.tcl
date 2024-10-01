source $env(ASI_TEST_SCRIPTS)/inspection/recognize-cavities/__begin

# Set working variables.
set datafile  public/cad/milled/1815858-1.step
set maxSize   0
set refFids { 14 15 16 17 18 19 20 21 26 27 28 29 30 31 32 33 34 35 37 38 39 40 41 }

__recognize-cavities
