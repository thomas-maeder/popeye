#if !defined(OPTIMISATION_KILLER_MOVE_COLLECTOR_H)
#define OPTIMISATION_KILLER_MOVE_COLLECTOR_H

#include "solving/solve.h"

/* This module provides functionality dealing with the attacking side
 * in STKillerDefenseCollector stipulation slices.
 */

/* Allocate a STKillerDefenseCollector slice.
 * @return index of allocated slice
 */
slice_index alloc_killer_defense_collector_slice(void);

/* Try to solve in n half-moves.
 * @param si slice index
 * @param n maximum number of half moves
 * @return length of solution found and written, i.e.:
 *            slack_length-2 the move just played or being played is illegal
 *            <=n length of shortest solution found
 *            n+2 no solution found
 */
stip_length_type killer_defense_collector_solve(slice_index si,
                                                 stip_length_type n);

/* Allocate a STKillerAttackCollector slice.
 * @return index of allocated slice
 */
slice_index alloc_killer_attack_collector_slice(void);

/* Try to solve in n half-moves.
 * @param si slice index
 * @param n maximum number of half moves
 * @return length of solution found and written, i.e.:
 *            slack_length-2 the move just played or being played is illegal
 *            <=n length of shortest solution found
 *            n+2 no solution found
 */
stip_length_type killer_attack_collector_solve(slice_index si,
                                                stip_length_type n);

#endif
