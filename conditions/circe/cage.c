#include "conditions/circe/cage.h"
#include "conditions/circe/rebirth_avoider.h"
#include "conditions/circe/circe.h"
#include "conditions/conditions.h"
#include "conditions/circe/rex_inclusive.h"
#include "solving/has_solution_type.h"
#include "stipulation/stipulation.h"
#include "stipulation/pipe.h"
#include "stipulation/branch.h"
#include "stipulation/fork.h"
#include "solving/temporary_hacks.h"
#include "solving/single_piece_move_generator.h"
#include "solving/post_move_iteration.h"
#include "solving/move_generator.h"
#include "solving/conditional_pipe.h"
#include "solving/pipe.h"
#include "solving/fork.h"
#include "debugging/trace.h"

#include "debugging/assert.h"

static boolean cage_found_for_current_capture[maxply+1];
static boolean no_cage_for_current_capture[maxply+1];

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
  stip_traverse_structure(SLICE_NEXT2(temporary_hack_cagecirce_noncapture_finder[side]),&st);

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

/* Instrument the solving machinery with Circe Cage (apart from the rebirth
 * square determination, whose instrumentation is elsewhere)
 * @param si identifies entry slice into solving machinery
 * @param variant identifies address of structure holding the Circe variant
 * @param interval_start type of slice that starts the sequence of slices
 *                       implementing that variant
 */
void circe_solving_instrument_cage(slice_index si,
                                   struct circe_variant_type const *variant,
                                   slice_type interval_start)
{
  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceEnumerator(slice_type,interval_start);
  TraceFunctionParamListEnd();

  substitute_single_piece_move_generator(White);
  substitute_single_piece_move_generator(Black);

  circe_instrument_solving(si,
                           interval_start,
                           STCirceDeterminedRebirth,
                           alloc_pipe(STCirceCageCageTester));
  circe_cage_optimise_away_futile_captures(si);
  if (circe_get_on_occupied_rebirth_square(variant)
      !=circe_on_occupied_rebirth_square_strict)
    circe_insert_rebirth_avoider(si,
                                 interval_start,
                                 interval_start,
                                 alloc_fork_slice(STCirceCageNoCageFork,no_slice),
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
void circe_cage_no_cage_fork_solve(slice_index si)
{
  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParamListEnd();

  if (!post_move_am_i_iterating())
  {
    /* Initialise for trying the first potential cage. */
    cage_found_for_current_capture[nbply] = false;
    no_cage_for_current_capture[nbply] = false;

    post_move_iteration_solve_delegate(si);
  }
  else if (!no_cage_for_current_capture[nbply])
  {
    /* There is still at least 1 potential cages to be tried. */
    post_move_iteration_solve_delegate(si);

    if (!post_move_iteration_is_locked())
    {
      if (circe_rebirth_context_stack[circe_rebirth_context_stack_pointer].rebirth_square==initsquare
          && !cage_found_for_current_capture[nbply])
        /* No potential cage has materialised. */
        no_cage_for_current_capture[nbply] = true;
      else
        post_move_iteration_end();
    }
  }
  else
  {
    /* Take "the no cage path", */
    post_move_iteration_solve_fork(si);

    /* Terminate this little iteration. */
    if (!post_move_iteration_is_locked())
      post_move_iteration_end();
  }

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

static boolean find_non_capturing_move(move_effect_journal_index_type rebirth,
                                       Side moving_side)
{
  boolean result;
  square const sq_rebirth = move_effect_journal[rebirth].u.piece_addition.added.on;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",rebirth);
  TraceEnumerator(Side,moving_side);
  TraceFunctionParamListEnd();

  init_single_piece_move_generator(sq_rebirth);
  result = (conditional_pipe_solve_delegate(temporary_hack_cagecirce_noncapture_finder[moving_side])
            ==previous_move_has_solved);

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
void circe_cage_cage_tester_solve(slice_index si)
{
  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParamListEnd();

  {
    move_effect_journal_index_type const rebirth = circe_find_current_rebirth();
    if (rebirth<move_effect_journal_base[nbply]+move_effect_journal_index_offset_other_effects)
      pipe_dispatch_delegate(si);
    else if (find_non_capturing_move(rebirth,advers(SLICE_STARTER(si))))
      solve_result = this_move_is_illegal;
    else
    {
      cage_found_for_current_capture[nbply] = true;
      pipe_dispatch_delegate(si);
    }
  }

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

static boolean is_false(numecoup n)
{
  return false;
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
void circe_cage_futile_captures_remover_solve(slice_index si)
{
  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParamListEnd();

  move_generator_filter_captures(MOVEBASE_OF_PLY(nbply),&is_false);

  pipe_dispatch_delegate(si);

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

static void remember_finding(slice_index si, stip_structure_traversal *st)
{
  boolean * const finding = st->param;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParamListEnd();

  stip_traverse_structure_children_pipe(si,st);

  *finding = true;
  stip_traverse_structure_conditional_pipe_tester(si,st);
  *finding = false;

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

static void insert_remover(slice_index si, stip_structure_traversal *st)
{
  boolean const * const finding = st->param;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParamListEnd();

  stip_traverse_structure_children(si,st);

  if (*finding)
  {
    slice_index const prototype = alloc_pipe(STCageCirceFutileCapturesRemover);
    slice_insertion_insert_contextually(si,st->context,&prototype,1);
  }

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

/* Avoid examining captures while looking for a cage
 * @param si identifies root slice of stipulation
 */
void circe_cage_optimise_away_futile_captures(slice_index si)
{
  boolean finding = false;
  stip_structure_traversal st;

  TraceFunctionEntry(__func__);
  TraceFunctionParamListEnd();

  stip_structure_traversal_init(&st,&finding);
  stip_structure_traversal_override_single(&st,
                                           STCageCirceNonCapturingMoveFinder,
                                           &remember_finding);
  stip_structure_traversal_override_single(&st,
                                           STDoneGeneratingMoves,
                                           &insert_remover);
  stip_traverse_structure(si,&st);

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}
