source $env(ASI_TEST_SCRIPTS)/inspection/find-isomorphisms/__begin

# Set working variables.
set datafile    cad/nist/nist_ctc_01.stp
set maxSize     0
set patternFids { 105 106 107 108 109 110 }
set matchedFids { 105 106 107 108 109 110 111 112 113 114 115 116 }

__find-isomorphisms
