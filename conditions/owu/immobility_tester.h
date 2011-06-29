#if !defined(CONDITION_OWU_IMMOBILITY_TESTER_H)
#define CONDITION_OWU_IMMOBILITY_TESTER_H

#include "pyslice.h"

/* This module provides functionality dealing with slices that detect
 * whether a side is immobile
 */

/* Substitute OWU specific immobility testers
 * @param si where to start (entry slice into stipulation)
 */
void owu_replace_immobility_testers(slice_index si);

/* Determine whether a slice.has just been solved with the move
 * by the non-starter
 * @param si slice identifier
 * @return whether there is a solution and (to some extent) why not
 */
has_solution_type owu_immobility_tester_king_has_solution(slice_index si);

/* Determine whether a slice.has just been solved with the move
 * by the non-starter
 * @param si slice identifier
 * @return whether there is a solution and (to some extent) why not
 */
has_solution_type owu_immobility_tester_other_has_solution(slice_index si);

#endif
