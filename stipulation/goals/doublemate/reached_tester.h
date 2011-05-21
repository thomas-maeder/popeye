#if !defined(STIPULATION_GOAL_DOUBLEMATE_REACHED_TESTER_H)
#define STIPULATION_GOAL_DOUBLEMATE_REACHED_TESTER_H

#include "pyslice.h"

/* This module provides functionality dealing with slices that detect
 * whether a double mate goal has just been reached
 */

/* TODO get rid of this */
extern boolean are_we_testing_immobility_with_opposite_king_en_prise;

/* Allocate a system of slices that tests whether doublemate has been reached
 * @return index of entry slice
 */
slice_index alloc_doublemate_mate_reached_tester_system(void);

/* Determine whether a slice has just been solved with the move
 * by the non-starter
 * @param si slice identifier
 * @return whether there is a solution and (to some extent) why not
 */
has_solution_type goal_doublemate_reached_tester_has_solution(slice_index si);

#endif
