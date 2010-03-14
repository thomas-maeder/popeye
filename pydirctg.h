#if !defined(PYDIRCTG_H)
#define PYDIRCTG_H

/* STDirectDefense
 * Deals with short ends in direct branches
 * Implements the direct attacker interface
 */

#include "stipulation/battle_play/attack_play.h"

/* Allocate a STDirectDefense slice
 * @param length maximum number of half-moves of slice (+ slack)
 * @param min_length minimum number of half-moves of slice (+ slack)
 * @param proxy_to_goal identifies slice leading towards goal
 * @return index of allocated slice
 */
slice_index alloc_direct_defense(stip_length_type length,
                                 stip_length_type min_length,
                                 slice_index proxy_to_goal);

/* Allocate a STDirectDefenseRootSolvableFilter slice
 * @param length maximum number of half-moves of slice (+ slack)
 * @param min_length minimum number of half-moves of slice (+ slack)
 * @param proxy_to_goal identifies slice leading towards goal
 * @return index of allocated slice
 */
slice_index
alloc_direct_defense_root_solvable_filter(stip_length_type length,
                                          stip_length_type min_length,
                                          slice_index proxy_to_goal);

/* Insert root slices
 * @param si identifies (non-root) slice
 * @param st address of structure representing traversal
 */
void direct_defense_insert_root(slice_index si, stip_structure_traversal *st);

/* Spin off a set play slice at root level
 * @param si slice index
 * @param st state of traversal
 */
void direct_defense_root_make_setplay_slice(slice_index si, stip_structure_traversal *st);

/* Find the first postkey slice and deallocate unused slices on the
 * way to it
 * @param si slice index
 * @param st address of structure capturing traversal state
 */
void direct_defense_root_reduce_to_postkey_play(slice_index si,
                                                stip_structure_traversal *st);

/* Determine whether the defense just played defends against the threats.
 * @param threats table containing the threats
 * @param len_threat length of threat(s) in table threats
 * @param si slice index
 * @param n maximum number of half moves until goal
 * @return true iff the defense defends against at least one of the
 *         threats
 */
boolean direct_defense_are_threats_refuted_in_n(table threats,
                                                stip_length_type len_threat,
                                                slice_index si,
                                                stip_length_type n);

/* Determine whether there is a solution in n half moves.
 * @param si slice index of slice being solved
 * @param n maximum number of half moves until end state has to be reached
 * @param n_min minimal number of half moves to try
 * @return length of solution found, i.e.:
 *            0 defense put defender into self-check
 *            n_min..n length of shortest solution found
 *            >n no solution found
 *         (the second case includes the situation in self
 *         stipulations where the defense just played has reached the
 *         goal (in which case n_min<slack_length_battle and we return
 *         n_min)
 */
stip_length_type
direct_defense_direct_has_solution_in_n(slice_index si,
                                        stip_length_type n,
                                        stip_length_type n_min);

/* Determine and write the threats after the move that has just been
 * played.
 * @param threats table where to add threats
 * @param si slice index
 * @param n maximum number of half moves until goal
 * @param n_min minimal number of half moves to try
 * @return length of threats
 *         (n-slack_length_battle)%2 if the attacker has something
 *           stronger than threats (i.e. has delivered check)
 *         n+2 if there is no threat
 */
stip_length_type
direct_defense_direct_solve_threats_in_n(table threats,
                                         slice_index si,
                                         stip_length_type n,
                                         stip_length_type n_min);

/* Solve a slice at root level
 * @param si slice index
 * @return true iff >=1 solution was found
 */
boolean direct_defense_root_solve(slice_index si);

/* Solve a slice
 * @param si slice index
 * @param n maximum number of half moves until goal
 * @param n_min minimal number of half moves to try
 * @return number of half moves effectively used
 *         n+2 if no solution was found
 *         (n-slack_length_battle)%2 if the previous move led to a
 *            dead end (e.g. self-check)
 */
stip_length_type direct_defense_solve_in_n(slice_index si,
                                           stip_length_type n,
                                           stip_length_type n_min);

/* Impose the starting side on a stipulation
 * @param si identifies branch
 * @param st address of structure that holds the state of the traversal
 */
void direct_defense_impose_starter(slice_index si, stip_structure_traversal *st);

/* Instrument a branch with STDirectDefense slices
 * @param si root of branch to be instrumented
 * @param to_goal identifies slice leading towards goal
 * @return identifier of branch entry slice after insertion
 */
slice_index slice_insert_direct_guards(slice_index si, slice_index to_goal);

#endif
