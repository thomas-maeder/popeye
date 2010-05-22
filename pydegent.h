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
 */
void stip_insert_degenerate_tree_guards(void);

/* Determine whether there is a solution in n half moves.
 * @param si slice index of slice being solved
 * @param n maximum number of half moves until end state has to be reached
 * @param n_min minimal number of half moves to try
 * @return length of solution found, i.e.:
 * @return length of solution found, i.e.:
 *            n_min-2 defense has turned out to be illegal
 *            n_min..n length of shortest solution found
 *            n+2 no solution found
 */
stip_length_type
degenerate_tree_direct_has_solution_in_n(slice_index si,
                                         stip_length_type n,
                                         stip_length_type n_min);

/* Solve a slice
 * @param si slice index
 * @param n maximum number of half moves until goal
 * @param n_min minimal number of half moves to try
 * @return length of solution found and written, i.e.:
 *            n_min-2 defense has turned out to be illegal
 *            n_min..n length of shortest solution found
 *            n+2 no solution found
 */
stip_length_type degenerate_tree_direct_solve_in_n(slice_index si,
                                                   stip_length_type n,
                                                   stip_length_type n_min);

#endif
