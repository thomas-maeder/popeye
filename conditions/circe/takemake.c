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
#include "solving/conditional_pipe.h"
#include "solving/pipe.h"
#include "debugging/trace.h"

#include "debugging/assert.h"

static square rebirth_square[toppile+1];
static numecoup take_make_circe_current_rebirth_square_index[maxply+1];
static unsigned int stack_pointer = 1;

static boolean init_rebirth_squares(circe_rebirth_context_elmt_type const *context)
{
  boolean result = false;
  square const sq_capture = context->relevant_square;
  piece_walk_type const pi_capturing = get_walk_of_piece_on_square(sq_capture);
  Flags const flags_capturing = being_solved.spec[sq_capture];

  /* we need to do this for this module to work in both Circe and Anticirce:
   * normally (i.e. unless e.g. mirror is selected), the capturee's walk
   * determines the squares reachable by the make part of moves, independently
   * of whether the reborn piece is the capturer or the capturee.
   */
  Side const relevant_side = (trait[context->relevant_ply]==context->relevant_side
                              ? advers(context->relevant_side)
                              : context->relevant_side);

  TraceFunctionEntry(__func__);
  TraceFunctionParamListEnd();

  take_make_circe_current_rebirth_square_index[stack_pointer] = take_make_circe_current_rebirth_square_index[stack_pointer-1];

  occupy_square(context->relevant_square,
                context->relevant_walk,
                context->reborn_spec);

  init_single_piece_move_generator(context->relevant_square);

  result = (conditional_pipe_solve_delegate(temporary_hack_circe_take_make_rebirth_squares_finder[relevant_side])
            ==previous_move_has_solved);

  assert(pi_capturing!=Invalid);

  if (pi_capturing==Empty) /* en passant, Locust, ... */
    empty_square(context->relevant_square);
  else
    occupy_square(context->relevant_square,pi_capturing,flags_capturing);

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

  if (!post_move_am_i_iterating()
      && !init_rebirth_squares(context))
    solve_result = this_move_is_illegal;
  else
  {
    post_move_iteration_continue();

    context->rebirth_square = rebirth_square[take_make_circe_current_rebirth_square_index[stack_pointer]];

    ++stack_pointer;
    post_move_iteration_solve_delegate(si);
    --stack_pointer;

    if (post_move_iteration_is_locked())
      post_move_iteration_continue();
    else
    {
      assert(take_make_circe_current_rebirth_square_index[stack_pointer]>0);
      --take_make_circe_current_rebirth_square_index[stack_pointer];

      if (take_make_circe_current_rebirth_square_index[stack_pointer]>
          take_make_circe_current_rebirth_square_index[stack_pointer-1])
        post_move_iteration_lock();
      else
        post_move_iteration_end();
    }
  }

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

static void do_substitute(slice_index si,
                                                   stip_structure_traversal *st)
{
  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParamListEnd();

  pipe_substitute(si,alloc_single_piece_move_generator_slice());

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

static void substitute_single_piece_move_generator(Side side)
{
  stip_structure_traversal st;

  TraceFunctionEntry(__func__);
  TraceFunctionParamListEnd();

  stip_structure_traversal_init(&st,0);
  stip_structure_traversal_override_single(&st,STMoveGenerator,&do_substitute);
  stip_traverse_structure(SLICE_NEXT2(temporary_hack_circe_take_make_rebirth_squares_finder[side]),&st);

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

/* Instrument the solving machinery with Circe Take&Make
 */
void circe_solving_instrument_takemake(void)
{
  TraceFunctionEntry(__func__);
  TraceFunctionParamListEnd();

  substitute_single_piece_move_generator(White);
  substitute_single_piece_move_generator(Black);

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}
