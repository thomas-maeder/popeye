#if !defined(SOLVING_BATTLE_PLAY_CONTINUATION_H)
#define SOLVING_BATTLE_PLAY_CONTINUATION_H

#include "solving/machinery/solve.h"

/* This module provides functionality dealing with solutions and
 * continuations.
 */

/* Allocate a STContinuationSolver defender slice.
 * @return index of allocated slice
 */
slice_index alloc_continuation_solver_slice(void);

/* Try to defend after an attacking move
 * When invoked with some n, the function assumes that the key doesn't
 * solve in less than n half moves.
 * @param si slice index
 * @param n maximum number of half moves until end state has to be reached
 *         <=n solved  - <=acceptable number of refutations found
 *                       return value is maximum number of moves
 *                       (incl. defense) needed
 *         n+2 refuted - >acceptable number of refutations found */
void continuation_solver_solve(slice_index si);

/* Instrument the solving machinery with STContinuationSolver slices
 * @param root_slice root slice of the solving machinery
 */
void solving_insert_continuation_solvers(slice_index si);

#endif
