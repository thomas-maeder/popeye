#if !defined(PYBRADD_H)
#define PYBRADD_H

#include "boolean.h"
#include "pystip.h"
#include "pyslice.h"
#include "pytable.h"

/* This module provides functionality dealing with the defending side
 * in STBranchDirect stipulation slices.
 */

/* Write a priori unsolvability (if any) of a slice (e.g. forced
 * reflex mates).
 * Assumes slice_must_starter_resign(si)
 * @param si slice index
 */
void branch_d_defender_write_unsolvability(slice_index si);

/* Determine whether a side has reached the goal
 * @param just_moved side that has just moved
 * @param si slice index
 * @return true iff just_moved has reached the goal
 */
boolean branch_d_defender_is_goal_reached(Side just_moved, slice_index si);

/* Is there no chance left for the starting side at the move to win?
 * E.g. did the defender just capture that attacker's last potential
 * mating piece?
 * @param si identifies slice
 * @return true iff starter must resign
 */
boolean branch_d_defender_must_starter_resign(slice_index si);

/* Determine whether the starting side has made such a bad move that
 * it is clear without playing further that it is not going to win.
 * E.g. in s# or r#, has it taken the last potential mating piece of
 * the defender?
 * @param si slice identifier
 * @return true iff starter has lost
 */
boolean branch_d_defender_has_starter_apriori_lost(slice_index si);

/* Is the defense just played a refutation?
 * @param si slice index
 * @param n (even) number of half moves until goal
 * @return true iff the defense is a refutation
 */
boolean branch_d_defender_is_refuted(slice_index si, stip_length_type n);

/* Determine whether the attacker has won with his move just played
 * independently of the non-starter's possible further play during the
 * current slice.
 * @param si slice identifier
 * @return true iff the starter has won
 */
boolean branch_d_defender_has_starter_won(slice_index si);

/* Determine whether the attacker has reached slice si's goal with his
 * move just played.
 * @param si slice identifier
 * @return true iff the starter reached the goal
 */
boolean branch_d_defender_has_starter_reached_goal(slice_index si);

/* The enumerators in the following enumeration type are sorted in
 * descending possibilities of the defender.
 */
typedef enum
{
  already_won,
  win,
  loss,
  already_lost
} d_defender_win_type;

/* Determine whether the defender wins after a move by the attacker
 * @param si slice index
 * @param n (odd) number of half moves until goal
 * @return whether the defender wins or loses, and how fast
 */
d_defender_win_type branch_d_defender_does_defender_win(slice_index si,
                                                        stip_length_type n);

/* Determine whether a slice.has just been solved with the just played
 * move by the non-starter
 * @param si slice identifier
 * @return true iff the non-starting side has just solved
 */
boolean branch_d_defender_has_non_starter_solved(slice_index si);

/* Solve postkey play play after the move that has just
 * been played in the current ply.
 * @param si slice index
 * @param n (odd) number of half moves until goal
 */
void branch_d_defender_solve_postkey_in_n(slice_index si, stip_length_type n);

/* Try to finish the solution of the next slice starting with the key
 * move just played. 
 * @param si slice index
 * @return true iff finishing the solution was successful.
 */
boolean branch_d_defender_finish_solution_next(slice_index si);

/* Solve at non-root level.
 * @param si slice index
 */
void branch_d_defender_solve(slice_index si);

/* Find solutions in next slice
 * @param si slice index
 * @return true iff >=1 solution has been found
 */
boolean branch_d_defender_solve_next(slice_index si);

/* Solve postkey play at root level.
 * @param refutations table containing the refutations (if any)
 * @param si slice index
 */
void branch_d_defender_root_solve_postkey(table refutations, slice_index si);

/* Solve at root level.
 * @param si slice index
 */
void branch_d_defender_root_solve(slice_index si);

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
unsigned int branch_d_defender_find_refutations(table refutations,
                                                slice_index si);

/* Spin off a set play slice at non-root-level
 * @param si slice index
 * @return set play slice spun off; no_slice if not applicable
 */
slice_index branch_d_defender_make_setplay_slice(slice_index si);

/* Detect starter field with the starting side if possible. 
 * @param si identifies slice
 * @param same_side_as_root does si start with the same side as root?
 * @return does the leaf decide on the starter?
 */
who_decides_on_starter
branch_d_defender_detect_starter(slice_index si, boolean same_side_as_root);

/* Impose the starting side on a slice.
 * @param si identifies branch
 * @param s starting side of slice
 */
void branch_d_defender_impose_starter(slice_index si, Side s);

#endif
