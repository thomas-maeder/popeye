#include "conditions/blackchecks.h"
#include "solving/move_generator.h"
#include "position/effects/null_move.h"
#include "solving/check.h"
#include "solving/pipe.h"
#include "solving/fork.h"
#include "stipulation/structure_traversal.h"
#include "stipulation/proxy.h"
#include "stipulation/pipe.h"
#include "stipulation/fork.h"
#include "stipulation/branch.h"
#include "stipulation/slice_insertion.h"
#include "stipulation/move.h"
#include "stipulation/battle_play/branch.h"
#include "stipulation/help_play/branch.h"
#include "debugging/trace.h"
#include "debugging/assert.h"

/* Determine the length of a move for the Black Checks condition; the higher the
 * value the more likely the move is going to be played.
 * @return a value expressing the precedence of this move
 */
mummer_length_type blackchecks_measure_length(void)
{
   return move_generation_stack[CURRMOVE_OF_PLY(nbply)].arrival!=nullsquare;
}

/* Allocate a STNullMovePlayer slice.
 * @param after_move identifies landing slice after move playing
 * @return index of allocated slice
 */
static slice_index alloc_null_move_player_slice(slice_index after_move)
{
  slice_index result;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",after_move);
  TraceFunctionParamListEnd();

  result = alloc_fork_slice(STNullMovePlayer,after_move);

  TraceFunctionExit(__func__);
  TraceFunctionResult("%u",result);
  TraceFunctionResultEnd();
  return result;
}

static void insert_null_move_handler(slice_index si, stip_structure_traversal *st)
{
  slice_index const * const landing = st->param;
  slice_index const proxy = alloc_proxy_slice();
  slice_index const prototype = alloc_null_move_player_slice(proxy);

  assert(*landing!=no_slice);
  link_to_branch(proxy,*landing);

  move_insert_slices(si,st->context,&prototype,1);
}

static void insert_landing(slice_index si, stip_structure_traversal *st)
{
  slice_index const prototype = alloc_pipe(STLandingAfterMovingPieceMovement);
  move_insert_slices(si,st->context,&prototype,1);
}

static void instrument_move(slice_index si, stip_structure_traversal *st)
{
  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParamListEnd();

  if (SLICE_STARTER(si)==Black)
  {
    slice_index * const landing = st->param;
    slice_index const save_landing = *landing;

    *landing = no_slice;
    insert_landing(si,st);

    stip_traverse_structure_children(si,st);

    insert_null_move_handler(si,st);
    *landing = save_landing;

    {
      slice_index const prototype = alloc_pipe(STBlackChecks);
      move_insert_slices(si,st->context,&prototype,1);
    }
  }
  else
    stip_traverse_structure_children(si,st);

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

static void remember_landing(slice_index si, stip_structure_traversal *st)
{
  slice_index * const landing = st->param;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParamListEnd();

  assert(*landing==no_slice);
  stip_traverse_structure_children_pipe(si,st);
  assert(*landing==no_slice);
  *landing = si;

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

static void instrument_move_generator(slice_index si,
                                      stip_structure_traversal *st)
{
  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParamListEnd();

  if (SLICE_STARTER(si)==Black)
  {
    slice_index const prototype = alloc_pipe(STBlackChecksNullMoveGenerator);
    slice_insertion_insert_contextually(si,st->context,&prototype,1);
  }

  stip_traverse_structure_children(si,st);

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

/* Instrument the solving machinery for BlackChecks
 * @param si identifies root slice of stipulation
 */
void blackchecks_initialise_solving(slice_index si)
{
  stip_structure_traversal st;
  slice_index landing = no_slice;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParamListEnd();

  stip_structure_traversal_init(&st,&landing);
  stip_structure_traversal_override_single(&st,STGoalReachedTester,&stip_structure_visitor_noop);
  stip_structure_traversal_override_single(&st,STMove,&instrument_move);
  stip_structure_traversal_override_single(&st,STGeneratingMoves,&instrument_move_generator);
  stip_structure_traversal_override_single(&st,
                                           STLandingAfterMovingPieceMovement,
                                           &remember_landing);
  stip_traverse_structure(si,&st);

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
void blackchecks_solve(slice_index si)
{
  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParamListEnd();

  pipe_this_move_illegal_if(si,
                            move_generation_stack[CURRMOVE_OF_PLY(nbply)].arrival!=nullsquare
                            && !is_in_check(White));

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
void null_move_player_solve(slice_index si)
{
  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParamListEnd();

  if (move_generation_stack[CURRMOVE_OF_PLY(nbply)].arrival==nullsquare)
  {
    move_effect_journal_do_null_move();
    fork_solve_delegate(si);
  }
  else
    pipe_solve_delegate(si);

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
void black_checks_null_move_generator_solve(slice_index si)
{
  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParamListEnd();

  assert(trait[nbply]==Black);

  curr_generation->departure = nullsquare;
  curr_generation->arrival = nullsquare;
  push_move_no_capture();
  pipe_solve_delegate(si);

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}
