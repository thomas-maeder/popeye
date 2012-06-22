#if !defined(OPTIMISATION_INTELLIGENT_STALEMATE_FINISH_H)
#define OPTIMISATION_INTELLIGENT_STALEMATE_FINISH_H

#include "py.h"
#include "position/board.h"

/* Test the position created by the taken operations; if the position is a
 * solvable target position: solve it; otherwise: improve it
 */
void intelligent_stalemate_test_target_position(void);

#endif
