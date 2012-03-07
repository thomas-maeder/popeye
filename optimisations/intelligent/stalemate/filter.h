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

/* Try to solve in n half-moves after a defense.
 * @param si slice index
 * @param n maximum number of half moves until end state has to be reached
 * @return length of solution found and written, i.e.:
 *            slack_length-2 defense has turned out to be illegal
 *            <=n length of shortest solution found
 *            n+2 no solution found
 */
stip_length_type intelligent_stalemate_filter_help(slice_index si,
                                                   stip_length_type n);

#endif
