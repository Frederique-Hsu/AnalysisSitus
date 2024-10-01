source $env(ASI_TEST_SCRIPTS)/inspection/find-isomorphisms/__begin

# Set working variables.
set datafile    public/cad/sheet-metal/grabcad_EnclosureBody.stp
set maxSize     0
set patternFids { 1 427 428 429 430 431 432 466 }
set matchedFids { 1 3 421 422 423 424 425 426 427 428 429 430 431 432 465 466 }

__find-isomorphisms
