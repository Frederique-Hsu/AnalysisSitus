source $env(ASI_TEST_SCRIPTS)/inspection/find-isomorphisms/__begin

# Set working variables.
set datafile    public/cad/sheet-metal/grabcad_EnclosureBody.stp
set maxSize     0
set patternFids { 402 409 410 411 412 446 453 454 455 456 }
set matchedFids { 401 402 403 404 405 406 407 408 409 410 411 412 413 414 415 416 417 418 419 420 445 446 447 448 449 450 451 452 453 454 455 456 457 458 459 460 461 462 463 464 }

__find-isomorphisms
