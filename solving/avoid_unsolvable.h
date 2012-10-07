#if !defined(SOLVING_AVOID_UNSOLVABLE_H)
#define SOLVING_AVOID_UNSOLVABLE_H

/* Avoid solving branches that we know are unsolvable
 */

#include "solving/solve.h"

/* maximum number of half-moves that we know have no solution
 */
extern stip_length_type max_unsolvable;

/* Instrument STEndOfBranch* slices with the necessary STAvoidUnusable slices
 * @param root_slice identifes root slice of stipulation
 */
void stip_insert_avoid_unsolvable_forks(slice_index root_slice);

/* Try to solve in n half-moves.
 * @param si slice index
 * @param n maximum number of half moves
 * @return length of solution found and written, i.e.:
 *            slack_length-2 the move just played or being played is illegal
 *            <=n length of shortest solution found
 *            n+2 no solution found
 */
stip_length_type avoid_unsolvable_solve(slice_index si, stip_length_type n);

/* Allocate a STResetUnsolvable slice
 * @return allocated slice
 */
slice_index alloc_reset_unsolvable_slice(void);

/* Try to solve in n half-moves.
 * @param si slice index
 * @param n maximum number of half moves
 * @return length of solution found and written, i.e.:
 *            slack_length-2 the move just played or being played is illegal
 *            <=n length of shortest solution found
 *            n+2 no solution found
 */
stip_length_type reset_unsolvable_solve(slice_index si, stip_length_type n);

/* Allocate a STLearnUnsolvable slice
 * @return allocated slice
 */
slice_index alloc_learn_unsolvable_slice(void);

/* Try to solve in n half-moves.
 * @param si slice index
 * @param n maximum number of half moves
 * @return length of solution found and written, i.e.:
 *            slack_length-2 the move just played or being played is illegal
 *            <=n length of shortest solution found
 *            n+2 no solution found
 */
stip_length_type learn_unsolvable_solve(slice_index si, stip_length_type n);

#endif
