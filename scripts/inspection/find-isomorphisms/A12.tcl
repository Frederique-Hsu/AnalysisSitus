source $env(ASI_TEST_SCRIPTS)/inspection/find-isomorphisms/__begin

# Set working variables.
set datafile    public/cad/nist/nist_ctc_04.brep
set maxSize     0
set patternFids { 394 395 396 397 398 399 400 }
set matchedFids { 394 395 396 397 398 399 400 401 402 403 404 405 406 407 }

__find-isomorphisms
