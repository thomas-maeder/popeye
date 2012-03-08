#if !defined(OPTIMISATION_INTELLIGENT_MATE_GOALREACHABLE_GUARD_H)
#define OPTIMISATION_INTELLIGENT_MATE_GOALREACHABLE_GUARD_H

#include "py.h"

/* Try to solve in n half-moves after a defense.
 * @param si slice index
 * @param n maximum number of half moves until end state has to be reached
 * @return length of solution found and written, i.e.:
 *            slack_length-2 defense has turned out to be illegal
 *            <=n length of shortest solution found
 *            n+2 no solution found
 */
stip_length_type goalreachable_guard_mate_attack(slice_index si,
                                                 stip_length_type n);

#endif
