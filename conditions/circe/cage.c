#include "conditions/circe/cage.h"
#include "conditions/circe/rebirth_avoider.h"
#include "conditions/circe/circe.h"
#include "conditions/conditions.h"
#include "stipulation/has_solution_type.h"
#include "stipulation/stipulation.h"
#include "stipulation/pipe.h"
#include "stipulation/branch.h"
#include "stipulation/temporary_hacks.h"
#include "solving/single_piece_move_generator.h"
#include "solving/post_move_iteration.h"
#include "solving/move_generator.h"
#include "debugging/trace.h"

#include <assert.h>

static post_move_iteration_id_type prev_post_move_iteration_id_no_cage[maxply+1];

static boolean cage_found_for_current_capture[maxply+1];
static boolean no_cage_for_current_capture[maxply+1];

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
stip_length_type circe_cage_no_cage_fork_solve(slice_index si,
                                                stip_length_type n)
{
  stip_length_type result;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParam("%u",n);
  TraceFunctionParamListEnd();

  if (post_move_iteration_id[nbply]==prev_post_move_iteration_id_no_cage[nbply])
  {
    if (no_cage_for_current_capture[nbply])
      result = solve(slices[si].next2,n);
    else
    {
      result = solve(slices[si].next1,n);

      if (!post_move_iteration_locked[nbply]
          && circe_rebirth_context_stack[circe_rebirth_context_stack_pointer].rebirth_square==initsquare
          && !cage_found_for_current_capture[nbply])
      {
        no_cage_for_current_capture[nbply] = true;
        lock_post_move_iterations();
      }
    }
  }
  else
  {
    cage_found_for_current_capture[nbply] = false;
    no_cage_for_current_capture[nbply] = false;
    result = solve(slices[si].next1,n);
  }

  prev_post_move_iteration_id_no_cage[nbply] = post_move_iteration_id[nbply];

  TraceFunctionExit(__func__);
  TraceFunctionResult("%u",result);
  TraceFunctionResultEnd();
  return result;
}

static boolean find_non_capturing_move(square sq_departure, Side moving_side)
{
  boolean result;

  TraceFunctionEntry(__func__);
  TraceSquare(sq_departure);
  TraceEnumerator(Side,moving_side,"");
  TraceFunctionParamListEnd();

  init_single_piece_move_generator(sq_departure);
  result = solve(slices[temporary_hack_cagecirce_noncapture_finder[moving_side]].next2,length_unspecified)==next_move_has_solution;

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
stip_length_type circe_cage_cage_tester_solve(slice_index si,
                                               stip_length_type n)
{
  stip_length_type result;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParam("%u",n);
  TraceFunctionParamListEnd();

  {
    move_effect_journal_index_type const rebirth = circe_find_current_rebirth();
    if (rebirth==move_effect_journal_base[nbply+1])
      result = solve(slices[si].next1,n);
    else
    {
      square const sq_rebirth = move_effect_journal[rebirth].u.piece_addition.on;
      assert(move_effect_journal[rebirth].type==move_effect_piece_readdition);

      TraceSquare(sq_rebirth);TraceText("\n");

      if (find_non_capturing_move(sq_rebirth,advers(slices[si].starter)))
        result = previous_move_is_illegal;
      else
      {
        cage_found_for_current_capture[nbply] = true;
        result = solve(slices[si].next1,n);
      }
    }
  }

  TraceFunctionExit(__func__);
  TraceFunctionResult("%u",result);
  TraceFunctionResultEnd();
  return result;
}

static boolean is_false(numecoup n)
{
  return false;
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
stip_length_type circe_cage_futile_captures_remover_solve(slice_index si,
                                                          stip_length_type n)
{
  stip_length_type result;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParam("%u",n);
  TraceFunctionParamListEnd();

  move_generator_filter_captures(MOVEBASE_OF_PLY(nbply),&is_false);

  result = solve(slices[si].next1,n);

  TraceFunctionExit(__func__);
  TraceFunctionResult("%u",result);
  TraceFunctionResultEnd();
  return result;
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

static void instrument_move(slice_index si, stip_structure_traversal *st)
{
  boolean const * const finding = st->param;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParamListEnd();

  stip_traverse_structure_children_pipe(si,st);

  if (!*finding)
  {
    slice_index const prototypes[] =
    {
        alloc_pipe(STCirceDetermineRebornPiece),
        alloc_pipe(STSuperCirceRebirthHandler),
        alloc_pipe(STCircePlaceReborn),
        alloc_pipe(STCirceCageCageTester)
    };
    enum { nr_prototypes = sizeof prototypes / sizeof prototypes[0] };
    branch_insert_slices_contextual(si,st->context,prototypes,nr_prototypes);
  }

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
    branch_insert_slices_contextual(si,st->context,&prototype,1);
  }

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

/* Instrument the solving machinery with Circe Cage
 * @param si identifies root slice of stipulation
 */
void circe_cage_initialise_solving(slice_index si)
{
  boolean finding = false;
  stip_structure_traversal st;

  TraceFunctionEntry(__func__);
  TraceFunctionParamListEnd();

  stip_structure_traversal_init(&st,&finding);
  stip_structure_traversal_override_single(&st,
                                           STMove,
                                           &instrument_move);
  stip_structure_traversal_override_single(&st,
                                           STCageCirceNonCapturingMoveFinder,
                                           &remember_finding);
  stip_structure_traversal_override_single(&st,
                                           STDoneGeneratingMoves,
                                           &insert_remover);
  stip_traverse_structure(si,&st);

  stip_insert_rebirth_avoider(si,STSuperCirceCaptureFork);

  if (!CondFlag[immuncage])
    stip_insert_rebirth_avoider(si,STCirceCageNoCageFork);

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}
