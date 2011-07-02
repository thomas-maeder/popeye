#if !defined(STIPULATION_CONDITION_OHNESCHACH_IMMOBILE_TESTER_H)
#define STIPULATION_CONDITION_OHNESCHACH_IMMOBILE_TESTER_H

#include "pyslice.h"

/* This module provides functionality dealing with slices that detect
 * whether a side is immobile
 */

/* Replace immobility tester slices to cope with condition Ohneschach
 * @param si where to start (entry slice into stipulation)
 */
void ohneschach_replace_immobility_testers(slice_index si);

/* Determine whether a slice.has just been solved with the move
 * by the non-starter
 * @param si slice identifier
 * @return whether there is a solution and (to some extent) why not
 */
has_solution_type ohneschach_immobility_tester_has_solution(slice_index si);

/* Determine whether a side is immobile in Ohneschach
 * @return true iff side is immobile
 */
boolean ohneschach_immobile(Side isde);

#endif
