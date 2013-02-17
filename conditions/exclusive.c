#include "conditions/exclusive.h"
#include "stipulation/stipulation.h"
#include "optimisations/goals/mate/neutralretractable.h"
#include "pymsg.h"
#include "pydata.h"
#include "stipulation/pipe.h"
#include "stipulation/has_solution_type.h"
#include "stipulation/temporary_hacks.h"
#include "stipulation/branch.h"
#include "debugging/trace.h"

#include <assert.h>

table undefined_moves_after_current_move[maxply+1];

static Goal exclusive_goal;

static unsigned int nr_moves_reaching_goal_after_current_move[maxply+1];
static unsigned int nr_defined_continuations[maxply+1];
static boolean detecting_exclusivity[maxply+1];
static ply ply_horizon = maxply;

/* Perform the necessary verification steps for solving an Exclusive
 * Chess problem
 * @param si identifies root slice of stipulation
 * @return true iff verification passed
 */
boolean exclusive_verifie_position(slice_index si)
{
  boolean result;
  Goal goal;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParamListEnd();

  /* we have to know which goal has to be reached in a dual-free
   * way */
  goal = find_unique_goal(si);
  if (goal.type==no_goal)
  {
    VerifieMsg(StipNotSupported);
    result = false;
  }
  else
  {
    exclusive_goal = goal;
    optim_neutralretractable = false;
    result = true;
  }

  TraceFunctionExit(__func__);
  TraceFunctionResult("%u",result);
  TraceFunctionResultEnd();
  return result;
}

