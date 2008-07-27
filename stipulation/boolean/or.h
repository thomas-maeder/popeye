#if !defined(PYQUODLI_H)
#define PYQUODLI_H

#include "py.h"
#include "boolean.h"

/* This module provides functionality dealing with quodlibet
 * (i.e. logical OR) stipulation slices.
 */


/* Determine and write continuations at end of quodlibet slice
 * @param attacker attacking side
 * @param t table where to store continuing moves (i.e. threats)
 * @param si index of quodlibet slice
 */
void d_quodlibet_end_solve_continuations(couleur attacker,
                                         int t,
                                         slice_index si);

/* Find and write defender's set play
 * @param defender defending side
 * @param si slice index
 */
void d_quodlibet_end_solve_setplay(couleur defender, slice_index si);

/* Find and write defender's set play in self/reflex play if every
 * set move leads to end
 * @param defender defending side
 * @param si slice index
 * @return true iff every defender's move leads to end
 */
boolean d_quodlibet_end_solve_complete_set(couleur defender, slice_index si);

/* Determine and write solutions starting at the end of a quodlibet
 * direct/self/reflex stipulation. 
 * @param attacker attacking side
 * @param restartenabled true iff the written solution should only
 *                       start at the Nth legal move of attacker
 *                       (determined by user input)
 * @param si slice index
 */
void d_quodlibet_end_solve(couleur attacker,
                           boolean restartenabled,
                           slice_index si);

/* Write the key just played, then solve the post key play (threats,
 * variations) and write the refutations (if any), starting at the end
 * of a quodlibet slice.
 * @param attacker attacking side (has just played key)
 * @param refutations table containing the refutations (if any)
 * @param si slice index
 * @param is_try true iff what we are solving is a try
 */
void d_quodlibet_end_write_key_solve_postkey(couleur attacker,
                                             int refutations,
                                             slice_index si,
                                             boolean is_try);

/* Determine whether the attacker wins at the end of a quodlibet slice
 * @param attacker attacking side (at move)
 * @param si slice index of leaf slice
 * @param parent_is_exact true iff parent of slice si has exact length
 * @return true iff attacker wins
 */
boolean d_quodlibet_end_does_attacker_win(couleur attacker, slice_index si);

/* Find and write variations from the end of a quodlibet slice.
 * @param defender attacking side
 * @param leaf slice index
 */
void d_quodlibet_end_solve_variations(couleur attacker, slice_index si);

/* Determine whether the defending side wins at the end of quodlibet
 * in direct play. 
 * @param defender defending side
 * @param si slice identifier
 */
boolean d_quodlibet_end_does_defender_win(couleur defender, slice_index si);

#endif
