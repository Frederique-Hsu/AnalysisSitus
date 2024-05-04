source $env(ASI_TEST_SCRIPTS)/inspection/find-isomorphisms/__begin

# Set working variables.
set datafile    cad/surverul.stp
set maxSize     0
set patternFids { 33 34 35 36 41 44 45 46 }
set matchedFids { 33 34 35 36 37 38 39 40 41 42 43 44 45 46 47 48 }

__find-isomorphisms
