#if !defined(SOLVING_BATTLE_PLAY_CONTINUATION_H)
#define SOLVING_BATTLE_PLAY_CONTINUATION_H

#include "stipulation/battle_play/defense_play.h"

/* This module provides functionality dealing with solutions and
 * continuations.
 */

/* Allocate a STContinuationSolver defender slice.
 * @return index of allocated slice
 */
slice_index alloc_continuation_solver_slice(void);

/* Traversal of the moves
 * @param si identifies root of subtree
 * @param st address of structure representing traversal
 */
void stip_traverse_moves_continuation_solver(slice_index si,
                                             stip_moves_traversal *st);

/* Try to defend after an attacking move
 * When invoked with some n, the function assumes that the key doesn't
 * solve in less than n half moves.
 * @param si slice index
 * @param n maximum number of half moves until end state has to be reached
 * @return <=n solved  - return value is maximum number of moves
 *                       (incl. defense) needed
 *         n+2 refuted - <=acceptable number of refutations found
 *         n+4 refuted - >acceptable number of refutations found
 */
stip_length_type continuation_solver_defend(slice_index si, stip_length_type n);

/* Instrument the stipulation structure with STContinuationSolver slices
 * @param root_slice root slice of the stipulation
 */
void stip_insert_continuation_solvers(slice_index si);

#endif
