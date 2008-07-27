#if !defined(PYSEQUEN_H)
#define PYSEQUEN_H

#include "py.h"
#include "pyhash.h"
#include "boolean.h"

/* This module provides functionality dealing with sequence
 * (i.e. simply concatenated) stipulation slices.
 */

/* Determine and write continuations at end of sequence slice
 * @param attacker attacking side
 * @param table table where to store continuing moves (i.e. threats)
 * @param si index of sequence slice
 */
void d_sequence_end_solve_continuations(couleur attacker,
                                        int table,
                                        slice_index si);

/* Find and write defender's set play
 * @param defender defending side
 * @param leaf slice index
 */
void d_sequence_end_solve_setplay(couleur defender, slice_index si);

/* Find and write defender's set play in self/reflex play if every
 * set move leads to end
 * @param defender defending side
 * @param leaf slice index
 * @return true iff every defender's move leads to end
 */
boolean d_sequence_end_solve_complete_set(couleur defender, slice_index si);

/* Continue solving at the end of a sequence slice
 * Unsolvability (e.g. because of a forced reflex move) has already
 * been dealt with.
 * @param attacker attacking side
 * @param restartenabled true iff the written solution should only
 *                       start at the Nth legal move of attacker
 *                       (determined by user input)
 * @param leaf slice index 
 */
void d_sequence_end_solve(couleur attacker,
                          boolean restartenabled,
                          slice_index si);

/* Write the key just played, then continue solving at end of sequence
 * slice to find and write the post key play (threats, variations) and
 * write the refutations (if any)
 * @param attacker attacking side (has just played key)
 * @param refutations table containing the refutations (if any)
 * @param leaf slice index
 * @param is_try true iff what we are solving is a try
 */
void d_sequence_end_write_key_solve_postkey(couleur attacker,
                                            int refutations,
                                            slice_index si,
                                            boolean is_try);

/* Continue solving help play at the end of a sequence slice
 * @param side_at_move side at the move
 * @param no_succ_hash_category hash category for storing failures
 * @param restartenabled true iff option movenum is activated
 * @param si slice index
 * @return true iff >=1 solution was found
 */
boolean h_sequence_end_solve(couleur side_at_move,
                             hashwhat no_succ_hash_category,
                             boolean restartenabled,
                             slice_index si);

/* Continue solving series play at the end of a sequence slice
 * @param side_at_move side at the move
 * @param no_succ_hash_category hash category for storing failures
 * @param restartenabled true iff option movenum is activated
 * @param si slice index
 * @return true iff >=1 solution was found
 */
boolean ser_sequence_end_solve(couleur series_side,
                               boolean restartenabled,
                               slice_index si);

/* Determine whether the attacking side wins at the end of a sequence slice
 * @param attacker attacking side
 * @param si slice identifier
 * @return true iff attacker wins
 */
boolean d_sequence_end_does_attacker_win(couleur attacker, slice_index si);

/* Find and write variations starting at end of sequence slice
 * @param defender attacking side
 * @param leaf slice index
 */
void d_sequence_end_solve_variations(couleur attacker, slice_index si);

/* Determine whether the defending side wins at the end of a sequence slice
 * @param defender defending side
 * @param si slice identifier
 */
boolean d_sequence_end_does_defender_win(couleur defender, slice_index si);

#endif
