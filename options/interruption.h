#if !defined(OPTIONS_INTERRUPTION_H)
#define OPTIONS_INTERRUPTION_H

/* This module allows various options to report whether they have caused solving
 * to be interrupted. And to the output modules to report that the written
 * solution may not be complete for that reason.
 */

#include "stipulation/stipulation.h"

/* Reset our state before delegating, then be ready to report our state
 * @param si identifies the STProblemSolvingInterrupted slice
 */
void option_interruption_solve(slice_index si);

/* Remember that solving has been interrupted
 * @param si identifies the STProblemSolvingInterrupted slice
 */
void option_interruption_remember(slice_index si);

/* Report whether solving has been interrupted
 * @param si identifies the STProblemSolvingInterrupted slice
 * @return true iff solving has been interrupted
 */
boolean option_interruption_is_set(slice_index si);

#endif
