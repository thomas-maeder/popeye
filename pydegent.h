#if !defined(PYDEGENT_H)
#define PYDEGENT_H

/* Implementation of the "degenerate tree" optimisation: if due to
 * options or conditions the tree of played moves degenerates to a
 * branch with few short twigs, determining whether a certain position
 * has a solution in 1 is almost as expensive whether it has a
 * solution in N.
 * The optimisation prevents us from wasting time looking for a
 * solution in all lengths from 1 to N.
 */

#include "stipulation/battle_play/attack_play.h"

/* Reset the degenerate tree setting
 */
void reset_degenerate_tree(void);

/* Initialize the degenerate tree setting
 * @param max_length_short only look for a solution in
 * 1..max_length_short and full length
 */
void init_degenerate_tree(stip_length_type max_length_short);

/* Instrument stipulation with STDegenerateTree slices
 * @param si identifies slice where to start
 */
void stip_insert_degenerate_tree_guards(slice_index si);

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
stip_length_type degenerate_tree_can_attack(slice_index si,
                                            stip_length_type n,
                                            stip_length_type n_max_unsolvable);

#endif
