#if !defined(PYQUODLI_H)
#define PYQUODLI_H

#include "py.h"
#include "pyslice.h"
#include "boolean.h"

/* This module provides functionality dealing with quodlibet
 * (i.e. logical OR) stipulation slices.
 */

/* Allocate a quodlibet slice.
 * @param proxy1 1st operand
 * @param proxy2 2nd operand
 * @return index of allocated slice
 */
slice_index alloc_quodlibet_slice(slice_index proxy1, slice_index proxy2);

/* Determine whether a quodlibet slice jas a solution
 * @param si slice index
 * @return whether there is a solution and (to some extent) why not
 */
has_solution_type quodlibet_has_solution(slice_index si);

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
stip_length_type quodlibet_can_attack(slice_index si,
                                      stip_length_type n,
                                      stip_length_type n_max_unsolvable);

/* Solve a slice
 * @param si slice index
 * @return whether there is a solution and (to some extent) why not
 */
has_solution_type quodlibet_solve(slice_index si);

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
stip_length_type quodlibet_attack(slice_index si,
                                  stip_length_type n,
                                  stip_length_type n_max_unsolvable);

/* Detect starter field with the starting side if possible.
 * @param si identifies slice being traversed
 * @param st status of traversal
 */
void quodlibet_detect_starter(slice_index si, stip_structure_traversal *st);

#endif
