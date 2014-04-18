#if !defined(SOLVING_AVOID_UNSOLVABLE_H)
#define SOLVING_AVOID_UNSOLVABLE_H

/* Avoid solving branches that we know are unsolvable
 */

#include "solving/machinery/solve.h"

/* maximum number of half-moves that we know have no solution
 */
extern stip_length_type max_unsolvable;

/* Instrument the solving machinery with the necessary STAvoidUnusable slices
 * @param root_slice identifies root slice of the solving machinery
 */
void solving_insert_avoid_unsolvable_forks(slice_index root_slice);

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
void avoid_unsolvable_solve(slice_index si);

/* Allocate a STResetUnsolvable slice
 * @return allocated slice
 */
slice_index alloc_reset_unsolvable_slice(void);

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
void reset_unsolvable_solve(slice_index si);

/* Allocate a STLearnUnsolvable slice
 * @return allocated slice
 */
slice_index alloc_learn_unsolvable_slice(void);

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
void learn_unsolvable_solve(slice_index si);

#endif
