#if !defined(OPTIMISATION_INTELLIGENT_MATE_INTERCEPT_CHECK_BY_WHITE_H)
#define OPTIMISATION_INTELLIGENT_MATE_INTERCEPT_CHECK_BY_WHITE_H

#include "py.h"

/* Intercept a check to the black king
 * @param dir_from_rider direction from rider giving check to black king
 * @param go_on what to do after each successful interception?
 */
void intelligent_intercept_check_by_white(int dir_from_rider,
                                          void (*go_on)(void));

#endif
