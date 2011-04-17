#if !defined(OPTIMISATIONS_INTELLIGENT_HELP_FILTER_H)
#define OPTIMISATIONS_INTELLIGENT_HELP_FILTER_H

#include "py.h"
#include "pystip.h"

/* This module provides functionality dealing with STIntelligentHelpFilter
 * stipulation slice type.
 * Slices of this type make solve help stipulations in intelligent mode
 */

/* Allocate a STIntelligentHelpFilter slice.
 * @apram full_length full length (half-moves) of branch
 * @return allocated slice
 */
slice_index alloc_intelligent_help_filter(stip_length_type full_length);

/* Determine and write the solution(s) in a help stipulation
 * @param si slice index of slice being solved
 * @param n exact number of half moves until end state has to be reached
 * @return length of solution found, i.e.:
 *         n+4 the move leading to the current position has turned out
 *             to be illegal
 *         n+2 no solution found
 *         n   solution found
 */
stip_length_type intelligent_help_filter_solve_in_n(slice_index si,
                                                    stip_length_type n);

/* Determine whether the slice has a solution in n half moves.
 * @param si slice index of slice being solved
 * @param n number of half moves until end state has to be reached
 * @return true iff >= 1 solution has been found
 */
boolean intelligent_help_filter_has_solution_in_n(slice_index si,
                                                  stip_length_type n);

#endif
