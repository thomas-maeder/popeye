#if !defined(SOLVING_MOVE_DIFF_CODE_H)
#define SOLVING_MOVE_DIFF_CODE_H

/* This module implements the move difference code table
 */

#include "position/board.h"

typedef unsigned int move_diff_type;

/* This are the codes for the length-difference */
/* between two squares */
/* ATTENTION: use abs(square from - square to) for indexing this table. */
/*        all move_down_codes are mapped this way to move_up_codes !    */
extern move_diff_type const move_diff_code[square_h8 - square_a1 + 1];

#endif
