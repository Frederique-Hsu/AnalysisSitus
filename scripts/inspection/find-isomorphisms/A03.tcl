source $env(ASI_TEST_SCRIPTS)/inspection/find-isomorphisms/__begin

# Set working variables.
set datafile    cad/ANC101.stp
set maxSize     0
set patternFids { 31 32 33 57 }
set matchedFids { 31 32 33 43 45 47 57 59 }

__find-isomorphisms
