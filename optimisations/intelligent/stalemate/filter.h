#if !defined(OPTIMISATIONS_INTELLIGENT_STALEMATE_FILTER_H)
#define OPTIMISATIONS_INTELLIGENT_STALEMATE_FILTER_H

#include "stipulation/structure_traversal.h"
#include "solving/solve.h"

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

/* Try to solve in n half-moves.
 * @param si slice index
 * @param n maximum number of half moves
 * @return length of solution found and written, i.e.:
 *            previous_move_is_illegal the move just played is illegal
 *            this_move_is_illegal     the move being played is illegal
 *            immobility_on_next_move  the moves just played led to an
 *                                     unintended immobility on the next move
 *            <=n+1 length of shortest solution found (n+1 only if in next
 *                                     branch)
 *            n+2 no solution found in this branch
 *            n+3 no solution found in next branch
 */
stip_length_type intelligent_stalemate_filter_solve(slice_index si,
                                                     stip_length_type n);

#endif
