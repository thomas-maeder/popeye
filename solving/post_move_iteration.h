#if !defined(SOLVING_POST_MOVE_ITERATION_H)
#define SOLVING_POST_MOVE_ITERATION_H

#include "solving/solve.h"

/* This module provides functionality for coordinating "post move iterations"
 * (i.e. promotions, Supercirce rebirths etc.).
 */

/* a unique number for each post move iteration (e.g. promotion,
 * rebirth square (Super Circe), king position (Republican Chess)
 */
typedef unsigned int post_move_iteration_id_type;
extern post_move_iteration_id_type post_move_iteration_id[maxply+1];

/* true iff a component has advanced its iteration; set to prevent outer
 * components from advancing theirs as well
 */
extern boolean post_move_iteration_locked[maxply+1];

/* Lock post move iterations in the current move retraction
 */
void lock_post_move_iterations(void);

/* Try to solve in n half-moves.
 * @param si slice index
 * @param n maximum number of half moves
 * @return length of solution found and written, i.e.:
 *            slack_length-2 the move just played or being played is illegal
 *            <=n length of shortest solution found
 *            n+2 no solution found
 */
stip_length_type post_move_iteration_initialiser_solve(slice_index si,
                                                        stip_length_type n);

/* Instrument slices with post move iteration slices
 */
void stip_insert_post_move_iteration(slice_index si);

#endif
