#if !defined(PYCOMPOS_H)
#define PYCOMPOS_H

#include "py.h"
#include "pystip.h"
#include "pyoutput.h"

/* This module provides functionality dealing with composite
 * (i.e. non-leaf) stipulation slices.
 */

/* Determine whether a composite slice has a solution
 * @param si slice index
 * @return true iff slice si has a solution
 */
boolean composite_has_solution(slice_index si);

/* Determine and write set play of a direct/self/reflex stipulation
 * @param si slice index
 */
void composite_root_solve_setplay(slice_index si);

/* Write the key just played, then solve the post key play (threats,
 * variations) and write the refutations (if any).
 * @param refutations table containing the refutations (if any)
 * @param si slice index
 * @param type type of attack
 */
void d_composite_root_write_key_solve_postkey(int refutations,
                                              slice_index si,
                                              attack_type type);

/* Determine and write the threat and variations in direct/self/reflex
 * play after the move that has just been played in the current ply.
 * We have already determined that this move doesn't have more
 * refutations than allowed.
 * This is an indirectly recursive function.
 * @param n number of moves until end state has to be reached,
 *          including the move just played
 * @param len_threat length of threats
 * @param threats table containing threats
 */
void d_composite_solve_variations(stip_length_type n,
                                  int len_threat,
                                  int threats,
                                  slice_index si);

/* Determine and write only the post-key solution in the current
 * position in direct/self/reflex play.
 * @param n number of moves until end state has to be reached
 * @param si slice index
 */
void d_composite_root_solve_postkeyonly(stip_length_type n, slice_index si);

/* Determine and write the continuations in the current position
 * (i.e. attacker's moves winning after a defender's move that refuted
 * the threat).
 * This is an indirectly recursive function.
 * @param continuations table where to store continuing moves (i.e. threats)
 * @param si slice index
 */
void composite_solve_continuations(int continuations, slice_index si);

/* Has the threat just played been refuted by the preceding defense?
 * @param si identifies stipulation slice
 * @return true iff the threat is refuted
 */
boolean d_composite_is_threat_refuted(slice_index si);

/* Solve a composite slice with series play at root level
 * @param restartenabled true iff option movenum is active
 * @param si slice index
 * @param n number of moves until the slice's goal has to be reached
 *          (this may be shorter than the slice's length if we are
 *          searching for short solutions only)
 * @return true iff >= 1 solution was found
 */
boolean ser_composite_root_exact_solve(boolean restartenabled,
                                       slice_index si,
                                       stip_length_type n);

/* Determine and write the solutions in the current position in series
 * play.
 * @param si slice index
 * @param n number of moves until the slice's goal has to be reached
 *          (this may be shorter than the slice's length if we are
 *          searching for short solutions only)
 * @return true iff >= 1 solution was found
 */
boolean ser_composite_exact_solve(slice_index si, stip_length_type n);

/* Solve the root composite slice at root level.
 * @param restartenabled true iff option movenum is active
 * @param si slice index
 * @param n number of moves until the slice's goal has to be reached
 * @return true iff >= 1 solution was found
 */
boolean composite_root_solve(boolean restartenabled,
                             slice_index si,
                             stip_length_type n);

/* Solve a composite slice at root level
 * @param restartenabled true iff option movenum is active
 * @param si slice index
 * @param n exact number of moves until the slice's goal has to be
 *          reached (this may be shorter than the slice's length if
 *          we are searching for short solutions only)
 * @return true iff >= 1 solution was found
 */
boolean composite_root_exact_solve(boolean restartenabled,
                                   slice_index si,
                                   stip_length_type n);

/* Determine and write the solutions in the current position.
 * @param si slice index
 * @return true iff >= 1 solution was found
 */
boolean composite_solve(slice_index si);

#endif
