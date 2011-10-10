#if !defined(OPTIMISATIONS_INTELLIGENT_STALEMATE_FILTER_H)
#define OPTIMISATIONS_INTELLIGENT_STALEMATE_FILTER_H

#include "stipulation/help_play/play.h"

/* This module provides functionality dealing with STIntelligentStalemateFilter
 * stipulation slice type.
 * Slices of this type make solve help stipulations in intelligent mode
 */

/* Allocate a STIntelligentStalemateFilter slice.
 * @return allocated slice
 */
slice_index alloc_intelligent_stalemate_filter(void);

/* Impose the starting side on a stipulation.
 * @param si identifies slice
 * @param st address of structure that holds the state of the traversal
 */
void impose_starter_intelligent_stalemate_filter(slice_index si,
                                                 stip_structure_traversal *st);

/* Determine and write the solution(s) in a help stipulation
 * @param si slice index of slice being solved
 * @param n exact number of half moves until end state has to be reached
 * @return length of solution found, i.e.:
 *         n+4 the move leading to the current position has turned out
 *             to be illegal
 *         n+2 no solution found
 *         n   solution found
 */
stip_length_type intelligent_stalemate_filter_help(slice_index si,
                                                   stip_length_type n);

#endif
