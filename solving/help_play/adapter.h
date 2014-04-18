#if !defined(SOLVING_HELP_PLAY_ADAPTER_H)
#define SOLVING_HELP_PLAY_ADAPTER_H

#include "solving/machinery/solve.h"
#include "stipulation/structure_traversal.h"

/* This module provides functionality dealing with STHelpAdapter
 * stipulation slices. STHelpAdapter slices switch from general play to help
 * play.
 */

/* Wrap the slices representing the nested slices
 * @param adapter identifies attack adapter slice
 * @param st address of structure holding the traversal state
 */
void help_adapter_make_intro(slice_index adapter, stip_structure_traversal *st);

/* Try to solve in solve_nr_remaining half-moves.
 * @param si slice index
 * @note assigns solve_result the length of solution found and written, i.e.:
 *            previous_move_is_illegal the move just played is illegal
 *            this_move_is_illegal     the move being played is illegal
 *            immobility_on_next_move  the moves just played led to an
 *                                     unintended immobility on the next move
 *            <=n+1 length of shortest solution found (n+1 only if in next
 *                                     branch)
 *            n+2 no solution found in this branch
 *            n+3 no solution found in next branch
 *            (with n denominating solve_nr_remaining)
 */
void help_adapter_solve(slice_index si);

#endif
