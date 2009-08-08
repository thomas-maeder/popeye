#if !defined(PYDIRECT_H)
#define PYDIRECT_H

/* Interface for dynamically dispatching direct play functions to slices
 * depending on the slice type
 */

#include "py.h"
#include "pystip.h"
#include "pytable.h"

/* Solve in a number of half-moves
 * @param si identifies slice
 * @param n number of half moves until end state has to be reached
 * @return true iff >=1 solution was found
 */
boolean direct_solve_in_n(slice_index si, stip_length_type n);

/* Determine whether there is a solution in n half moves.
 * @param si slice index of slice being solved
 * @param n number of half moves until end state has to be reached
 * @param curr_max_nr_nontrivial remaining maximum number of
 *                               allowed non-trivial variations
 * @return true iff >= 1 solution has been found
 */
boolean direct_has_solution_in_n(slice_index si,
                                 stip_length_type n,
                                 int curr_max_nr_nontrivial);

/* Determine and write solution(s): add first moves to table (as
 * threats for the parent slice. First consult hash table.
 * @param continuations table where to add first moves
 * @param si slice index of slice being solved
 * @param n number of half moves until end state has to be reached
 */
void direct_solve_continuations_in_n(table continuations,
                                     slice_index si,
                                     stip_length_type n);

/* Determine whether the defender wins after a move by the attacker
 * @param si slice index
 * @param n (odd) number of half moves until goal
 * @param curr_max_nr_nontrivial remaining maximum number of
 *                               allowed non-trivial variations
 * @return true iff defender wins
 */
boolean direct_defender_does_defender_win(slice_index si,
                                          stip_length_type n,
                                          int curr_max_nr_nontrivial);

/* Is the defense just played a refutation?
 * @param si slice index
 * @param n (even) number of half moves until goal
 * @param curr_max_nr_nontrivial remaining maximum number of
 *                               allowed non-trivial variations
 * @return true iff the defense is a refutation
 */
boolean direct_defender_is_refuted(slice_index si,
                                   stip_length_type n,
                                   int curr_max_nr_nontrivial);

/* Solve postkey play play after the move that has just
 * been played in the current ply.
 * @param si slice index
 * @param n (odd) number of half moves until goal
 * @return true iff >=1 solution was found
 */
boolean direct_defender_solve_postkey_in_n(slice_index si, stip_length_type n);

/* Solve postkey play at root level.
 * @param refutations table containing the refutations (if any)
 * @param si slice index
 */
/* TODO get rid of this */
void direct_defender_root_solve_postkey(table refutations, slice_index si);

/* Try to finish the solution of the next slice starting with the key
 * move just played. 
 * @param si slice index
 * @return true iff finishing the solution was successful.
 */
boolean direct_defender_finish_solution_next(slice_index si);

/* Find refutations after a move of the attacking side at root level.
 * @param t table where to store refutations
 * @param si slice index
 * @return max_nr_refutations+1 if
 *            if the defending side is immobile (it shouldn't be here!)
 *            if the defending side has more non-trivial moves than allowed
 *            if the defending king has more flights than allowed
 *            if there is no threat in <= the maximal threat length
 *               as entered by the user
 *         number (0..max_nr_refutations) of refutations otherwise
 */
unsigned int direct_defender_find_refutations(table refutations,
                                              slice_index si);

#endif
