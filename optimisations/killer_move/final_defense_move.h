#if !defined(OPTIMISATION_KILLER_MOVE_FINAL_DEFENSE_MOVE_H)
#define OPTIMISATION_KILLER_MOVE_FINAL_DEFENSE_MOVE_H

#include "solving/solve.h"
#include "stipulation/structure_traversal.h"

/* This module provides functionality dealing with the defending side
 * in STKillerMoveFinalDefenseMove stipulation slices.
 * These provide an optimisation in stipulations where a defense move
 * leads to a goal (e.g. selfmate): if we have a "killer" piece, its
 * moves will be prefered when looking for refutations in the final
 * move.
 */

/* Optimise a STMoveGenerator slice for defending against a goal
 * @param si identifies slice to be optimised
 * @param st structure holding traversal that led to the generator to be
 *        optimised
 */
void killer_move_optimise_final_defense_move(slice_index si,
                                             stip_structure_traversal *st);

/* Try to solve in n half-moves.
 * @param si slice index
 * @param n maximum number of half moves
 * @return length of solution found and written, i.e.:
 *            previous_move_is_illegal the move just played (or being played)
 *                                     is illegal
 *            immobility_on_next_move  the moves just played led to an
 *                                     unintended immobility on the next move
 *            <=n+1 length of shortest solution found (n+1 only if in next
 *                                     branch)
 *            n+2 no solution found in this branch
 *            n+3 no solution found in next branch
 */
stip_length_type killer_move_final_defense_move_solve(slice_index si,
                                                       stip_length_type n);

#endif
