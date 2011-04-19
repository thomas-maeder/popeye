#if !defined(OPTIMISATIONS_COUNTNROPPONENTMOVES_MOVE_GENERATOR_H)
#define OPTIMISATIONS_COUNTNROPPONENTMOVES_MOVE_GENERATOR_H

#include "stipulation/battle_play/defense_play.h"

/* This module provides functionality dealing with the defending side in
 * STDefenseMoveGenerator stipulation slices.
 * These slices provide unoptimised move generation
 */

/* Reset the enabled state
 */
void reset_countnropponentmoves_defense_move_optimisation(void);

/* Disable the optimisation for defenses by a side
 * @param side side for which to disable the optimisation
 */
void disable_countnropponentmoves_defense_move_optimisation(Side side);

/* Try to defend after an attacking move
 * When invoked with some n, the function assumes that the key doesn't
 * solve in less than n half moves.
 * @param si slice index
 * @param n maximum number of half moves until end state has to be reached
 * @param n_max_unsolvable maximum number of half-moves that we
 *                         know have no solution
 * @note n==n_max_unsolvable means that we are solving refutations
 * @return <=n solved  - return value is maximum number of moves
 *                       (incl. defense) needed
 *         n+2 refuted - <=acceptable number of refutations found
 *         n+4 refuted - >acceptable number of refutations found
 */
stip_length_type
countnropponentmoves_move_generator_defend(slice_index si,
                                           stip_length_type n,
                                           stip_length_type n_max_unsolvable);

/* Determine whether there are defenses after an attacking move
 * @param si slice index
 * @param n maximum number of half moves until end state has to be reached
 * @param n_max_unsolvable maximum number of half-moves that we
 *                         know have no solution
 * @return <=n solved  - return value is maximum number of moves
                         (incl. defense) needed
           n+2 refuted - <=acceptable number of refutations found
           n+4 refuted - >acceptable number of refutations found
 */
stip_length_type
countnropponentmoves_move_generator_can_defend(slice_index si,
                                               stip_length_type n,
                                               stip_length_type n_max_unsolvable);

/* Instrument stipulation with optimised move generation based on the number of
 * opponent moves
 * @param si identifies slice where to start
 */
void stip_optimise_with_countnropponentmoves(slice_index si);

#endif
