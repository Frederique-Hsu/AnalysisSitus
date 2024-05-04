source $env(ASI_TEST_SCRIPTS)/inspection/find-isomorphisms/__begin

# Set working variables.
set datafile    cad/sheet-metal/grabcad_EnclosureBody.stp
set maxSize     0
set patternFids { 220 221 222 223 224 225 440 }
set matchedFids { 178 179 180 181 182 183 192 193 194 195 196 197 206 207 208 209 210 211 220 221 222 223 224 225 234 235 236 237 238 239 248 249 250 251 252 253 434 436 438 440 442 444 }

__find-isomorphisms
