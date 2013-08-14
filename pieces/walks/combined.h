#if !defined(PIECES_WALKS_COMBINED_H)
#define PIECES_WALKS_COMBINED_H

/* This module implements leaper pieces */

#include "solving/observation.h"

boolean amazone_check(evalfunction_t *evaluate);
boolean dolphin_check(evalfunction_t *evaluate);
boolean dragon_check(evalfunction_t *evaluate);
boolean empress_check(evalfunction_t *evaluate);
boolean princess_check(evalfunction_t *evaluate);
boolean gryphon_check(evalfunction_t *evaluate);
boolean ship_check(evalfunction_t *evaluate);
boolean gral_check(evalfunction_t *evaluate);
boolean scorpion_check(evalfunction_t *evaluate);

#endif
