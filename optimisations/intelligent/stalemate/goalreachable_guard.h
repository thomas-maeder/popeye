#if !defined(OPTIMISATION_INTELLIGENT_STALEMATE_GOALREACHABLE_GUARD_H)
#define OPTIMISATION_INTELLIGENT_STALEMATE_GOALREACHABLE_GUARD_H

#include "py.h"

/* Solve in a number of half-moves
 * @param si slice index
 * @param n maximum number of half moves until end state has to be reached
 * @return length of solution found and written, i.e.:
 *            slack_length-2 the move leading to the current position has
 *                           turned out to be illegal
 *            n   solution found
 *            n+2 no solution found
 */
stip_length_type goalreachable_guard_stalemate_help(slice_index si,
                                                    stip_length_type n);

#endif
