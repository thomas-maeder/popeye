#include "optimisations/goals/enpassant/remove_non_reachers.h"
#include "pydata.h"
#include "pieces/hunters.h"
#include "stipulation/stipulation.h"
#include "stipulation/pipe.h"
#include "stipulation/has_solution_type.h"
#include "solving/en_passant.h"
#include "solving/move_generator.h"
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

static boolean is_enpassant_capture(square sq_departure,
                                    square sq_arrival,
                                    square sq_capture)
{
  boolean result;
  PieNam pi_moving = get_walk_of_piece_on_square(sq_departure);
  PieNam pi_captured = get_walk_of_piece_on_square(sq_capture);

  TraceFunctionEntry(__func__);
  TraceSquare(sq_departure);
  TraceSquare(sq_arrival);
  TraceSquare(sq_capture);
  TraceFunctionParamListEnd();

  if (pi_moving>=Hunter0)
     pi_moving = huntertypes[pi_moving-Hunter0].away;

  if (pi_captured>=Hunter0)
    pi_captured = huntertypes[pi_captured-Hunter0].away;

  result = is_pawn(pi_moving) && is_pawn(pi_captured) && is_square_empty(sq_arrival);

  TraceFunctionExit(__func__);
  TraceFunctionResult("%u",result);
  TraceFunctionResultEnd();
  return result;
}

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
stip_length_type enpassant_remove_non_reachers_solve(slice_index si,
                                                     stip_length_type n)
{
  stip_length_type result;
  slice_index const next = slices[si].next1;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParam("%u",n);
  TraceFunctionParamListEnd();

  move_generator_filter_moves(&is_enpassant_capture);

  result = solve(next,n);

  TraceFunctionExit(__func__);
  TraceFunctionResult("%u",result);
  TraceFunctionResultEnd();
  return result;
}
