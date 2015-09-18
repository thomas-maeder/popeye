#if !defined(OPTIMISATION_INTELLIGENT_MATE_INTERCEPT_BLACK_MOVE_H)
#define OPTIMISATION_INTELLIGENT_MATE_INTERCEPT_BLACK_MOVE_H

#include "position/board.h"
#include "stipulation/stipulation.h"

/* Intercept a black move
 * @param from departure square of move to be intercepted
 * @param from arrival square
 * @param go_on what to do after each successful interception?
 */
void intelligent_intercept_black_move(slice_index si,
                                      square from, square to,
                                      void (*go_on)(slice_index si));

#endif
