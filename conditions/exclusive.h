#if !defined(PYEXCLUS_H)
#define PYEXCLUS_H

/* This module implements Exclusive Chess
 */

#include "boolean.h"
#include "pyposit.h"
#include "pystip.h"

/* Perform the necessary verification steps for solving an Exclusive
 * Chess problem
 * @param si identifies root slice of stipulation
 * @return true iff verification passed
 */
boolean exclusive_verifie_position(slice_index si);

/* Do preparations before generating moves for a side in an Exclusive
 * Chess problem
 * @param side side for which to generate moves
 */
void exclusive_init_genmove(Side side);

/* Determine whether the current position is illegal because of
 * Exclusive Chess rules
 * @return true iff the position is legal
 */
boolean exclusive_pos_legal(void);

/* When counting mating moves, it is not necessary to detect self-check in moves
 * that don't deliver mate; remove the slices that would detect these
 * self-checks
 * @param si identifies slice where to start
 */
void optimise_away_unnecessary_selfcheckguards(slice_index si);

/* Determine whether there is a solution in n half moves.
 * @param si slice index of slice being solved
 * @param n exact number of half moves until end state has to be reached
 * @return length of solution found, i.e.:
 *         n+4 the move leading to the current position has turned out
 *             to be illegal
 *         n+2 no solution found
 *         n   solution found
 */
stip_length_type exclusive_chess_unsuspender_can_help(slice_index si,
                                                      stip_length_type n);

#endif
