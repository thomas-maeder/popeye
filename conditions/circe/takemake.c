#include "conditions/circe/takemake.h"
#include "position/position.h"
#include "conditions/circe/rebirth_avoider.h"
#include "conditions/circe/circe.h"
#include "conditions/circe/rex_inclusive.h"
#include "solving/has_solution_type.h"
#include "stipulation/pipe.h"
#include "stipulation/branch.h"
#include "stipulation/move.h"
#include "solving/temporary_hacks.h"
#include "solving/post_move_iteration.h"
#include "solving/single_piece_move_generator.h"
#include "solving/move_generator.h"
#include "solving/fork.h"
#include "solving/pipe.h"
#include "debugging/trace.h"

#include "debugging/assert.h"

static post_move_iteration_id_type prev_post_move_iteration_id[maxply+1];
static square rebirth_square[toppile+1];
static numecoup take_make_circe_current_rebirth_square_index[maxply+1];
static unsigned int stack_pointer = 1;

static boolean init_rebirth_squares(Side side_reborn)
{
  boolean result = false;
  square const sq_capture = move_generation_stack[CURRMOVE_OF_PLY(nbply)].capture;
  piece_walk_type const pi_capturing = get_walk_of_piece_on_square(sq_capture);
  Flags const flags_capturing = spec[sq_capture];
  move_effect_journal_index_type const top = move_effect_journal_base[nbply];
  move_effect_journal_index_type const capture = top+move_effect_journal_index_offset_capture;

  TraceFunctionEntry(__func__);
  TraceFunctionParamListEnd();

  take_make_circe_current_rebirth_square_index[stack_pointer] = take_make_circe_current_rebirth_square_index[stack_pointer-1];

  occupy_square(sq_capture,
                move_effect_journal[capture].u.piece_removal.walk,
                move_effect_journal[capture].u.piece_removal.flags);

  init_single_piece_move_generator(sq_capture);

  result = (fork_solve(temporary_hack_circe_take_make_rebirth_squares_finder[side_reborn],
                       length_unspecified)
            ==next_move_has_solution);

  assert(pi_capturing!=Invalid);

  if (pi_capturing==Empty)
    empty_square(sq_capture);
  else
    occupy_square(sq_capture,pi_capturing,flags_capturing);

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
void take_make_circe_collect_rebirth_squares_solve(slice_index si)
{
  numecoup i;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParamListEnd();

  if (CURRMOVE_OF_PLY(nbply)>MOVEBASE_OF_PLY(nbply))
  {
    solve_result = MOVE_HAS_SOLVED_LENGTH();

    for (i = CURRMOVE_OF_PLY(nbply); i>MOVEBASE_OF_PLY(nbply); --i)
    {
      ++take_make_circe_current_rebirth_square_index[stack_pointer];
      rebirth_square[take_make_circe_current_rebirth_square_index[stack_pointer]] = move_generation_stack[i].arrival;
    }
  }
  else
    solve_result = this_move_is_illegal;

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
void take_make_circe_determine_rebirth_squares_solve(slice_index si)
{
  circe_rebirth_context_elmt_type * const context = &circe_rebirth_context_stack[circe_rebirth_context_stack_pointer];

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParamListEnd();

  if (post_move_iteration_id[nbply]!=prev_post_move_iteration_id[nbply]
      && !init_rebirth_squares(advers(context->relevant_side)))
    solve_result = this_move_is_illegal;
  else
  {
    context->rebirth_square = rebirth_square[take_make_circe_current_rebirth_square_index[stack_pointer]];

    ++stack_pointer;
    pipe_solve_delegate(si);
    --stack_pointer;

    if (!post_move_iteration_locked[nbply])
    {
      --take_make_circe_current_rebirth_square_index[stack_pointer];

      if (take_make_circe_current_rebirth_square_index[stack_pointer]>take_make_circe_current_rebirth_square_index[stack_pointer-1])
        lock_post_move_iterations();
    }

    prev_post_move_iteration_id[nbply] = post_move_iteration_id[nbply];
  }

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}
