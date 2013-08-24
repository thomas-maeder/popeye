#if !defined(OPTIMISATION_INTELLIGENT_MATE_INTERCEPT_GUARD_BY_WHITE_H)
#define OPTIMISATION_INTELLIGENT_MATE_INTERCEPT_GUARD_BY_WHITE_H

#include "position/position.h"

/* Intercept a guard by white
 * @param target guard of what square
 * @param dir_from_rider direction from guarding rider
 * @param go_on what to do after each successful interception?
 */
void intelligent_intercept_guard_by_white(square target,
                                          int dir_from_rider,
                                          void (*go_on)(void));

#endif
