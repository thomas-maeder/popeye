#if !defined(CONDITIONS_OHNESCHACH_REDUNDANT_IMMOBILILTY_TESTS_H)
#define CONDITIONS_OHNESCHACH_REDUNDANT_IMMOBILILTY_TESTS_H

/* This module provides an optimisation for the condition Ohneschach:
 * it's not necessary to test for immobility if we have already done so or if
 * immobility doesn't matter.
 */

#include "solving/solve.h"

/* Try to solve in n half-moves.
 * @param si slice index
 * @param n maximum number of half moves
 * @return length of solution found and written, i.e.:
 *            slack_length-2 the move just played or being played is illegal
 *            <=n length of shortest solution found
 *            n+2 no solution found
 */
stip_length_type ohneschach_stop_if_check_solve(slice_index si,
                                                stip_length_type n);

/* Optimise away redundant immobility tests
 * @param si identifies root slice of stipulation
 */
void ohneschach_optimise_away_redundant_immobility_tests(slice_index si);

#endif
