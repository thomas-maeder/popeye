#if !defined(OUTPUT_PLAINTEXT_LINE_END_OF_INTRO_SERIES_MARKER_H)
#define OUTPUT_PLAINTEXT_LINE_END_OF_INTRO_SERIES_MARKER_H

#include "solving/machinery/solve.h"
#include "solving/ply.h"

/* This module provides the STOutputPlaintextLineEndOfIntroSeriesMarker
 * slice type. Slices of this type remember the end of an intro series.
 */

/* Number of move inversions up to the current move.
 * Exposed for read-only access only
 */
extern boolean is_end_of_intro_series[maxply+1];


/* Allocate a STOutputPlaintextLineEndOfIntroSeriesMarker slice.
 * @return index of allocated slice
 */
slice_index alloc_output_plaintext_line_end_of_intro_series_marker_slice(void);

/* Try to solve in solve_nr_remaining half-moves.
 * @param si slice index
 * @note assigns solve_result the length of solution found and written, i.e.:
 *            previous_move_is_illegal the move just played is illegal
 *            this_move_is_illegal     the move being played is illegal
 *            immobility_on_next_move  the moves just played led to an
 *                                     unintended immobility on the next move
 *            <=n+1 length of shortest solution found (n+1 only if in next
 *                                     branch)
 *            n+2 no solution found in this branch
 *            n+3 no solution found in next branch
 *            (with n denominating solve_nr_remaining)
 */
void output_plaintext_line_end_of_intro_series_marker_solve(slice_index si);

#endif
