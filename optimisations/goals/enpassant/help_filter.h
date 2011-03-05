#if !defined(OPTIMISATIONS_GOALS_ENPASSANT_HELP_FILTER_H)
#define OPTIMISATIONS_GOALS_ENPASSANT_HELP_FILTER_H

#include "pystip.h"

/* This module provides functionality dealing with STEnPassantHelpFilter
 * stipulation slices.
 * Slices of this type optimise solving the goal "en passant" by
 * testing whether there is a pawn that can be captured en passant
 * before the final move.
 */

/* Allocate a STEnPassantHelpFilter slice.
 * @return index of allocated slice
 */
slice_index alloc_enpassant_help_filter_slice(void);

/* Determine and write the solution(s) in a help stipulation
 * @param si slice index of slice being solved
 * @param n exact number of half moves until end state has to be reached
 * @return length of solution found, i.e.:
 *         n+4 the move leading to the current position has turned out
 *             to be illegal
 *         n+2 no solution found
 *         n   solution found
 */
stip_length_type enpassant_help_filter_solve_in_n(slice_index si,
                                                  stip_length_type n);

/* Determine whether the slice has a solution in n half moves.
 * @param si slice index of slice being solved
 * @param n number of half moves until end state has to be reached
 * @return length of solution found, i.e.:
 *         n+4 the move leading to the current position has turned out
 *             to be illegal
 *         n+2 no solution found
 *         n   solution found
 */
stip_length_type enpassant_help_filter_has_solution_in_n(slice_index si,
                                                         stip_length_type n);

#endif
