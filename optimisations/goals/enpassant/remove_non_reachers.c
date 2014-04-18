#include "optimisations/goals/enpassant/remove_non_reachers.h"
#include "pieces/walks/hunters.h"
#include "pieces/walks/pawns/en_passant.h"
#include "pieces/walks/classification.h"
#include "solving/move_generator.h"
#include "stipulation/stipulation.h"
#include "stipulation/pipe.h"
#include "solving/has_solution_type.h"
#include "solving/pipe.h"
#include "debugging/trace.h"

/* Allocate a STEnPassantRemoveNonReachers slice.
 * @return index of allocated slice
 */
slice_index alloc_enpassant_remove_non_reachers_slice(void)
{
  slice_index result;

  TraceFunctionEntry(__func__);
  TraceFunctionParamListEnd();

  result = alloc_pipe(STEnPassantRemoveNonReachers);

  TraceFunctionExit(__func__);
  TraceFunctionResult("%u",result);
  TraceFunctionResultEnd();
  return result;
}

static boolean is_enpassant_capture(numecoup n)
{
  boolean result;

  TraceFunctionEntry(__func__);
  TraceFunctionParamListEnd();

  result = en_passant_is_ep_capture(move_generation_stack[n].capture);

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
void enpassant_remove_non_reachers_solve(slice_index si)
{
  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParamListEnd();

  move_generator_filter_moves(MOVEBASE_OF_PLY(nbply),&is_enpassant_capture);

  pipe_solve_delegate(si);

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}
