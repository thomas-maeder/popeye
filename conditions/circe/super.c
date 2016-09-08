#include "conditions/circe/super.h"
#include "position/position.h"
#include "conditions/circe/rebirth_avoider.h"
#include "conditions/circe/circe.h"
#include "conditions/circe/rex_inclusive.h"
#include "solving/has_solution_type.h"
#include "stipulation/stipulation.h"
#include "stipulation/fork.h"
#include "solving/post_move_iteration.h"
#include "solving/pipe.h"
#include "solving/fork.h"
#include "debugging/trace.h"

#include "debugging/assert.h"

static post_move_iteration_id_type prev_post_move_iteration_id_rebirth[maxply+1];
static post_move_iteration_id_type prev_post_move_iteration_id_no_rebirth[maxply+1];

static boolean is_rebirth_square_dirty[maxply+1];

/* Instrument the solving machinery with Circe Super (apart from the rebirth
 * square determination, whose instrumentation is elsewhere)
 * @param si identifies entry slice into solving machinery
 * @param variant identifies address of structure holding the Circe variant
 * @param interval_start type of slice that starts the sequence of slices
 *                       implementing that variant
 */
void circe_solving_instrument_super(slice_index si,
                                    struct circe_variant_type const *variant,
                                    slice_type interval_start)
{
  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceEnumerator(slice_type,interval_start);
  TraceFunctionParamListEnd();

  if (circe_get_on_occupied_rebirth_square(variant)
      !=circe_on_occupied_rebirth_square_strict)
    circe_insert_rebirth_avoider(si,
                                 interval_start,
                                 interval_start,
                                 alloc_fork_slice(STSuperCirceNoRebirthFork,
                                                  no_slice),
                                 STCirceRebirthAvoided,
                                 STCirceDoneWithRebirth);

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
void supercirce_no_rebirth_fork_solve(slice_index si)
{
  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParamListEnd();

  if (post_move_am_i_iterating(prev_post_move_iteration_id_no_rebirth[nbply]))
  {
    pipe_dispatch_delegate(si);
    prev_post_move_iteration_id_no_rebirth[nbply] = post_move_iteration_id[nbply];
  }
  else
  {
    fork_solve_delegate(si);
    if (!post_move_iteration_is_locked(&prev_post_move_iteration_id_no_rebirth[nbply]))
      post_move_iteration_lock(&prev_post_move_iteration_id_no_rebirth[nbply]);
  }

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
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

  TraceSquare(circe_rebirth_context_stack[circe_rebirth_context_stack_pointer].rebirth_square);
  TraceEOL();

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
void supercirce_determine_rebirth_square_solve(slice_index si)
{
  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParamListEnd();

  if (!post_move_am_i_iterating(prev_post_move_iteration_id_rebirth[nbply]))
  {
    circe_rebirth_context_stack[circe_rebirth_context_stack_pointer].rebirth_square = square_a1-1;
    is_rebirth_square_dirty[nbply] = true;
  }

  if (is_rebirth_square_dirty[nbply] && !advance_rebirth_square())
    solve_result = this_move_is_illegal;
  else
  {
    pipe_dispatch_delegate(si);

    if (!post_move_iteration_is_locked(&prev_post_move_iteration_id_rebirth[nbply]))
    {
      is_rebirth_square_dirty[nbply] = true;
      post_move_iteration_lock(&prev_post_move_iteration_id_rebirth[nbply]);
    }
  }

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
void supercirce_prevent_rebirth_on_non_empty_square_solve(slice_index si)
{
  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParamListEnd();

  solve_result = this_move_is_illegal;

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}
