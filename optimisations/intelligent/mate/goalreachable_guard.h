#if !defined(OPTIMISATION_INTELLIGENT_MATE_GOALREACHABLE_GUARD_H)
#define OPTIMISATION_INTELLIGENT_MATE_GOALREACHABLE_GUARD_H

#include "py.h"

/* Solve in a number of half-moves
 * @param si identifies slice
 * @param n exact number of half moves until end state has to be reached
 * @return length of solution found, i.e.:
 *         n+4 the move leading to the current position has turned out
 *             to be illegal
 *         n+2 no solution found
 *         n   solution found
 */
stip_length_type goalreachable_guard_mate_help(slice_index si, stip_length_type n);

/* Determine whether there is a solution in n half moves.
 * @param si slice index of slice being solved
 * @param n exact number of half moves until end state has to be reached
 * @return length of solution found, i.e.:
 *         n+4 the move leading to the current position has turned out
 *             to be illegal
 *         n+2 no solution found
 *         n   solution found
 */
stip_length_type goalreachable_guard_mate_can_help(slice_index si,
                                                   stip_length_type n);

#endif
