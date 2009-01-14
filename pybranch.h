#if !defined(PYBRANCH_H)
#define PYBRANCH_H

#include "py.h"
#include "pystip.h"
#include "boolean.h"

/* This module provides functionality dealing with STBranch*
 * stipulation slices.
 */

/* Is there no chance left for the starting side at the move to win?
 * E.g. did the defender just capture that attacker's last potential
 * mating piece?
 * @param leaf leaf's slice index
 * @return true iff starter must resign
 */
boolean branch_must_starter_resign(slice_index leaf);

/* Write a priori unsolvability (if any) of a slice (e.g. forced
 * reflex mates).
 * Assumes slice_must_starter_resign(si)
 * @param si slice index
 */
void branch_write_unsolvability(slice_index si);

/* Determine and write continuations at end of branch slice
 * @param continuations table where to store continuing moves
 *                      (e.g. threats)
 * @param si index of branch slice
 */
void branch_end_solve_continuations(int continuations, slice_index si);

/* Find and write set play
 * @param si slice index
 * @return true iff >= 1 set play was found
 */
boolean branch_root_end_solve_setplay(slice_index si);

/* Find and write set play provided every set move leads to end
 * @param si slice index
 * @return true iff every defender's move leads to end
 */
boolean branch_root_end_solve_complete_set(slice_index si);

/* Write the key just played, then continue solving at end of branch
 * slice to find and write the post key play (threats, variations)
 * @param si slice index
 * @param type type of attack
 */
void branch_root_end_write_key_solve_postkey(slice_index si,
                                               attack_type type);

/* Solve at root level at the end of a branch slice
 * @param si slice index
 */
void branch_root_end_solve(slice_index si);

/* Continue solving at the end of a branch slice
 * @param si slice index
 * @return true iff >=1 solution was found
 */
boolean branch_end_solve(slice_index si);

/* Find and write variations starting at end of branch slice
 * @param si slice index
 */
void branch_end_solve_variations(slice_index si);

/* Determine whether there is a solution at the end of a quodlibet
 * slice. 
 * @param si slice index
 * @return true iff slice si has a solution
 */
boolean branch_end_has_solution(slice_index si);

/* Determine whether a branch slice.has just been solved with the
 * just played move by the non-starter
 * @param si slice identifier
 * @return true iff the non-starting side has just solved
 */
boolean branch_end_has_non_starter_solved(slice_index si);

/* Determine whether the starting side has made such a bad move that
 * it is clear without playing further that it is not going to win.
 * E.g. in s# or r#, has it taken the last potential mating piece of
 * the defender?
 * @param si slice identifier
 * @return true iff starter has lost
 */
boolean branch_has_starter_apriori_lost(slice_index si);

/* Determine whether the attacker has won with his move just played
 * independently of the non-starter's possible further play during the
 * current slice.
 * @param si slice identifier
 * @return true iff the starter has won
 */
boolean branch_has_starter_won(slice_index si);

/* Detect starter field with the starting side if possible. 
 * @param si identifies slice
 * @param is_duplex is this for duplex?
 */
void branch_detect_starter(slice_index si, boolean is_duplex);

#endif
