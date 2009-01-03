#if !defined(PYRECIPR_H)
#define PYRECIPR_H

#include "boolean.h"
#include "py.h"
#include "pystip.h"

/* This module provides functionality dealing with reciprocal
 * (i.e. logical AND) stipulation slices.
 */

/* Detect a priori unsolvability of a slice (e.g. because of forced
 * reflex mates)
 * @param si slice index
 * @return true iff slice is a priori unsolvable
 */
boolean reci_end_is_unsolvable(slice_index si);

/* Determine whether there is a solution at the end of a quodlibet
 * slice. 
 * @param si slice index
 * @return true iff slice si has a solution
 */
boolean reci_end_has_solution(slice_index si);

/* Determine whether a reciprocal slice.has just been solved with the
 * just played move by the non-starter
 * @param si slice identifier
 * @return true iff the non-starting side has just solved
 */
boolean reci_end_has_non_starter_solved(slice_index si);

/* Determine whether the defender has immediately won in direct play
 * with his move just played.
 * @param si slice identifier
 * @return true iff the defending side has directly won
 */
boolean d_reci_end_has_defender_won(slice_index si);

/* Determine whether the attacker has immediately lost in direct play
 * with his move just played.
 * @param si slice identifier
 * @return true iff the attacking side has directly lost
 */
boolean d_reci_end_has_attacker_lost(slice_index si);

/* Determine whether the attacker has immediately won in direct play
 * with his move just played.
 * @param si slice identifier
 * @return true iff the attacking side has directly won
 */
boolean d_reci_end_has_attacker_won(slice_index si);

/* Write a priori unsolvability (if any) of a slice (e.g. forced
 * reflex mates).
 * Assumes slice_is_unsolvable(si)
 * @param si slice index
 */
void reci_write_unsolvability(slice_index si);

/* Find and write variations from the end of a reciprocal slice.
 * @param si slice index
 */
void d_reci_end_solve_variations(slice_index si);

/* Determine and write continuations at end of reciprocal slice
 * @param continuations table where to store continuing moves
 *                      (e.g. threats)
 * @param si index of quodlibet slice
 */
void reci_end_solve_continuations(int continuations, slice_index si);

/* Find and write set play
 * @param si slice index
 */
void reci_root_end_solve_setplay(slice_index si);

/* Write the key just played, then solve the post key play (threats,
 * variations), starting at the end of a reciprocal slice.
 * @param si slice index
 * @param type type of attack
 */
void d_reci_root_end_write_key_solve_postkey(slice_index si,
                                             attack_type type);

/* Has the threat just played been refuted by the preceding defense?
 * @param si identifies stipulation slice
 * @return true iff the threat is refuted
 */
boolean d_reci_end_is_threat_refuted(slice_index si);

/* Solve at root level at the end of a reciprocal slice
 * @param restartenabled true iff option movenum is activated
 * @param si slice index
 * @return true iff >=1 solution was found
 */
boolean reci_root_end_solve(boolean restartenabled, slice_index si);

/* Continue solving at the end of a reciprocal slice
 * @param si slice index
 * @return true iff >=1 solution was found
 */
boolean reci_end_solve(slice_index si);

/* Detect starter field with the starting side if possible. 
 * @param si identifies slice
 * @param is_duplex is this for duplex?
 */
void reci_detect_starter(slice_index si, boolean is_duplex);

/* Impose the starting side on a slice.
 * @param si identifies sequence
 * @param s starting side of slice
 */
void reci_impose_starter(slice_index si, Side s);

#endif
