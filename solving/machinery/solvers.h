#if !defined(SOLVING_SOLVERS_H)
#define SOLVING_SOLVERS_H

#include "stipulation/stipulation.h"

/* Instrument the slices representing the stipulation with solving slices
 * @param stipulation_root_hook proxy slice into stipulation
 * @return a copy of the stipulation instrumented with solvers
 */
slice_index build_solvers(slice_index stipulation_root_hook);

#endif
