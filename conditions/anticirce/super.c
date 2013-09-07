#include "conditions/anticirce/super.h"
#include "conditions/circe/circe.h"
#include "stipulation/has_solution_type.h"
#include "stipulation/stipulation.h"
#include "stipulation/pipe.h"
#include "stipulation/branch.h"
#include "stipulation/move.h"
#include "solving/post_move_iteration.h"
#include "solving/move_effect_journal.h"
#include "solving/observation.h"
#include "solving/move_generator.h"
#include "conditions/anticirce/anticirce.h"
#include "conditions/anticirce/capture_fork.h"
#include "debugging/trace.h"

#include <assert.h>

static post_move_iteration_id_type prev_post_move_iteration_id[maxply+1];

static boolean is_rebirth_square_dirty[maxply+1];

static square next_rebirth_square(square beyond_prev_rebirth_square)
{
  square const sq_capture = move_generation_stack[current_move[nbply]-1].capture;
  square result = beyond_prev_rebirth_square;

  while (result!=sq_capture
         && !is_square_empty(result)
         && result<=square_h8)
    ++result;

  return result;
}

static boolean advance_rebirth_square()
{
  boolean result;

  TraceFunctionEntry(__func__);
  TraceFunctionParamListEnd();

  {
    square const next = next_rebirth_square(circe_rebirth_context_stack[circe_rebirth_context_stack_pointer].rebirth_square+1);
    if (next>square_h8)
    {
      circe_rebirth_context_stack[circe_rebirth_context_stack_pointer].rebirth_square = square_a1;
      result = false;
    }
    else
    {
      circe_rebirth_context_stack[circe_rebirth_context_stack_pointer].rebirth_square = next;
      result = true;
    }
  }

  is_rebirth_square_dirty[nbply] = false;

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
stip_length_type antisupercirce_determine_rebirth_square_solve(slice_index si,
                                                               stip_length_type n)
{
  stip_length_type result;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParam("%u",n);
  TraceFunctionParamListEnd();

  if (post_move_iteration_id[nbply]!=prev_post_move_iteration_id[nbply])
  {
    circe_rebirth_context_stack[circe_rebirth_context_stack_pointer].rebirth_square = square_a1-1;
    is_rebirth_square_dirty[nbply] = true;
  }

  if (is_rebirth_square_dirty[nbply] && !advance_rebirth_square())
  {
    circe_rebirth_context_stack[circe_rebirth_context_stack_pointer].rebirth_square = initsquare;
    result = previous_move_is_illegal;
  }
  else
  {
    circe_rebirth_context_stack[circe_rebirth_context_stack_pointer].rebirth_reason = move_effect_reason_antisupercirce_rebirth;
    result = solve(slices[si].next1,n);

    if (!post_move_iteration_locked[nbply])
    {
      is_rebirth_square_dirty[nbply] = true;
      lock_post_move_iterations();
    }
  }

  prev_post_move_iteration_id[nbply] = post_move_iteration_id[nbply];

  TraceFunctionExit(__func__);
  TraceFunctionResult("%u",result);
  TraceFunctionResultEnd();
  return result;
}

/* Initialise solving in Antisupercirce
 * @param si identifies root slice of stipulation
 */
void antisupercirce_initialise_solving(slice_index si)
{
  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParamListEnd();

  stip_instrument_moves(si,STAnticirceDetermineRebornPiece);
  stip_instrument_moves(si,STAntisupercirceDetermineRebirthSquare);
  stip_insert_anticirce_capture_forks(si);

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}
