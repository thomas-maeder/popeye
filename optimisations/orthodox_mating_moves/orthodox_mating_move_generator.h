#if !defined(OPTIMISATIONS_ORTHODOX_MATING_MOVES_GENERATOR_H)
#define OPTIMISATIONS_ORTHODOX_MATING_MOVES_GENERATOR_H

#include "stipulation/battle_play/attack_play.h"

/* This module provides a generator for moves that are supposed to reach a goal.
 * The generator is optimised for mostly orthodox problems.
 */

/* Optimise move generation by inserting orthodox mating move generators
 * @param si identifies the root slice of the stipulation
 */
void stip_optimise_with_orthodox_mating_move_generators(slice_index si);

/* Determine whether there is a solution in n half moves.
 * @param si slice index
 * @param n maximal number of moves
 * @param n_max_unsolvable maximum number of half-moves that we
 *                         know have no solution
 * @return length of solution found, i.e.:
 *            slack_length_battle-2 defense has turned out to be illegal
 *            <=n length of shortest solution found
 *            n+2 no solution found
 */
stip_length_type
orthodox_mating_move_generator_can_attack(slice_index si,
                                          stip_length_type n,
                                          stip_length_type n_max_unsolvable);

/* Try to solve in n half-moves after a defense.
 * @param si slice index
 * @param n maximum number of half moves until goal
 * @param n_max_unsolvable maximum number of half-moves that we
 *                         know have no solution
 * @note n==n_max_unsolvable means that we are solving refutations
 * @return length of solution found and written, i.e.:
 *            slack_length_battle-2 defense has turned out to be illegal
 *            <=n length of shortest solution found
 *            n+2 no solution found
 */
stip_length_type
orthodox_mating_move_generator_attack(slice_index si,
                                      stip_length_type n,
                                      stip_length_type n_max_unsolvable);

#endif
