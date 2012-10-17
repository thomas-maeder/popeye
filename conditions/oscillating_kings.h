#if !defined(CONDITIONS_OSCILLATING_KINGS_H)
#define CONDITIONS_OSCILLATING_KINGS_H

/* This module implements the condition Oscillating Kings */

#include "solving/solve.h"

extern boolean OscillatingKingsTypeB[nr_sides];
extern boolean OscillatingKingsTypeC[nr_sides];
extern boolean oscillatedKs[toppile+1];

/* Try to solve in n half-moves.
 * @param si slice index
 * @param n maximum number of half moves
 * @return length of solution found and written, i.e.:
 *            slack_length-2 the move just played or being played is illegal
 *            <=n length of shortest solution found
 *            n+2 no solution found
 */
stip_length_type king_oscillator_solve(slice_index si, stip_length_type n);

/* Instrument a stipulation for Oscillating Kings
 * @param si identifies root slice of stipulation
 */
void stip_insert_king_oscillators(slice_index si);

#endif
