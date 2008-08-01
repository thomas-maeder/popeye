#if !defined(PYSEQUEN_H)
#define PYSEQUEN_H

#include "py.h"
#include "pyhash.h"
#include "pycompos.h"
#include "boolean.h"

/* This module provides functionality dealing with sequence
 * (i.e. simply concatenated) stipulation slices.
 */

/* Determine and write continuations at end of sequence slice
 * @param continuations table where to store continuing moves
 *                      (e.g. threats)
 * @param si index of sequence slice
 */
void d_sequence_end_solve_continuations(int continuations, slice_index si);

/* Find and write defender's set play
 * @param si slice index
 */
void d_sequence_end_solve_setplay(slice_index si);

/* Find and write defender's set play in self/reflex play if every
 * set move leads to end
 * @param si slice index
 * @return true iff every defender's move leads to end
 */
boolean d_sequence_end_solve_complete_set(slice_index si);

/* Continue solving at the end of a sequence slice
 * Unsolvability (e.g. because of a forced reflex move) has already
 * been dealt with.
 * @param restartenabled true iff the written solution should only
 *                       start at the Nth legal move of attacker
 *                       (determined by user input)
 * @param si slice index 
 */
void d_sequence_end_solve(boolean restartenabled, slice_index si);

/* Write the key just played, then continue solving at end of sequence
 * slice to find and write the post key play (threats, variations) and
 * write the refutations (if any)
 * @param refutations table containing the refutations (if any)
 * @param si slice index
 * @param is_try true iff what we are solving is a try
 */
void d_sequence_end_write_key_solve_postkey(int refutations,
                                            slice_index si,
                                            boolean is_try);

/* Continue solving help play at the end of a sequence slice
 * @param no_succ_hash_category hash category for storing failures
 * @param restartenabled true iff option movenum is activated
 * @param si slice index
 * @return true iff >=1 solution was found
 */
boolean h_sequence_end_solve(hashwhat no_succ_hash_category,
                             boolean restartenabled,
                             slice_index si);

/* Continue solving series play at the end of a sequence slice
 * @param restartenabled true iff option movenum is activated
 * @param si slice index
 * @return true iff >=1 solution was found
 */
boolean ser_sequence_end_solve(boolean restartenabled, slice_index si);

/* Find and write variations starting at end of sequence slice
 * @param attacker attacking side
 * @param len_threat length of threat (shorter variations are suppressed)
 * @param threats table containing threats (variations not defending
 *                against all threats are suppressed)
 * @param refutations table containing refutations (written at end)
 * @param si slice index
 */
void d_sequence_end_solve_variations(int len_threat,
                                     int threats,
                                     int refutations,
                                     slice_index si);

/* Determine whether the attacking side wins at the end of a sequence slice
 * @param si slice identifier
 * @return true iff attacker wins
 */
boolean d_sequence_end_does_attacker_win(slice_index si);

/* Determine whether the defending side wins at the end of a sequence slice
 * @param si slice identifier
 * @return "how much or few" the defending side wins
 */
d_composite_win_type d_sequence_end_does_defender_win(slice_index si);

/* Determine whether the defender has directly lost in direct play
 * with his move just played.
 * Assumes that there is no short win for the defending side.
 * @param si slice identifier
 * @return true iff the defending side has directly lost
 */
boolean d_sequence_end_has_defender_lost(slice_index si);

/* Intialize starter field with the starting side if possible, and
 * no_side otherwise. 
 * @param si identifies slice
 * @param is_duplex is this for duplex?
 */
void sequence_init_starter(slice_index si, boolean is_duplex);

#endif
