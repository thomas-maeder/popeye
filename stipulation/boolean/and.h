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

/* Determine whether the defending side wins at the end of reciprocal
 * in direct play. 
 * @param defender defending side
 * @param si slice identifier
 * @return "how much or few" the defending side wins
 */
d_defender_win_type d_reci_end_does_defender_win(slice_index si);

/* Determine whether the attacker wins at the end of a reciprocal slice
 * @param si slice index of leaf slice
 * @return true iff attacker wins
 */
boolean d_reci_end_does_attacker_win(slice_index si);

/* Determine whether the defender has immediately lost in direct play
 * with his move just played.
 * @param si slice identifier
 * @return true iff the defending side has directly lost
 */
boolean d_reci_end_has_defender_lost(slice_index si);

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

/* Write a priori unsolvability (if any) of a slice in direct play
 * (e.g. forced reflex mates).
 * Assumes slice_is_unsolvable(si)
 * @param si slice index
 */
void d_reci_write_unsolvability(slice_index si);

/* Find and write variations from the end of a reciprocal slice.
 * @param len_threat length of threat (shorter variations are suppressed) 
 * @param threats table containing threats (variations not defending
 *                against all threats are suppressed)
 * @param refutations table containing refutations (written at end)
 * @param si slice index
 */
void d_reci_end_solve_variations(int len_threat,
                                 int threats,
                                 int refutations,
                                 slice_index si);

/* Determine and write continuations at end of reciprocal slice
 * @param continuations table where to store continuing moves
 *                      (e.g. threats)
 * @param si index of quodlibet slice
 */
void d_reci_end_solve_continuations(int continuations, slice_index si);

/* Find and write defender's set play
 * @param si slice index
 */
void d_reci_end_solve_setplay(slice_index si);

/* Determine and write solutions starting at the end of a reciprocal
 * @param restartenabled true iff the written solution should only
 *                       start at the Nth legal move of attacker
 *                       (determined by user input)
 * @param si slice index
 */
void d_reci_end_solve(boolean restartenabled, slice_index si);

/* Write the key just played, then solve the post key play (threats,
 * variations) and write the refutations (if any), starting at the end
 * of a reciprocal slice.
 * @param refutations table containing the refutations (if any)
 * @param si slice index
 * @param is_try true iff what we are solving is a try
 */
void d_reci_end_write_key_solve_postkey(int refutations,
                                        slice_index si,
                                        boolean is_try);

/* Continue solving at the end of a reciprocal slice
 * @param si slice index
 * @return true iff >=1 solution was found
 */
boolean h_reci_end_solve(slice_index si);

/* Intialize starter field with the starting side if possible, and
 * no_side otherwise. 
 * @param si identifies slice
 * @param is_duplex is this for duplex?
 */
void reci_init_starter(slice_index si, boolean is_duplex);

#endif
