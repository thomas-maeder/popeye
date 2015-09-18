#if !defined(OPTIMISATIONS_INTELLIGENT_INTERCEPT_FROM_GUARD_H)
#define OPTIMISATIONS_INTELLIGENT_INTERCEPT_FROM_GUARD_H

#include "position/board.h"
#include "stipulation/stipulation.h"

/* Intercept a check by a white rider on the flight guarded by the rider
 * @param to_be_intercepted where to intercept
 * @note will leave to_be_intercepted occupied by the last piece tried
 */
void intercept_check_on_guarded_square(slice_index si, square to_be_intercepted);

/* Intercept a check with a pinned black piece
 * @param placed_on where to place the pinned black piece
 * @note will leave placed_on occupied by the last piece tried
 */
void intelligent_intercept_check_by_pin(slice_index si, square placed_on);

#endif
