source $env(ASI_TEST_SCRIPTS)/inspection/find-isomorphisms/__begin

# Set working variables.
set datafile    cad/nist/nist_ctc_02.stp
set maxSize     0
set patternFids { 133 134 655 }
set matchedFids { 123 124 125 126 127 128 129 130 133 134 148 149 150 152 153 154 156 157 239 240 241 243 338 339 340 342 343 344 345 655 }

__find-isomorphisms
