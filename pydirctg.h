#if !defined(PYDIRCTG_H)
#define PYDIRCTG_H

/* STDirectAttack
 * Deals with short ends in direct branches
 * Implements the direct attacker interface
 */

#include "pydirect.h"

/* Determine whether the defense just played defends against the threats.
 * @param threats table containing the threats
 * @param len_threat length of threat(s) in table threats
 * @param si slice index
 * @param n maximum number of half moves until goal
 * @param curr_max_nr_nontrivial remaining maximum number of
 *                               allowed non-trivial variations
 * @return true iff the defense defends against at least one of the
 *         threats
 */
boolean direct_defense_are_threats_refuted_in_n(table threats,
                                                stip_length_type len_threat,
                                                slice_index si,
                                                stip_length_type n,
                                                int curr_max_nr_nontrivial);

/* Determine whether there is a solution in n half moves.
 * @param si slice index of slice being solved
 * @param n maximum number of half moves until end state has to be reached
 * @param curr_max_nr_nontrivial remaining maximum number of
 *                               allowed non-trivial variations
 * @return whether there is a solution and (to some extent) why not
 */
has_solution_type
direct_defense_direct_has_solution_in_n(slice_index si,
                                        stip_length_type n,
                                        int curr_max_nr_nontrivial);

/* Determine and write solution(s): add first moves to table (as
 * threats for the parent slice. First consult hash table.
 * @param continuations table where to add first moves
 * @param si slice index of slice being solved
 * @param n maximum number of half moves until end state has to be reached
 */
void direct_defense_direct_solve_continuations_in_n(table continuations,
                                                    slice_index si,
                                                    stip_length_type n);

/* Find refutations after a move of the attacking side at a nested level.
 * @param si slice index
 * @param n maximum number of half moves until end state has to be reached
 * @param curr_max_nr_nontrivial remaining maximum number of
 *                               allowed non-trivial variations
 * @return attacker_has_reached_deadend if we are in a situation where
 *              the position after the attacking move is to be
 *              considered hopeless for the attacker
 *         attacker_has_solved_next_slice if the attacking move has solved the branch
 *         found_refutations if there is a refutation
 *         found_no_refutation otherwise
 */
quantity_of_refutations_type
direct_attack_find_refutations_in_n(slice_index si,
                                    stip_length_type n,
                                    int curr_max_nr_nontrivial);

/* Solve postkey play play after the move that has just
 * been played in the current ply.
 * @param si slice index
 * @param n maximum number of half moves until goal
 */
boolean direct_attack_solve_postkey_in_n(slice_index si, stip_length_type n);

/* Solve a slice
 * @param si slice index
 * @return true iff >=1 solution was found
 */
boolean direct_defense_solve(slice_index si);

/* Solve a slice at root level
 * @param si slice index
 * @return true iff >=1 solution was found
 */
boolean direct_guard_root_solve(slice_index si);

/* Solve postkey play at root level.
 * @param refutations table containing the refutations (if any)
 * @param si slice index
 * @return true iff >=1 solution was found
 */
boolean direct_attack_root_solve_postkey(table refutations, slice_index si);

/* Find refutations after a move of the attacking side at root level.
 * @param refutations table where to store refutations
 * @param si slice index
 * @return attacker_has_reached_deadend if we are in a situation where
 *            the attacking move is to be considered to have failed, e.g.:
 *            if the defending side is immobile and shouldn't be
 *            if some optimisation tells us so
 *         attacker_has_solved_next_slice if the attacking move has
 *            solved the branch
 *         found_refutations if refutations contains some refutations
 *         found_no_refutation otherwise
 */
quantity_of_refutations_type
direct_attack_root_find_refutations(table refutations, slice_index si);

/* Write the key just played
 * @param si slice index
 * @param type type of attack
 */
void direct_attack_root_write_key(slice_index si, attack_type type);

/* Impose the starting side on a stipulation
 * @param si identifies branch
 * @param st address of structure that holds the state of the traversal
 * @return true iff the operation is successful in the subtree of
 *         which si is the root
 */
boolean direct_attack_impose_starter(slice_index si, slice_traversal *st);

/* Impose the starting side on a stipulation
 * @param si identifies branch
 * @param st address of structure that holds the state of the traversal
 * @return true iff the operation is successful in the subtree of
 *         which si is the root
 */
boolean direct_defense_impose_starter(slice_index si, slice_traversal *st);

/* Instrument a branch with STDirectAttack and STDirectDefense slices
 * @param si root of branch to be instrumented
 * @param towards_goal identifies slice leading towards goal
 */
void slice_insert_direct_guards(slice_index si, slice_index towards_goal);

#endif
