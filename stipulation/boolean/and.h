#if !defined(PYRECIPR_H)
#define PYRECIPR_H

#include "stipulation/structure_traversal.h"
#include "solving/solve.h"

/* This module provides functionality dealing logical AND stipulation slices.
 */

/* Allocate a STAnd slice.
 * @param op1 proxy to 1st operand
 * @param op2 proxy to 2nd operand
 * @return index of allocated slice
 */
slice_index alloc_and_slice(slice_index op1, slice_index op2);

/* Try to solve in n half-moves.
 * @param si slice index
 * @param n maximum number of half moves
 * @return length of solution found and written, i.e.:
 *            previous_move_is_illegal the move just played (or being played)
 *                                     is illegal
 *            immobility_on_next_move  the moves just played led to an
 *                                     unintended immobility on the next move
 *            <=n+1 length of shortest solution found (n+1 only if in next
 *                                     branch)
 *            n+2 no solution found in this branch
 *            n+3 no solution found in next branch
 */
stip_length_type and_solve(slice_index si, stip_length_type n);

/* Enable shortcut logic for all STAnd slices
 * @param si root slice of the stipulation
 */
void and_enable_shortcut_logic(slice_index si);

#endif
