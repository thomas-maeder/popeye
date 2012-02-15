#if !defined(STIPULATION_AMU_MATE_FILTER_H)
#define STIPULATION_AMU_MATE_FILTER_H

#include "pyslice.h"

/* This module provides slice type STAmuMateFilter
 */

/* Solve a slice
 * @param si slice index
 * @return whether there is a solution and (to some extent) why not
 */
has_solution_type amu_mate_filter_solve(slice_index si);

/* Instrument a stipulation with AMU mate filter slices
 * @param si root of branch to be instrumented
 */
void stip_insert_amu_mate_filters(slice_index si);

#endif
