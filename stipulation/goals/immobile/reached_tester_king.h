#if !defined(STIPULATION_GOAL_IMMOBILE_REACHED_TESTER_KING_H)
#define STIPULATION_GOAL_IMMOBILE_REACHED_TESTER_KING_H

#include "pyslice.h"

/* This module provides functionality dealing with slices that detect
 * whether a side's king is immobile
 */

/* Convert a help branch into a branch testing for immobility of a side's king
 * @param help_branch identifies entry slice into help branch
 * @return identifier of entry slice into tester branch
 */
slice_index make_immobility_tester_king(slice_index help_branch);

/* Determine whether a slice.has just been solved with the move
 * by the non-starter
 * @param si slice identifier
 * @return whether there is a solution and (to some extent) why not
 */
has_solution_type immobility_tester_king_has_solution(slice_index si);

#endif
