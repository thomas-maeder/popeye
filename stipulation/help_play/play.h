#if !defined(STIPULATION_HELP_PLAY_PLAY_H)
#define STIPULATION_HELP_PLAY_PLAY_H

/* Interface for dynamically dispatching help play functions to slices
 * depending on the slice type
 */

#include "boolean.h"
#include "pystip.h"
#include "pyslice.h"

/* Solve in a number of half-moves
 * @param si identifies slice
 * @param n exact number of half moves until end state has to be reached
 * @return length of solution found, i.e.:
 *         n+4 the move leading to the current position has turned out
 *             to be illegal
 *         n+2 no solution found
 *         n   solution found
 */
stip_length_type help_solve_in_n(slice_index si, stip_length_type n);

/* Solve a slice
 * @param si slice index
 * @return whether there is a solution and (to some extent) why not
 */
has_solution_type help_solve(slice_index si);

/* Determine whether there is a solution in n half moves.
 * @param si slice index of slice being solved
 * @param n exact number of half moves until end state has to be reached
 * @return length of solution found, i.e.:
 *         n+4 the move leading to the current position has turned out
 *             to be illegal
 *         n+2 no solution found
 *         n   solution found
 */
stip_length_type help_has_solution_in_n(slice_index si, stip_length_type n);

/* Determine whether a slice has a solution
 * @param si slice index
 * @return true iff slice si has a solution
 */
has_solution_type help_has_solution(slice_index si);

#endif
