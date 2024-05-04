source $env(ASI_TEST_SCRIPTS)/inspection/find-isomorphisms/__begin

# Set working variables.
set datafile    cad/nist/nist_ctc_01.stp
set maxSize     0
set patternFids { 12 13 14 }
set matchedFids { 12 13 14 16 17 18 31 32 33 34 35 37 38 39 40 41 77 78 79 80 81 82 83 84 85 86 87 88 121 129 130 131 133 136 }

__find-isomorphisms
