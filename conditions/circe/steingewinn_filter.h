#if !defined(STIPULATION_CIRCE_STEINGEWINN_FILTER_H)
#define STIPULATION_CIRCE_STEINGEWINN_FILTER_H

#include "pyslice.h"

/* This module provides slice type STCirceSteingewinnFilter - detects whether a
 *  capture isn't a Steingewinn because the captured piece is reborn
 */

/* Determine whether a slice has just been solved with the move
 * by the non-starter
 * @param si slice identifier
 * @return whether there is a solution and (to some extent) why not
 */
has_solution_type circe_steingewinn_filter_has_solution(slice_index si);

/* Solve a slice
 * @param si slice index
 * @return whether there is a solution and (to some extent) why not
 */
has_solution_type circe_steingewinn_filter_solve(slice_index si);

/* Instrument a stipulation with AMU mate filter slices
 * @param si root of branch to be instrumented
 */
void stip_insert_circe_steingewinn_filters(slice_index si);

#endif
