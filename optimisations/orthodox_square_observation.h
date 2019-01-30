#if !defined(OPTIMISATIONS_ORTHODOX_SQUARE_OBSERVATION_H)
#define OPTIMISATIONS_ORTHODOX_SQUARE_OBSERVATION_H

#include "position/side.h"
#include "position/board.h"
#include "utilities/boolean.h"

/* This module implements an optimised algorithm for detecting square
 * observations
 */

boolean is_square_observed_ortho(Side side_checking,
                                 square sq_target);

boolean is_square_uninterceptably_observed_ortho(Side side_checking,
                                                 square sq_target);

unsigned int count_interceptable_orthodox_checks(Side side_checking, square sq_target);

#endif
