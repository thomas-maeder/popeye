#if !defined(PYQUODLI_H)
#define PYQUODLI_H

#include "py.h"
#include "pyslice.h"
#include "boolean.h"

/* This module provides functionality dealing with quodlibet
 * (i.e. logical OR) stipulation slices.
 */


/* Allocate a quodlibet slice.
 * @param op1 1st operand
 * @param op2 2nd operand
 * @return index of allocated slice
 */
slice_index alloc_quodlibet_slice(slice_index op1, slice_index op2);

/* Is there no chance left for the starting side at the move to win?
 * E.g. did the defender just capture that attacker's last potential
 * mating piece?
 * @param si slice index
 * @return true iff starter must resign
 */
boolean quodlibet_must_starter_resign(slice_index si);

/* Write a priori unsolvability (if any) of a slice (e.g. forced
 * reflex mates).
 * Assumes slice_must_starter_resign(si)
 * @param si slice index
 */
void quodlibet_write_unsolvability(slice_index si);

/* Determine and write continuations of a quodlibet slice
 * @param continuations table where to store continuing moves
 *                      (e.g. threats)
 * @param si index of quodlibet slice
 */
void quodlibet_solve_continuations(int continuations, slice_index si);

/* Find and write set play
 * @param si slice index
 * @return true iff >= 1 set play was found
 */
boolean quodlibet_root_solve_setplay(slice_index si);

/* Find and write set play provided every set move leads to end
 * @param si slice index
 * @return true iff every defender's move leads to end
 */
boolean quodlibet_root_solve_complete_set(slice_index si);

/* Write the key just played, then solve the post key play (threats,
 * variations) of a quodlibet slice.
 * @param si slice index
 * @param type type of attack
 */
void quodlibet_root_write_key_solve_postkey(slice_index si,
                                            attack_type type);

/* Find and write variations of a quodlibet slice.
 * @param si slice index
 */
void quodlibet_solve_variations(slice_index si);

/* Determine whether a quodlibet slice jas a solution
 * @param si slice index
 * @return true iff slice si has a solution
 */
boolean quodlibet_has_solution(slice_index si);

/* Determine whether a quodlibet slice.has just been solved with the
 * just played move by the non-starter
 * @param si slice identifier
 * @return true iff the non-starting side has just solved
 */
boolean quodlibet_has_non_starter_solved(slice_index si);

/* Determine whether the attacker has won with his move just played
 * independently of the non-starter's possible further play during the
 * current slice.
 * @param si slice identifier
 * @return true iff the starter has won
 */
boolean quodlibet_has_starter_won(slice_index si);

/* Determine whether the attacker has reached slice si's goal with his
 * move just played.
 * @param si slice identifier
 * @return true iff the starter reached the goal
 */
boolean quodlibet_has_starter_reached_goal(slice_index si);

/* Determine whether the starting side has made such a bad move that
 * it is clear without playing further that it is not going to win.
 * E.g. in s# or r#, has it taken the last potential mating piece of
 * the defender?
 * @param si slice identifier
 * @return true iff starter has lost
 */
boolean quodlibet_has_starter_apriori_lost(slice_index si);

/* Solve a quodlibet slice
 * @param si slice index
 * @return true iff >=1 solution was found
 */
boolean quodlibet_solve(slice_index si);

/* Solve a quodlibet slice at root level
 * @param si slice index
 * @return true iff >=1 solution was found
 */
boolean quodlibet_root_solve(slice_index si);

/* Detect starter field with the starting side if possible. 
 * @param si identifies slice
 * @param is_duplex is this for duplex?
 * @param same_side_as_root does si start with the same side as root?
 * @return does the leaf decide on the starter?
 */
who_decides_on_starter quodlibet_detect_starter(slice_index si,
                                                boolean is_duplex,
                                                boolean same_side_as_root);

/* Impose the starting side on a slice.
 * @param si identifies slice
 * @param s starting side of slice
 */
void quodlibet_impose_starter(slice_index si, Side s);

#endif
