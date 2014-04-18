#if !defined(STIPULATION_GOAL_KISS_REACHED_TESTER_H)
#define STIPULATION_GOAL_KISS_REACHED_TESTER_H

#include "stipulation/stipulation.h"
#include "position/pieceid.h"

extern PieceIdType id_to_be_kissed;

/* This module provides functionality dealing with slices that detect
 * whether a kiss goal has just been reached
 */

/* Remember the id of the piece to be kissed
 * @param si root of the solving machinery
 */
void goal_kiss_init_piece_id(slice_index si);

/* Allocate a system of slices that tests whether first row or last row has been reached
 * @return index of entry slice
 */
slice_index alloc_goal_kiss_reached_tester_system(square s);

#endif
