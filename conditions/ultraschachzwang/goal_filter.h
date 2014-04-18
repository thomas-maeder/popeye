#if !defined(STIPULATION_ULTRASCHACHZWANG_GOAL_FILTER_H)
#define STIPULATION_ULTRASCHACHZWANG_GOAL_FILTER_H

#include "solving/machinery/solve.h"

/* This module provides slice type STUltraschachzwangGoalFilter. This slice
 * suspends Ultraschachzwang when testing for mate.
 */

/* Instrument the solving machinery with Ultraschachzwang
 * @param si root of branch to be instrumented
 */
void ultraschachzwang_initialise_solving(slice_index si);

#endif
