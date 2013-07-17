#include "conditions/exclusive.h"
#include "stipulation/stipulation.h"
#include "optimisations/detect_retraction.h"
#include "pymsg.h"
#include "pydata.h"
#include "stipulation/pipe.h"
#include "stipulation/has_solution_type.h"
#include "stipulation/temporary_hacks.h"
#include "stipulation/branch.h"
#include "debugging/trace.h"

#include <assert.h>

table exclusive_chess_undecidable_continuations[maxply+1];
unsigned int exclusive_chess_nr_continuations_reaching_goal[maxply+1];

static Goal exclusive_goal;

static unsigned int nr_decidable_continuations_not_reaching_goal[maxply+1];
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
    boolean are_we_counting_goal_reaching_moves;
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

  if (!state->is_this_mating_move_played_for_testing_exclusivity)
  {
    slice_type const type = (state->are_we_testing_exclusivity
                             ? STExclusiveChessNestedExclusivityDetector
                             : STExclusiveChessExclusivityDetector);
    slice_index const prototype = alloc_pipe(type);
    branch_insert_slices_contextual(si,st->context,&prototype,1);
  }

  stip_traverse_structure_children_pipe(si,st);

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

static void leave_exclusivity_testing(slice_index si,
                                      stip_structure_traversal *st)
{
  insertion_state_type * const state = st->param;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParamListEnd();

  state->are_we_testing_exclusivity = false;
  stip_traverse_structure_children(si,st);
  state->are_we_testing_exclusivity = true;

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

static void remember_counting(slice_index si, stip_structure_traversal *st)
{
  insertion_state_type * const state = st->param;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParamListEnd();

  state->are_we_counting_goal_reaching_moves = true;
  stip_traverse_structure_children(si,st);
  state->are_we_counting_goal_reaching_moves = false;

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

static void substitute_self_check_guard(slice_index si, stip_structure_traversal *st)
{
  insertion_state_type * const state = st->param;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParamListEnd();

  if (state->are_we_counting_goal_reaching_moves)
  {
    state->are_we_counting_goal_reaching_moves = false;
    stip_traverse_structure_children(si,st);
    state->are_we_counting_goal_reaching_moves = true;

    {
      slice_index const guard = branch_find_slice(STSelfCheckGuard,slices[si].next2,st->context);
      if (guard!=no_slice)
      {
        slice_index const prototype = alloc_pipe(STExclusiveChessGoalReachingMoveCounterSelfCheckGuard);
        goal_branch_insert_slices(slices[si].next2,&prototype,1);
        st->traversed[guard] = slice_not_traversed;
        pipe_remove(guard);
      }
    }
  }
  else
    stip_traverse_structure_children(si,st);

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

/* Instrument a stipulation
 * @param si identifies root slice of stipulation
 */
void stip_insert_exclusive_chess(slice_index si)
{
  stip_structure_traversal st;
  insertion_state_type state = { false, false, false };

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParamListEnd();

  stip_structure_traversal_init(&st,&state);
  stip_structure_traversal_override_single(&st,
                                           STExclusiveChessMatingMoveCounterFork,
                                           &avoid_instrumenting_exclusivity_detecting_move);
  stip_structure_traversal_override_single(&st,
                                           STExclusiveChessNestedExclusivityDetector,
                                           &leave_exclusivity_testing);
  stip_structure_traversal_override_single(&st,
                                           STExclusiveChessGoalReachingMoveCounter,
                                           &remember_counting);
  stip_structure_traversal_override_single(&st,
                                           STGoalReachedTester,
                                           &substitute_self_check_guard);
  stip_structure_traversal_override_single(&st,
                                           STGeneratingMoves,
                                           &insert_exclusivity_detector);
  stip_structure_traversal_override_single(&st,STMove,&insert_legality_tester);
  stip_traverse_structure(si,&st);

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
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
stip_length_type exclusive_chess_legality_tester_solve(slice_index si,
                                                       stip_length_type n)
{
  stip_length_type result;
  slice_index const next = slices[si].next1;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParam("%u",n);
  TraceFunctionParamListEnd();

  if ((table_length(exclusive_chess_undecidable_continuations[parent_ply[nbply]])
       +exclusive_chess_nr_continuations_reaching_goal[parent_ply[nbply]])
      >1)
  {
    if (is_current_move_in_table(exclusive_chess_undecidable_continuations[parent_ply[nbply]]))
      result = previous_move_is_illegal;
    else
    {
      stip_length_type const test_result = solve(slices[temporary_hack_mate_tester[advers(trait[nbply])]].next2,slack_length);
      if (test_result==previous_move_is_illegal)
        result = test_result;
      else if (test_result!=slack_length+2)
        result = previous_move_has_solved;
      else
        result = solve(next,n);
    }
  }
  else
    result = solve(next,n);

  TraceFunctionExit(__func__);
  TraceFunctionResult("%u",result);
  TraceFunctionResultEnd();
  return result;
}

/* Detect exclusivity and solve accordingly
 * @param si slice index
 * @param n maximum number of half moves
 * @return see exclusive_chess_exclusivity_detector_solve
 * @note The caller must have initialized ply_horizon for recursive testing for
 *       exclusivity and is responsible of allocating
 *       exclusive_chess_undecidable_continuations[nbply] before and deallocating it
 *       after the invokation.
 */
static stip_length_type detect_exclusivity_and_solve_accordingly(slice_index si,
                                                                 stip_length_type n)
{
  stip_length_type result;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParam("%u",n);
  TraceFunctionParamListEnd();

  exclusive_chess_nr_continuations_reaching_goal[nbply] = 0;
  nr_decidable_continuations_not_reaching_goal[nbply] = 0;

  solve(slices[temporary_hack_exclusive_mating_move_counter[slices[si].starter]].next2,length_unspecified);

  TraceValue("%u",nbply);
  TraceValue("%u",nr_decidable_continuations_not_reaching_goal[nbply]);
  TraceValue("%u",exclusive_chess_nr_continuations_reaching_goal[nbply]);
  TraceValue("%u\n",table_length(exclusive_chess_undecidable_continuations[nbply]));

  ply_horizon = maxply;

  result = solve(slices[si].next1,n);

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
stip_length_type exclusive_chess_exclusivity_detector_solve(slice_index si,
                                                            stip_length_type n)
{
  stip_length_type result;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParam("%u",n);
  TraceFunctionParamListEnd();

  assert(ply_horizon==maxply);
  ply_horizon = nbply+6;

  exclusive_chess_undecidable_continuations[nbply] = allocate_table();

  result = detect_exclusivity_and_solve_accordingly(si,n);

  free_table(exclusive_chess_undecidable_continuations[nbply]);

  TraceFunctionExit(__func__);
  TraceFunctionResult("%u",result);
  TraceFunctionResultEnd();
  return result;
}

static void remember_previous_move_as_undecidable(void)
{
  append_to_table(exclusive_chess_undecidable_continuations[parent_ply[nbply]]);
  TraceValue("%u",nbply);
  TraceValue("%u",parent_ply[nbply]);
  TraceValue("%u\n",table_length(exclusive_chess_undecidable_continuations[parent_ply[nbply]]));
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
    TraceText("stopping recursion");
    remember_previous_move_as_undecidable();
    result = previous_move_is_illegal;
  }
  else
  {
    ply const save_ply_horizon = ply_horizon;

    exclusive_chess_undecidable_continuations[nbply] = allocate_table();

    result = detect_exclusivity_and_solve_accordingly(si,n);

    TraceValue("%u",nbply);
    TraceValue("%u",exclusive_chess_nr_continuations_reaching_goal[nbply]);
    TraceValue("%u",nr_decidable_continuations_not_reaching_goal[nbply]);
    TraceValue("%u\n",table_length(exclusive_chess_undecidable_continuations[nbply]));

    if (nr_decidable_continuations_not_reaching_goal[nbply]==0
        && exclusive_chess_nr_continuations_reaching_goal[nbply]<=1
        && table_length(exclusive_chess_undecidable_continuations[nbply])+exclusive_chess_nr_continuations_reaching_goal[nbply]>1)
      remember_previous_move_as_undecidable();

    free_table(exclusive_chess_undecidable_continuations[nbply]);

    ply_horizon = save_ply_horizon;
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
 *                                     unintended immobility on the next move
 *            <=n+1 length of shortest solution found (n+1 only if in next
 *                                     branch)
 *            n+2 no solution found in this branch
 *            n+3 no solution found in next branch
 */
stip_length_type exclusive_chess_goal_reaching_move_counter_solve(slice_index si,
                                                                  stip_length_type n)
{
  stip_length_type result;
  unsigned int const nr_undecidable_before = table_length(exclusive_chess_undecidable_continuations[parent_ply[nbply]]);

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParam("%u",n);
  TraceFunctionParamListEnd();

  result = solve(slices[si].next1,n);

  if (table_length(exclusive_chess_undecidable_continuations[parent_ply[nbply]])==nr_undecidable_before)
  {
    if (result==n)
    {
      ++exclusive_chess_nr_continuations_reaching_goal[parent_ply[nbply]];

      TraceValue("%u",nbply);
      TraceValue("%u",parent_ply[nbply]);
      TraceValue("%u\n",exclusive_chess_nr_continuations_reaching_goal[parent_ply[nbply]]);

      if (exclusive_chess_nr_continuations_reaching_goal[parent_ply[nbply]]==1)
        /* look for one more */
        result = n+2;
    }
    else if (result==n+2)
    {
      ++nr_decidable_continuations_not_reaching_goal[parent_ply[nbply]];
      TraceText("remembering defined continuation");
      TraceValue("%u",nbply);
      TraceValue("%u",parent_ply[nbply]);
      TraceValue("%u\n",nr_decidable_continuations_not_reaching_goal[parent_ply[nbply]]);
    }
  }
  else if (result==n)
    result = n+2;

  TraceFunctionExit(__func__);
  TraceFunctionResult("%u",result);
  TraceFunctionResultEnd();
  return result;
}
