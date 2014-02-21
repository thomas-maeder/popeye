#include "conditions/circe/super.h"
#include "conditions/circe/rebirth_avoider.h"
#include "conditions/circe/circe.h"
#include "stipulation/has_solution_type.h"
#include "stipulation/stipulation.h"
#include "stipulation/move.h"
#include "stipulation/pipe.h"
#include "solving/post_move_iteration.h"
#include "debugging/trace.h"

#include "debugging/assert.h"

static post_move_iteration_id_type prev_post_move_iteration_id_rebirth[maxply+1];
static post_move_iteration_id_type prev_post_move_iteration_id_no_rebirth[maxply+1];

static boolean is_rebirth_square_dirty[maxply+1];

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
stip_length_type supercirce_capture_fork_solve(slice_index si,
                                               stip_length_type n)
{
  stip_length_type result;
  move_effect_journal_index_type const top = move_effect_journal_base[nbply];
  move_effect_journal_index_type const capture = top+move_effect_journal_index_offset_capture;
  Flags const removedspec = move_effect_journal[capture].u.piece_removal.removedspec;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParam("%u",n);
  TraceFunctionParamListEnd();

  if (move_effect_journal[capture].type==move_effect_no_piece_removal
      || TSTFLAG(removedspec,Royal))
    result = solve(slices[si].next2,n);
  else
  {
    circe_rebirth_context_stack[circe_rebirth_context_stack_pointer].rebirth_reason = move_effect_reason_rebirth_choice;
    result = solve(slices[si].next1,n);
  }

  TraceFunctionExit(__func__);
  TraceFunctionResult("%u",result);
  TraceFunctionResultEnd();
  return result;
}

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
stip_length_type supercirce_no_rebirth_fork_solve(slice_index si,
                                                   stip_length_type n)
{
  stip_length_type result;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParam("%u",n);
  TraceFunctionParamListEnd();

  if (post_move_iteration_id[nbply]!=prev_post_move_iteration_id_no_rebirth[nbply])
  {
    result = solve(slices[si].next2,n);
    if (!post_move_iteration_locked[nbply])
      lock_post_move_iterations();
  }
  else
    result = solve(slices[si].next1,n);

  prev_post_move_iteration_id_no_rebirth[nbply] = post_move_iteration_id[nbply];

  TraceFunctionExit(__func__);
  TraceFunctionResult("%u",result);
  TraceFunctionResultEnd();
  return result;
}

static boolean advance_rebirth_square(void)
{
  boolean result = true;

  TraceFunctionEntry(__func__);
  TraceFunctionParamListEnd();

  do
  {
    if (circe_rebirth_context_stack[circe_rebirth_context_stack_pointer].rebirth_square<square_h8)
      ++circe_rebirth_context_stack[circe_rebirth_context_stack_pointer].rebirth_square;
    else
    {
      circe_rebirth_context_stack[circe_rebirth_context_stack_pointer].rebirth_square = initsquare;
      result = false;
      break;
    }
  } while (is_square_blocked(circe_rebirth_context_stack[circe_rebirth_context_stack_pointer].rebirth_square));

  is_rebirth_square_dirty[nbply] = false;

  TraceSquare(circe_rebirth_context_stack[circe_rebirth_context_stack_pointer].rebirth_square);TraceEOL();

  TraceFunctionExit(__func__);
  TraceFunctionResult("%u",result);
  TraceFunctionResultEnd();
  return result;
}

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
stip_length_type supercirce_determine_rebirth_square_solve(slice_index si,
                                                           stip_length_type n)
{
  stip_length_type result;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParam("%u",n);
  TraceFunctionParamListEnd();

  if (post_move_iteration_id[nbply]!=prev_post_move_iteration_id_rebirth[nbply])
  {
    circe_rebirth_context_stack[circe_rebirth_context_stack_pointer].rebirth_square = square_a1-1;
    is_rebirth_square_dirty[nbply] = true;
  }

  if (is_rebirth_square_dirty[nbply] && !advance_rebirth_square())
    result = this_move_is_illegal;
  else
  {
    result = solve(slices[si].next1,n);

    if (!post_move_iteration_locked[nbply])
    {
      is_rebirth_square_dirty[nbply] = true;
      lock_post_move_iterations();
    }
  }

  prev_post_move_iteration_id_rebirth[nbply] = post_move_iteration_id[nbply];

  TraceFunctionExit(__func__);
  TraceFunctionResult("%u",result);
  TraceFunctionResultEnd();
  return result;
}

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
stip_length_type supercirce_prevent_rebirth_on_non_empty_square_solve(slice_index si,
                                                                      stip_length_type n)
{
  stip_length_type result;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParam("%u",n);
  TraceFunctionParamListEnd();

  result = this_move_is_illegal;

  TraceFunctionExit(__func__);
  TraceFunctionResult("%u",result);
  TraceFunctionResultEnd();
  return result;
}

static void append_stop(slice_index si, stip_structure_traversal*st)
{
  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParamListEnd();

  stip_traverse_structure_children_pipe(si,st);

  {
    slice_index const prototype = alloc_pipe(STSupercircePreventRebirthOnNonEmptySquare);
    branch_insert_slices_contextual(si,st->context,&prototype,1);
  }

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

/* Instrument the solving machinery with Super-Circe
 * @param si identifies root slice of stipulation
 */
void supercirce_initialise_solving(slice_index si)
{
  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParamListEnd();

  stip_instrument_moves(si,STCirceDetermineRebornPiece);
  stip_instrument_moves(si,STSuperCirceDetermineRebirthSquare);
  stip_instrument_moves(si,STCircePlacingReborn);
  stip_instrument_moves(si,STCircePlaceReborn);
  stip_insert_rebirth_avoider(si,STCirceTestRebirthSquareEmpty,STCirceRebirthOnNonEmptySquare);
  stip_insert_rebirth_avoider(si,STCirceTestRebornExistance,STCirceRebirthAvoided);
  stip_insert_rebirth_avoider(si,STSuperCirceNoRebirthFork,STCirceRebirthAvoided);
  stip_insert_rebirth_avoider(si,STSuperCirceCaptureFork,STCirceRebirthAvoided);

  {
    stip_structure_traversal st;
    stip_structure_traversal_init(&st,0);
    stip_structure_traversal_override_single(&st,
                                             STCirceRebirthOnNonEmptySquare,
                                             &append_stop);
    stip_traverse_structure(si,&st);
  }

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}
