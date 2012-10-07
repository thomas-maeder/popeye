#if !defined(STIPULATION_AMU_MATE_FILTER_H)
#define STIPULATION_AMU_MATE_FILTER_H

#include "solving/solve.h"

/* This module provides slice type STAmuMateFilter
 */

/* Try to solve in n half-moves.
 * @param si slice index
 * @param n maximum number of half moves
 * @return length of solution found and written, i.e.:
 *            slack_length-2 the move just played or being played is illegal
 *            <=n length of shortest solution found
 *            n+2 no solution found
 */
stip_length_type amu_mate_filter_solve(slice_index si, stip_length_type n);

/* Instrument a stipulation with AMU mate filter slices
 * @param si root of branch to be instrumented
 */
void stip_insert_amu_mate_filters(slice_index si);

#endif
