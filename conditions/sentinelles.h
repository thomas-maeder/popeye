#if !defined(CONDITION_SENTINELLES_H)
#define CONDITION_SENTINELLES_H

#include "solving/solve.h"

/* This module implements Echecs Sentinelles.
 */

extern unsigned int sentinelles_max_nr_pawns[nr_sides];
extern unsigned int sentinelles_max_nr_pawns_total;

/* Try to solve in n half-moves.
 * @param si slice index
 * @param n maximum number of half moves
 * @return length of solution found and written, i.e.:
 *            slack_length-2 the move just played or being played is illegal
 *            <=n length of shortest solution found
 *            n+2 no solution found
 */
stip_length_type sentinelles_inserter_solve(slice_index si, stip_length_type n);

/* Instrument a stipulation
 * @param si identifies root slice of stipulation
 */
void stip_insert_sentinelles_inserters(slice_index si);

#endif
