source $env(ASI_TEST_SCRIPTS)/inspection/find-isomorphisms/__begin

# Set working variables.
set datafile    cad/nist/nist_ctc_05.brep
set maxSize     0
set patternFids { 4 27 69 70 138 142 149 150 151 158 163 }
set matchedFids { 4 5 6 27 28 68 69 70 128 129 131 132 135 136 138 142 148 149 150 151 154 155 158 159 163 164 }

__find-isomorphisms
