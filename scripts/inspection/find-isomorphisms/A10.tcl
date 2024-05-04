source $env(ASI_TEST_SCRIPTS)/inspection/find-isomorphisms/__begin

# Set working variables.
set datafile    cad/nist/nist_ctc_02.stp
set maxSize     0
set patternFids { 186 188 190 396 397 398 }
set matchedFids { 186 188 190 311 313 315 318 319 320 396 397 398 }

__find-isomorphisms
