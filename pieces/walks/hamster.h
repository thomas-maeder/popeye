#if !defined(PIECES_WALKS_HAMSTER_H)
#define PIECES_WALKS_HAMSTER_H

#include "utilities/boolean.h"
#include "solving/observation.h"

/* This module implements the Hamster */

/* Generate moves for an Hamster
 */
void hamster_generate_moves(void);

void contrahamster_generate_moves(void);

boolean contrahamster_check(validator_id evaluate);

#endif
