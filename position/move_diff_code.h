#if !defined(SOLVING_MOVE_DIFF_CODE_H)
#define SOLVING_MOVE_DIFF_CODE_H

/* This module implements the move difference code table
 */

#include "position/board.h"

typedef unsigned int move_diff_type;

/* This are the codes for the length-difference */
/* between two squares */
move_diff_type squared_distance_between_squares(square sq1, square sq2);

#endif
