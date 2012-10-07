#if !defined(OPTIONS_MOVENUMBERS_RESTARTGUARD_INTELLIGENT_H)
#define OPTIONS_MOVENUMBERS_RESTARTGUARD_INTELLIGENT_H

#include "solving/solve.h"

/* This module provides functionality dealing with STRestartGuardIntelligent
 * stipulation slice type.
 * Slices of this type make solve help stipulations in intelligent mode
 */

/* Allocate a STRestartGuardIntelligent slice.
 * @return allocated slice
 */
slice_index alloc_restart_guard_intelligent(void);

/* Try to solve in n half-moves.
 * @param si slice index
 * @param n maximum number of half moves
 * @return length of solution found and written, i.e.:
 *            slack_length-2 the move just played or being played is illegal
 *            <=n length of shortest solution found
 *            n+2 no solution found
 */
stip_length_type restart_guard_intelligent_solve(slice_index si,
                                                  stip_length_type n);

/* Allocate a STIntelligentTargetCounter slice.
 * @return allocated slice
 */
slice_index alloc_intelligent_target_counter(void);

/* Try to solve in n half-moves.
 * @param si slice index
 * @param n maximum number of half moves
 * @return length of solution found and written, i.e.:
 *            slack_length-2 the move just played or being played is illegal
 *            <=n length of shortest solution found
 *            n+2 no solution found
 */
stip_length_type intelligent_target_counter_solve(slice_index si,
                                                   stip_length_type n);

#endif
