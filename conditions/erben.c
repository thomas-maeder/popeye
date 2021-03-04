#include "conditions/erben.h"
#include "position/effects/piece_removal.h"
#include "position/effects/board_transformation.h"
#include "position/effects/total_side_exchange.h"
#include "solving/move_generator.h"
#include "solving/pipe.h"
#include "solving/fork.h"
#include "stipulation/structure_traversal.h"
#include "stipulation/proxy.h"
#include "stipulation/pipe.h"
#include "stipulation/fork.h"
#include "stipulation/branch.h"
#include "stipulation/slice_insertion.h"
#include "stipulation/move.h"
#include "debugging/trace.h"
#include "debugging/assert.h"

/* Allocate a STErbenMovePlayer slice.
 * @param after_move identifies landing slice after move playing
 * @return index of allocated slice
 */
slice_index alloc_erben_player_slice(slice_index after_move)
{
  slice_index result;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",after_move);
  TraceFunctionParamListEnd();

  result = alloc_fork_slice(STErbenMovePlayer,after_move);

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
void erben_player_solve(slice_index si)
{
  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParamListEnd();

  if (move_generation_stack[CURRMOVE_OF_PLY(nbply)].arrival==move_erben)
  {
    move_effect_journal_do_no_piece_removal();
    move_effect_journal_do_board_transformation(move_effect_reason_erben,rot180);
    move_effect_journal_do_total_side_exchange(move_effect_reason_erben);

    fork_solve_delegate(si);
  }
  else
    pipe_solve_delegate(si);

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

static void push_erben_move(void)
{
  TraceFunctionEntry(__func__);
  TraceFunctionParamListEnd();

  INCREMENT_COUNTER(add_to_move_generation_stack);

  assert(current_move[nbply]<toppile);

  curr_generation->departure = move_erben;
  curr_generation->arrival = move_erben;
  curr_generation->capture = move_erben;

  ++current_move[nbply];
  move_generation_stack[CURRMOVE_OF_PLY(nbply)] = *curr_generation;
  move_generation_stack[CURRMOVE_OF_PLY(nbply)].id = current_move_id[nbply];
  ++current_move_id[nbply];
  TraceValue("%u",CURRMOVE_OF_PLY(nbply));
  TraceValue("%u",move_generation_stack[CURRMOVE_OF_PLY(nbply)].id);
  TraceEOL();

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
void erben_generator_solve(slice_index si)
{
  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParamListEnd();

  if (SLICE_STARTER(si)==White)
    push_erben_move();

  pipe_solve_delegate(si);

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

typedef struct
{
    slice_index landing;
    Side side;
} init_struct;

static void insert_erben_handler(slice_index si, stip_structure_traversal *st)
{
  init_struct * const initialiser = st->param;
  slice_index const proxy = alloc_proxy_slice();
  slice_index const prototype = alloc_erben_player_slice(proxy);

  assert(initialiser->landing!=no_slice);
  link_to_branch(proxy,initialiser->landing);

  move_insert_slices(si,st->context,&prototype,1);
}

static void instrument_move_generator(slice_index si,
                                      stip_structure_traversal *st)
{
  init_struct const * const initialiser = st->param;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParamListEnd();

  if (initialiser->side==no_side || initialiser->side==SLICE_STARTER(si))
  {
    slice_index const prototype = alloc_pipe(STErbenMoveGenerator);
    slice_insertion_insert_contextually(si,st->context,&prototype,1);
  }

  stip_traverse_structure_children(si,st);

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

static void insert_landing(slice_index si, stip_structure_traversal *st)
{
  slice_index const prototype = alloc_pipe(STLandingAfterMovingPieceMovement);
  move_insert_slices(si,st->context,&prototype,1);
}

static void instrument_move(slice_index si, stip_structure_traversal *st)
{
  init_struct * const initialiser = st->param;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParamListEnd();

  if (initialiser->side==no_side || initialiser->side==SLICE_STARTER(si))
  {
    init_struct * const initialiser = st->param;
    slice_index const save_landing = initialiser->landing;

    initialiser->landing = no_slice;
    insert_landing(si,st);

    stip_traverse_structure_children(si,st);

    insert_erben_handler(si,st);
    initialiser->landing = save_landing;
  }
  else
    stip_traverse_structure_children(si,st);

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

static void remember_landing(slice_index si, stip_structure_traversal *st)
{
  init_struct * const initialiser = st->param;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParamListEnd();

  assert(initialiser->landing==no_slice);
  stip_traverse_structure_children_pipe(si,st);
  assert(initialiser->landing==no_slice);
  initialiser->landing = si;

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

/* Instrument the solving machinery for erben
 * @param si identifies root slice of stipulation
 * @param side which side may play erben moves? pass no_side for both_sides
 */
void erben_initialise_solving(slice_index si, Side side)
{
  stip_structure_traversal st;
  init_struct initialiser = {
      no_slice,
      side
  };

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceEnumerator(Side,side);
  TraceFunctionParamListEnd();

  stip_structure_traversal_init(&st,&initialiser);
  stip_structure_traversal_override_single(&st,STGeneratingMoves,&instrument_move_generator);
  stip_structure_traversal_override_single(&st,STMove,&instrument_move);
  stip_structure_traversal_override_single(&st,
                                           STLandingAfterMovingPieceMovement,
                                           &remember_landing);
  stip_structure_traversal_override_single(&st,
                                           STEndOfBranchGoal,
                                           &stip_traverse_structure_children_pipe);
  stip_structure_traversal_override_single(&st,
                                           STEndOfBranchGoalImmobile,
                                           &stip_traverse_structure_children_pipe);
  stip_traverse_structure(si,&st);

  position_total_side_exchange_initialise();

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}
