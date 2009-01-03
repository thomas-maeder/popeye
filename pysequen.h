#if !defined(PYSEQUEN_H)
#define PYSEQUEN_H

#include "py.h"
#include "pystip.h"
#include "boolean.h"

/* This module provides functionality dealing with sequence
 * (i.e. simply concatenated) stipulation slices.
 */

/* Detect a priori unsolvability of a slice (e.g. because of forced
 * reflex mates)
 * @param leaf leaf's slice index
 * @return true iff leaf is a priori unsolvable
 */
boolean sequence_end_is_unsolvable(slice_index leaf);

/* Write a priori unsolvability (if any) of a slice (e.g. forced
 * reflex mates).
 * Assumes slice_is_unsolvable(si)
 * @param si slice index
 */
void sequence_write_unsolvability(slice_index si);

/* Determine and write continuations at end of sequence slice
 * @param continuations table where to store continuing moves
 *                      (e.g. threats)
 * @param si index of sequence slice
 */
void sequence_end_solve_continuations(int continuations, slice_index si);

/* Find and write set play
 * @param si slice index
 */
void sequence_root_end_solve_setplay(slice_index si);

/* Find and write set play provided every set move leads to end
 * @param si slice index
 * @return true iff every defender's move leads to end
 */
boolean sequence_root_end_solve_complete_set(slice_index si);

/* Write the key just played, then continue solving at end of sequence
 * slice to find and write the post key play (threats, variations)
 * @param si slice index
 * @param type type of attack
 */
void d_sequence_root_end_write_key_solve_postkey(slice_index si,
                                                 attack_type type);

/* Solve at root level at the end of a sequence slice
 * @param restartenabled true iff option movenum is activated
 * @param si slice index
 * @return true iff >=1 solution was found
 */
boolean sequence_root_end_solve(boolean restartenabled, slice_index si);

/* Continue solving at the end of a sequence slice
 * @param si slice index
 * @return true iff >=1 solution was found
 */
boolean sequence_end_solve(slice_index si);

/* Find and write variations starting at end of sequence slice
 * @param si slice index
 */
void d_sequence_end_solve_variations(slice_index si);

/* Determine whether there is a solution at the end of a quodlibet
 * slice. 
 * @param si slice index
 * @return true iff slice si has a solution
 */
boolean sequence_end_has_solution(slice_index si);

/* Determine whether a sequence slice.has just been solved with the
 * just played move by the non-starter
 * @param si slice identifier
 * @return true iff the non-starting side has just solved
 */
boolean sequence_end_has_non_starter_solved(slice_index si);

/* Determine whether the non-starter has refuted with his move just
 * played independently of the starter's possible play during the
 * current slice.
 * Example: in direct play, the defender has just captured that last
 * piece that could deliver mate.
 * @param si slice identifier
 * @return true iff the non-starter has refuted
 */
boolean sequence_end_has_non_starter_refuted(slice_index si);

/* Determine whether the attacker has immediately lost in direct play
 * with his move just played.
 * @param si slice identifier
 * @return true iff the attacking side has directly lost
 */
boolean d_sequence_end_has_attacker_lost(slice_index si);

/* Determine whether the attacker has immediately won in direct play
 * with his move just played.
 * @param si slice identifier
 * @return true iff the defending side has directly won
 */
boolean d_sequence_end_has_attacker_won(slice_index si);

/* Has the threat just played been defended by the preceding defense?
 * @param si identifies stipulation slice
 * @return true iff the threat is refuted
 */
boolean d_sequence_end_is_threat_refuted(slice_index si);

/* Detect starter field with the starting side if possible. 
 * @param si identifies slice
 * @param is_duplex is this for duplex?
 */
void sequence_detect_starter(slice_index si, boolean is_duplex);

/* Impose the starting side on a slice.
 * @param si identifies sequence
 * @param s starting side of slice
 */
void sequence_impose_starter(slice_index si, Side s);

#endif
