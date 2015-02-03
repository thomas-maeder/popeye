#include "solving/find_by_increasing_length.h"
#include "stipulation/branch.h"
#include "stipulation/pipe.h"
#include "stipulation/branch.h"
#include "stipulation/proxy.h"
#include "stipulation/help_play/branch.h"
#include "debugging/trace.h"

#include "debugging/assert.h"

/* Allocate a STFindByIncreasingLength slice.
 * @param length maximum number of half-moves of slice (+ slack)
 * @param min_length minimum number of half-moves of slice (+ slack)
 * @return index of allocated slice
 */
slice_index alloc_find_by_increasing_length_slice(stip_length_type length,
                                                  stip_length_type min_length)
{
  slice_index result;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",length);
  TraceFunctionParam("%u",min_length);
  TraceFunctionParamListEnd();

  result = alloc_branch(STFindByIncreasingLength,length,min_length);

  TraceFunctionExit(__func__);
  TraceFunctionResult("%u",result);
  TraceFunctionResultEnd();
  return result;
}

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
void find_by_increasing_length_solve(slice_index si)
{
  stip_length_type result_intermediate = MOVE_HAS_NOT_SOLVED_LENGTH();
  stip_length_type const save_solve_nr_remaining = MOVE_HAS_SOLVED_LENGTH();

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParamListEnd();

  for (solve_nr_remaining = SLICE_U(si).branch.min_length;
       solve_nr_remaining<=save_solve_nr_remaining;
       solve_nr_remaining += 2)
  {
    solve(SLICE_NEXT1(si));
    if (solve_result==MOVE_HAS_SOLVED_LENGTH()
        && solve_nr_remaining<result_intermediate)
      result_intermediate = solve_nr_remaining;
  }

  solve_nr_remaining = save_solve_nr_remaining;

  solve_result = result_intermediate;

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}
