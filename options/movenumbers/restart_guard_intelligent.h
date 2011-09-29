#if !defined(OPTIONS_MOVENUMBERS_RESTARTGUARD_INTELLIGENT_H)
#define OPTIONS_MOVENUMBERS_RESTARTGUARD_INTELLIGENT_H

#include "stipulation/help_play/play.h"

/* This module provides functionality dealing with STRestartGuardIntelligent
 * stipulation slice type.
 * Slices of this type make solve help stipulations in intelligent mode
 */

extern unsigned long nr_potential_target_positions;

/* Allocate a STRestartGuardIntelligent slice.
 * @return allocated slice
 */
slice_index alloc_restart_guard_intelligent(void);

/* Determine and write the solution(s) in a help stipulation
 * @param si slice index of slice being solved
 * @param n exact number of half moves until end state has to be reached
 * @return length of solution found, i.e.:
 *         n+4 the move leading to the current position has turned out
 *             to be illegal
 *         n+2 no solution found
 *         n   solution found
 */
stip_length_type restart_guard_intelligent_help(slice_index si, stip_length_type n);

#endif
