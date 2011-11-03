#if !defined(OPTIMISATION_INTELLIGENT_STALEMATE_IMMOBILISE_BLACK_H)
#define OPTIMISATION_INTELLIGENT_STALEMATE_IMMOBILISE_BLACK_H

#include "py.h"

/* @return true iff >=1 black pieces needed to be immobilised
 */
boolean intelligent_stalemate_immobilise_black(void);

/* Determine whether there is a solution in n half moves.
 * @param si slice index of slice being solved
 * @param n number of half moves until end state has to be reached
 * @return length of solution found, i.e.:
 *         n+2 the move leading to the current position has turned out
 *             to be illegal
 *         n+1 no solution found
 *         n   solution found
 */
stip_length_type intelligent_immobilisation_counter_can_help(slice_index si,
                                                             stip_length_type n);

#endif
