#if !defined(CONDITION_MAFF_IMMOBILITY_TESTER_H)
#define CONDITION_MAFF_IMMOBILITY_TESTER_H

#include "pyslice.h"

/* This module provides functionality dealing with slices that detect
 * whether a side is immobile
 */

/* Replace immobility tester slices to cope with condition MAFF
 * @param si where to start (entry slice into stipulation)
 */
void maff_replace_immobility_testers(slice_index si);

/* Solve a slice
 * @param si slice index
 * @return whether there is a solution and (to some extent) why not
 */
has_solution_type maff_immobility_tester_king_solve(slice_index si);

#endif
