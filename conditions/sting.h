#if !defined(CONDITIONS_STING_H)
#define CONDITIONS_STING_H

/* This module implements the fairy piece Sting */

#include "solving/observation.h"

void sting_generate_moves(void);

boolean sting_check(evalfunction_t *evaluate);

#endif
