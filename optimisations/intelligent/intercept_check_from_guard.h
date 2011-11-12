#if !defined(OPTIMISATIONS_INTELLIGENT_INTERCEPT_FROM_GUARD_H)
#define OPTIMISATIONS_INTELLIGENT_INTERCEPT_FROM_GUARD_H

#include "py.h"
#include "pyboard.h"

/* Intercept a check by a white rider on the flight guarded by the rider
 * @param index_of_guarding_rider identifies the rider
 * @param to_be_intercepted where to intercept
 */
void intercept_check_on_guarded_square(unsigned int index_of_guarding_rider,
                                       square to_be_intercepted);

/* Intercept an orthogonal check with a pinned black piece
 * @param placed_on where to place the pinned black piece
 */
void intelligent_intercept_orthogonal_check_by_pin(square placed_on);

#endif
