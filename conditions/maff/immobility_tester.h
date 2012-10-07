#if !defined(CONDITION_MAFF_IMMOBILITY_TESTER_H)
#define CONDITION_MAFF_IMMOBILITY_TESTER_H

#include "solving/solve.h"

/* This module provides functionality dealing with slices that detect
 * whether a side is immobile
 */

/* Replace immobility tester slices to cope with condition MAFF
 * @param si where to start (entry slice into stipulation)
 */
void maff_replace_immobility_testers(slice_index si);

/* Try to solve in n half-moves.
 * @param si slice index
 * @param n maximum number of half moves
 * @return length of solution found and written, i.e.:
 *            slack_length-2 the move just played or being played is illegal
 *            <=n length of shortest solution found
 *            n+2 no solution found
 */
stip_length_type maff_immobility_tester_king_solve(slice_index si, stip_length_type n);

#endif
