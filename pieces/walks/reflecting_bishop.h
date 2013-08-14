#if !defined(PIECES_WALKS_REFLECTING_BISHOP_H)
#define PIECES_WALKS_REFLECTING_BISHOP_H

/* This module implements the reflecting bishops */

#include "position/board.h"
#include "solving/observation.h"

/* Generate moves for an reflecting bishop
 */
void reflecting_bishop_generate_moves(void);

boolean reflecting_bishop_check(evalfunction_t *evaluate);

/* Generate moves for an reflecting bishop
 */
void archbishop_generate_moves(void);

boolean archbishop_check(evalfunction_t *evaluate);

#endif
