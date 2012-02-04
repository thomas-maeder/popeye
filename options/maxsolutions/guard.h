#if !defined(OPTIONS_MAXSOLUTIONS_GUARD_H)
#define OPTIONS_MAXSOLUTIONS_GUARD_H

#include "pyslice.h"
#include "stipulation/battle_play/defense_play.h"
#include "stipulation/help_play/play.h"

/* This module provides functionality dealing with
 * STMaxSolutionsGuard stipulation slice type.
 * Slices of this type make sure that solving stops after the maximum
 * number of solutions have been found
 */

/* Allocate a STMaxSolutionsGuard slice.
 * @return allocated slice
 */
slice_index alloc_maxsolutions_guard_slice(void);

/* Solve a slice
 * @param si slice index
 * @return whether there is a solution and (to some extent) why not
 */
has_solution_type maxsolutions_guard_solve(slice_index si);

/* Try to defend after an attacking move
 * When invoked with some n, the function assumes that the key doesn't
 * solve in less than n half moves.
 * @param si slice index
 * @param n maximum number of half moves until end state has to be reached
 * @return <slack_length_battle - no legal defense found
 *         <=n solved  - <=acceptable number of refutations found
 *                       return value is maximum number of moves
 *                       (incl. defense) needed
 *         n+2 refuted - >acceptable number of refutations found
 */
stip_length_type maxsolutions_guard_defend(slice_index si, stip_length_type n);


/* Determine and write the solution(s) in a help stipulation
 * @param si slice index of slice being solved
 * @param n exact number of half moves until end state has to be reached
 * @return length of solution found, i.e.:
 *         n+4 the move leading to the current position has turned out
 *             to be illegal
 *         n+2 no solution found
 *         n   solution found
 */
stip_length_type maxsolutions_guard_help(slice_index si, stip_length_type n);

#endif
