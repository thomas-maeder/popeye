#if !defined(OUTPUT_PLAINTEXT_LINE_END_OF_INTRO_SERIES_MARKER_H)
#define OUTPUT_PLAINTEXT_LINE_END_OF_INTRO_SERIES_MARKER_H

#include "pyslice.h"

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

/* Determine whether a slice has just been solved with the move
 * by the non-starter
 * @param si slice identifier
 * @return whether there is a solution and (to some extent) why not
 */
has_solution_type
output_plaintext_line_end_of_intro_series_marker_has_solution(slice_index si);

/* Solve a slice
 * @param si slice index
 * @return whether there is a solution and (to some extent) why not
 */
has_solution_type
output_plaintext_line_end_of_intro_series_marker_solve(slice_index si);

#endif
