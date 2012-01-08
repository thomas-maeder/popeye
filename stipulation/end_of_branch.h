#if !defined(STIPULATION_END_OF_BRANCH_H)
#define STIPULATION_END_OF_BRANCH_H

/* Branch fork - branch decides that when to continue play in branch
 * and when to change to slice representing subsequent play
 */

#include "stipulation/battle_play/defense_play.h"
#include "stipulation/help_play/play.h"

/* Allocate a STEndOfBranch slice.
 * @param to_goal identifies slice leading towards goal
 * @return index of allocated slice
 */
slice_index alloc_end_of_branch_slice(slice_index to_goal);

/* Traverse a subtree
 * @param si root slice of subtree
 * @param st address of structure defining traversal
 */
void stip_traverse_structure_end_of_branch(slice_index si,
                                           stip_structure_traversal *st);

/* Traversal of the moves beyond a help fork slice
 * @param si identifies root of subtree
 * @param st address of structure representing traversal
 */
void stip_traverse_moves_end_of_branch(slice_index si,
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
stip_length_type end_of_branch_defend(slice_index si, stip_length_type n);

/* Determine whether there are defenses after an attacking move
 * @param si slice index
 * @param n maximum number of half moves until end state has to be reached
 * @return <=n solved  - return value is maximum number of moves
 *                       (incl. defense) needed
 *         n+2 refuted - <=acceptable number of refutations found
 *         n+4 refuted - >acceptable number of refutations found
 */
stip_length_type end_of_branch_can_defend(slice_index si, stip_length_type n);

/* Solve in a number of half-moves
 * @param si identifies slice
 * @param n exact number of half moves until end state has to be reached
 * @return length of solution found, i.e.:
 *         n+4 the move leading to the current position has turned out
 *             to be illegal
 *         n+2 no solution found
 *         n   solution found
 */
stip_length_type end_of_branch_help(slice_index si, stip_length_type n);

/* Determine whether there is a solution in n half moves.
 * @param si slice index of slice being solved
 * @param n exact number of half moves until end state has to be reached
 * @return length of solution found, i.e.:
 *         n+4 the move leading to the current position has turned out
 *             to be illegal
 *         n+2 no solution found
 *         n   solution found
 */
stip_length_type end_of_branch_can_help(slice_index si, stip_length_type n);

#endif
