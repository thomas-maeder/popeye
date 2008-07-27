#if !defined(PYCOMPOS_H)
#define PYCOMPOS_H

#include "py.h"
#include "pystip.h"

/* This module provides functionality dealing with composite
 * (i.e. non-leaf) stipulation slices.
 */

/* Determine and write set play of a direct/self/reflex stipulation
 * @param attacker attacking side
 * @param n number of moves until end state has to be reached,
 *          including the virtual key move
 * @param si slice index
 */
void d_composite_solve_setplay(couleur attacker, int n, slice_index si);

/* Determine and write the solutions and tries in the current position
 * in direct play.
 * @param attacker attacking side
 * @param n number of moves until end state has to be reached
 * @param restartenabled true iff the written solution should only
 *                       start at the Nth legal move of attacker
 *                       (determined by user input)
 * @param si slice index
 */
void d_composite_solve(couleur attacker,
                       int n,
                       boolean restartenabled,
                       slice_index si);

/* Determine and write the post-key solution in the current position
 * in direct/self/reflex play.
 * @param attacker attacking side
 * @param n number of moves until end state has to be reached
 * @param si slice index
 */
void d_composite_solve_postkey(couleur attacker, int n, slice_index si);

/* Determine and write the solutions in the current position in series
 * play.
 * @param attacker attacking side
 * @param n number of moves until end state has to be reached
 * @param restartenabled true iff the written solution should only
 *                       start at the Nth legal move of attacker
 *                       (determined by user input)
 * @param si slice index
 */
boolean ser_composite_solve(couleur camp,
                            int n,
                            boolean restartenabled,
                            slice_index si);

/* Determine and write the solutions in the current position in help
 * play.
 * @param attacker attacking side
 * @param n number of moves until end state has to be reached
 * @param restartenabled true iff the written solution should only
 *                       start at the Nth legal move of attacker
 *                       (determined by user input)
 * @param si slice index
 */
boolean h_composite_solve(couleur camp,
                          int n,
                          boolean restartenabled,
                          slice_index si);

#endif
