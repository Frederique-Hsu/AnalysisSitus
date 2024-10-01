source $env(ASI_TEST_SCRIPTS)/inspection/find-isomorphisms/__begin

# Set working variables.
set datafile    public/cad/chamfers/chamfers_21.stp
set maxSize     0
set patternFids { 77 78 79 80 }
set matchedFids { 61 62 63 64 65 66 67 68 69 70 71 72 73 74 75 76 77 78 79 80 81 82 83 84 85 86 87 88 89 90 91 92 }

__find-isomorphisms
