source $env(ASI_TEST_SCRIPTS)/inspection/find-isomorphisms/__begin

# Set working variables.
set datafile    public/cad/ANC101.stp
set maxSize     0
set patternFids { 37 38 39 40 41 42 44 46 58 }
set matchedFids { 26 27 28 29 30 34 35 36 37 38 39 40 41 42 44 46 56 58 }

__find-isomorphisms
