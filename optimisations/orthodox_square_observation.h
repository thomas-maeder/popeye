#if !defined(OPTIMISATIONS_ORTHODOX_SQUARE_OBSERVATION_H)
#define OPTIMISATIONS_ORTHODOX_SQUARE_OBSERVATION_H

#include "position/side.h"
#include "position/board.h"
#include "pieces/walks/vectors.h"
#include "utilities/boolean.h"

/* This module implements an optimised algorithm for detecting square
 * observations
 */

boolean is_square_observed_ortho(Side side_checking,
                                 square sq_target);

vec_index_type is_square_uninterceptably_observed_ortho(Side side_checking,
                                                        square sq_target);

unsigned int count_interceptable_orthodox_checks(Side side_checking, square sq_target);

vec_index_type king_check_ortho(Side side_checking, square sq_king);
vec_index_type knight_check_ortho(Side side_checking, square sq_king);
vec_index_type pawn_check_ortho(Side side_checking, square sq_king);

#endif
