#if !defined(SOLVING_DISPATCH_H)
#define SOLVING_DISPATCH_H

/* Interface for dynamically dispatching operations to slices depending on their
 * type
 */

#include "stipulation/stipulation.h"

/* Perform a dispatch
 */
void dispatch(slice_index si);

#endif