static void remove_guard(slice_index si, stip_structure_traversal *st)
{
  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParamListEnd();

  stip_traverse_structure_children_pipe(si,st);

  {
    slice_index const guard = branch_find_slice(STSelfCheckGuard,
                                                slices[si].next2,
                                                stip_traversal_context_intro);
    assert(guard!=no_slice);
    pipe_remove(guard);
  }

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

/* When counting mating moves, it is not necessary to detect self-check in moves
 * that don't deliver mate; remove the slices that would detect these
 * self-checks
 * @param si identifies slice where to start
 */
void optimise_away_unnecessary_selfcheckguards(slice_index si)
{
  stip_structure_traversal st;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParamListEnd();

  stip_structure_traversal_init(&st,0);
  stip_structure_traversal_override_single(&st,
                                           STExclusiveChessMatingMoveCounterFork,
                                           &remove_guard);
  stip_traverse_structure(si,&st);

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

typedef struct
{
    boolean is_this_mating_move_played_for_testing_exclusivity;
    boolean are_we_testing_exclusivity;
} insertion_state_type;

static void avoid_instrumenting_exclusivity_detecting_move(slice_index si,
                                                           stip_structure_traversal *st)
{
  insertion_state_type * const state = st->param;
  insertion_state_type const save_state = *state;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParamListEnd();

  stip_traverse_structure_children_pipe(si,st);

  state->is_this_mating_move_played_for_testing_exclusivity = true;
  state->are_we_testing_exclusivity = true;
  stip_traverse_structure_conditional_pipe_tester(si,st);
  *state = save_state;

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

static void insert_exclusivity_detector(slice_index si,
                                        stip_structure_traversal *st)
{
  insertion_state_type const * const state = st->param;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParamListEnd();

  stip_traverse_structure_children_pipe(si,st);

  if (!state->is_this_mating_move_played_for_testing_exclusivity)
  {
    slice_type const type = (state->are_we_testing_exclusivity
                             ? STExclusiveChessNestedExclusivityDetector
                             : STExclusiveChessExclusivityDetector);
    slice_index const prototype = alloc_pipe(type);
    branch_insert_slices_contextual(si,st->context,&prototype,1);
  }

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

static void insert_legality_tester(slice_index si,
                                   stip_structure_traversal *st)
{
  insertion_state_type * const state = st->param;
  boolean const save_is_this_mating_move_played_for_testing_exclusivity = state->is_this_mating_move_played_for_testing_exclusivity;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParamListEnd();

  state->is_this_mating_move_played_for_testing_exclusivity = false;
  stip_traverse_structure_children_pipe(si,st);
  state->is_this_mating_move_played_for_testing_exclusivity = save_is_this_mating_move_played_for_testing_exclusivity;

  if (!state->is_this_mating_move_played_for_testing_exclusivity)
  {
    slice_index const prototype = alloc_pipe(STExclusiveChessLegalityTester);
    branch_insert_slices_contextual(si,st->context,&prototype,1);
  }

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

/* Instrument a stipulation
 * @param si identifies root slice of stipulation
 */
void stip_insert_exclusive_chess(slice_index si)
{
  stip_structure_traversal st;
  insertion_state_type state = { false, false };

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParamListEnd();

  stip_structure_traversal_init(&st,&state);
  stip_structure_traversal_override_single(&st,
                                           STExclusiveChessMatingMoveCounterFork,
                                           &avoid_instrumenting_exclusivity_detecting_move);
  stip_structure_traversal_override_single(&st,
                                           STGeneratingMoves,
                                           &insert_exclusivity_detector);
  stip_structure_traversal_override_single(&st,STMove,&insert_legality_tester);
  stip_traverse_structure(si,&st);

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

static boolean is_exclusivity_violated(void)
{
  boolean result;

  TraceFunctionEntry(__func__);
  TraceFunctionParamListEnd();

  assert(exclusive_goal.type==goal_mate);

  if (nr_moves_reaching_goal_after_current_move[parent_ply[nbply]]>=2)
    result = solve(slices[temporary_hack_mate_tester[advers(trait[nbply])]].next2,slack_length)!=slack_length+2;
  else
    result = false;

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
 *                                     uninted immobility on the next move
 *            <=n+1 length of shortest solution found (n+1 only if in next
 *                                     branch)
 *            n+2 no solution found in this branch
 *            n+3 no solution found in next branch
 */
stip_length_type exclusive_chess_legality_tester_solve(slice_index si,
                                                       stip_length_type n)
{
  stip_length_type result;
  slice_index const next = slices[si].next1;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParam("%u",n);
  TraceFunctionParamListEnd();

  if (is_current_move_in_table(undefined_moves_after_current_move[parent_ply[nbply]]))
    result = n+2;
  else if (is_exclusivity_violated())
    result = previous_move_is_illegal;
  else
  {
    result = solve(next,n);

    if (result==n+2)
    {
      ++nr_defined_continuations[parent_ply[nbply]];
      TraceText("remembering defined refutation");
      TraceValue("%u",nbply);
      TraceValue("%u",parent_ply[nbply]);
      TraceValue("%u\n",nr_defined_continuations[parent_ply[nbply]]);
    }
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
 *            previous_move_is_illegal the move just played (or being played)
 *                                     is illegal
 *            immobility_on_next_move  the moves just played led to an
 *                                     uninted immobility on the next move
 *            <=n+1 length of shortest solution found (n+1 only if in next
 *                                     branch)
 *            n+2 no solution found in this branch
 *            n+3 no solution found in next branch
 */
stip_length_type exclusive_chess_exclusivity_detector_solve(slice_index si,
                                                            stip_length_type n)
{
  stip_length_type result;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParam("%u",n);
  TraceFunctionParamListEnd();

  assert(nbply<=ply_horizon);

  {
    ply const save_ply_horizon = ply_horizon;
    nr_moves_reaching_goal_after_current_move[nbply] = 0;
    undefined_moves_after_current_move[nbply] = allocate_table();
    nr_defined_continuations[nbply] = 0;

    detecting_exclusivity[nbply] = true;
//    assert(ply_horizon==maxply);
    if (ply_horizon==maxply)
      ply_horizon = nbply+6;
    solve(slices[temporary_hack_exclusive_mating_move_counter[slices[si].starter]].next2,length_unspecified);
    ply_horizon = save_ply_horizon;
    detecting_exclusivity[nbply] = false;

    TraceValue("%u",nbply);
    TraceValue("%u",nr_defined_continuations[nbply]);
    TraceValue("%u",nr_moves_reaching_goal_after_current_move[nbply]);
    TraceValue("%u\n",table_length(undefined_moves_after_current_move[nbply]));

    result = solve(slices[si].next1,n);

    if (nr_defined_continuations[nbply]==0
        && table_length(undefined_moves_after_current_move[nbply])>0
        && detecting_exclusivity[parent_ply[nbply]])
    {
      append_to_table(undefined_moves_after_current_move[parent_ply[nbply]]);
      TraceValue("%u",nbply);
      TraceValue("%u",parent_ply[nbply]);
      TraceValue("%u\n",table_length(undefined_moves_after_current_move[parent_ply[nbply]]));
    }

    free_table(undefined_moves_after_current_move[nbply]);
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
 *            previous_move_is_illegal the move just played (or being played)
 *                                     is illegal
 *            immobility_on_next_move  the moves just played led to an
 *                                     uninted immobility on the next move
 *            <=n+1 length of shortest solution found (n+1 only if in next
 *                                     branch)
 *            n+2 no solution found in this branch
 *            n+3 no solution found in next branch
 */
stip_length_type exclusive_chess_nested_exclusivity_detector_solve(slice_index si,
                                                                   stip_length_type n)
{
  stip_length_type result;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParam("%u",n);
  TraceFunctionParamListEnd();

  assert(ply_horizon<maxply);

  if (nbply>ply_horizon)
  {
    if (detecting_exclusivity[parent_ply[nbply]])
      append_to_table(undefined_moves_after_current_move[parent_ply[nbply]]);

    TraceText("stopping recursion");
    TraceValue("%u",nbply);
    TraceValue("%u",parent_ply[nbply]);
    TraceValue("%u",nr_moves_reaching_goal_after_current_move[parent_ply[nbply]]);
    TraceValue("%u\n",table_length(undefined_moves_after_current_move[parent_ply[nbply]]));
    result = n+2;
  }
  else
  {
    nr_moves_reaching_goal_after_current_move[nbply] = 0;
    undefined_moves_after_current_move[nbply] = allocate_table();
    nr_defined_continuations[nbply] = 0;

    detecting_exclusivity[nbply] = true;
    solve(slices[temporary_hack_exclusive_mating_move_counter[slices[si].starter]].next2,length_unspecified);
    detecting_exclusivity[nbply] = false;

    TraceValue("%u",nbply);
    TraceValue("%u",nr_defined_continuations[nbply]);
    TraceValue("%u",nr_moves_reaching_goal_after_current_move[nbply]);
    TraceValue("%u\n",table_length(undefined_moves_after_current_move[nbply]));

    result = solve(slices[si].next1,n);

    if (nr_defined_continuations[nbply]==0
        && table_length(undefined_moves_after_current_move[nbply])>0
        && detecting_exclusivity[parent_ply[nbply]])
    {
      append_to_table(undefined_moves_after_current_move[parent_ply[nbply]]);
      TraceValue("%u",nbply);
      TraceValue("%u",parent_ply[nbply]);
      TraceValue("%u\n",table_length(undefined_moves_after_current_move[parent_ply[nbply]]));
    }

    free_table(undefined_moves_after_current_move[nbply]);
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
 *            previous_move_is_illegal the move just played (or being played)
 *                                     is illegal
 *            immobility_on_next_move  the moves just played led to an
 *                                     uninted immobility on the next move
 *            <=n+1 length of shortest solution found (n+1 only if in next
 *                                     branch)
 *            n+2 no solution found in this branch
 *            n+3 no solution found in next branch
 */
stip_length_type exclusive_chess_goal_reaching_move_counter_solve(slice_index si,
                                                                  stip_length_type n)
{
  stip_length_type result;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParam("%u",n);
  TraceFunctionParamListEnd();

  assert(!detecting_exclusivity[nbply]);
  assert(detecting_exclusivity[parent_ply[nbply]]);

  result = solve(slices[si].next1,n);

  if (result==n)
  {
    ++nr_moves_reaching_goal_after_current_move[parent_ply[nbply]];

    TraceValue("%u",nbply);
    TraceValue("%u\n",nr_moves_reaching_goal_after_current_move[parent_ply[nbply]]);

    if (nr_moves_reaching_goal_after_current_move[parent_ply[nbply]]==1)
      result = n+2;
  }

  TraceFunctionExit(__func__);
  TraceFunctionResult("%u",result);
  TraceFunctionResultEnd();
  return result;
}
