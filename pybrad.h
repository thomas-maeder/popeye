#if !defined(PYBRAD_H)
#define PYBRAD_H

#include "py.h"
#include "pystip.h"

/* This module provides functionality dealing with STBranchDirect
 * stipulation slices.
 */

/* Determine whether a branch slice has a solution
 * @param si slice index
 * @param n maximal number of moves
 * @return true iff slice si has a solution
 */
boolean branch_d_has_solution_in_n(slice_index si, stip_length_type n);

/* Determine and write set play
 * @param si slice index
 * @return true iff >= 1 set play was found
 */
boolean branch_d_root_solve_setplay(slice_index si);

/* Write the key just played, then solve the post key play (threats,
 * variations) and write the refutations (if any).
 * @param refutations table containing the refutations (if any)
 * @param si slice index
 * @param type type of attack
 */
void branch_d_root_write_key_solve_postkey(int refutations,
                                           slice_index si,
                                           attack_type type);

/* Determine and write the threat and variations after the move that
 * has just been played in the current ply. 
 * @param si slice index
 */
void branch_d_solve_variations(slice_index si);

/* Determine and write the continuations in the current position
 * (i.e. attacker's moves winning after a defender's move that refuted
 * the threat).
 * This is an indirectly recursive function.
 * @param continuations table where to store continuing moves (i.e. threats)
 * @param si slice index
 * @param n maximal number of moves
 */
void branch_d_solve_continuations_in_n(int continuations,
                                       slice_index si,
                                       stip_length_type n);

/* Has the threat just played been refuted by the preceding defense?
 * @param si identifies stipulation slice
 * @param n maximal number of moves
 * @return true iff the threat is refuted
 */
boolean branch_d_is_threat_in_n_refuted(slice_index si, stip_length_type n);

/* Solve a branch slice at root level.
 * @param si slice index
 */
void branch_d_root_solve(slice_index si);

/* Impose the starting side on a slice.
 * @param si identifies branch
 * @param s starting side of slice
 */
void branch_d_impose_starter(slice_index si, Side s);

#endif
