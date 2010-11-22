#if !defined(OUTPUT_PLAINTEXT_TREE_REFLEX_ATTACK_WRITER_H)
#define OUTPUT_PLAINTEXT_TREE_REFLEX_ATTACK_WRITER_H

/* In reflex stipulations, write attacks forced by reflex-specific refutations.
 */

#include "stipulation/battle_play/attack_play.h"

/* Allocate a STOutputPlaintextTreeReflexAttackWriter slice
 * @param avoided prototype of slice that must not be solvable
 * @return index of allocated slice
 */
slice_index alloc_reflex_attack_writer(slice_index avoided);

/* Try to solve in n half-moves after a defense.
 * @param si slice index
 * @param n maximum number of half moves until goal
 * @param n_max_unsolvable maximum number of half-moves that we
 *                         know have no solution
 * @return length of solution found and written, i.e.:
 *            slack_length_battle-2 defense has turned out to be illegal
 *            <=n length of shortest solution found
 *            n+2 no solution found
 */
stip_length_type
reflex_attack_writer_solve_in_n(slice_index si,
                                stip_length_type n,
                                stip_length_type n_max_unsolvable);

/* Determine whether there is a solution in n half moves.
 * @param si slice index of slice being solved
 * @param n maximum number of half moves until end state has to be reached
 * @param n_max_unsolvable maximum number of half-moves that we
 *                         know have no solution
 * @return length of solution found, i.e.:
 *            slack_length_battle-2 defense has turned out to be illegal
 *            <=n length of shortest solution found
 *            n+2 no solution found
 */
stip_length_type
reflex_attack_writer_has_solution_in_n(slice_index si,
                                       stip_length_type n,
                                       stip_length_type n_max_unsolvable);

#endif
