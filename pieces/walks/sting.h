#if !defined(PIECES_WALKS_STING_H)
#define PIECES_WALKS_STING_H

/* This module implements the fairy piece Sting */

#include "solving/observation.h"

/* Generate moves for a Sting
 */
void sting_generate_moves(void);

boolean sting_check(evalfunction_t *evaluate);

#endif
