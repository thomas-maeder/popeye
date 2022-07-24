#include "solving/move_played.h"
#include "solving/has_solution_type.h"
#include "solving/pipe.h"
#include "debugging/trace.h"
#include "debugging/assert.h"

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
void attack_played_solve(slice_index si)
{
  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParamListEnd();

  assert(solve_nr_remaining>=next_move_has_solution);

  --solve_nr_remaining;
  pipe_solve_delegate(si);
  ++solve_nr_remaining;

  if (solve_result==immobility_on_next_move) /* oops - unintentional stalemate! */
    solve_result = MOVE_HAS_NOT_SOLVED_LENGTH();
  else
    ++solve_result;

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
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
void defense_played_solve(slice_index si)
{
  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParamListEnd();

  assert(solve_nr_remaining>=next_move_has_solution);

  --solve_nr_remaining;
  pipe_solve_delegate(si);
  ++solve_nr_remaining;

  ++solve_result;

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

/* Determine and write the solution(s) in a help stipulation
 * @param si slice index of slice being solved
 * @param n exact number of half moves until end state has to be reached
 * @return length of solution found, i.e.:
 *         n+4 the move leading to the current position has turned out
 *             to be illegal
 *         n+2 no solution found
 *         n   solution found
 */
void help_move_played_solve(slice_index si)
{
  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParamListEnd();

  assert(solve_nr_remaining>=next_move_has_solution);

  --solve_nr_remaining;
  pipe_solve_delegate(si);
  ++solve_nr_remaining;

  if (solve_result>=previous_move_has_solved)
    ++solve_result;

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}
