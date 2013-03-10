#if !defined(STIPULATION_IF_THEN_ELSE_H)
#define STIPULATION_IF_THEN_ELSE_H

/* Slice type STIfThenElse - select the sucessor based on a condition
 */

#include "stipulation/structure_traversal.h"
#include "solving/solve.h"

/* Allocate a STIfThenElse slice.
 * @param normal identifies "normal" (else) successor
 * @param exceptional identifies "exceptional" (if) successor
 * @param condition identifies condition on which to take exceptional path
 * @return index of allocated slice
 */
slice_index alloc_if_then_else_slice(slice_index normal,
                                     slice_index exceptional,
                                     slice_index condition);

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
stip_length_type if_then_else_solve(slice_index si, stip_length_type n);

/* Callback to stip_spin_off_testers
 * Spin a tester slice off a binary slice
 * @param si identifies the testing pipe slice
 * @param st address of structure representing traversal
 */
void stip_spin_off_testers_if_then_else(slice_index si,
                                        stip_structure_traversal *st);

#endif
