#include "conditions/exclusive.h"
#include "stipulation/stipulation.h"
#include "stipulation/goals/slice_insertion.h"
#include "optimisations/detect_retraction.h"
#include "output/plaintext/message.h"
#include "stipulation/pipe.h"
#include "stipulation/branch.h"
#include "stipulation/slice_insertion.h"
#include "stipulation/move.h"
#include "solving/has_solution_type.h"
#include "solving/temporary_hacks.h"
#include "solving/conditional_pipe.h"
#include "solving/pipe.h"
#include "debugging/trace.h"

#include "debugging/assert.h"

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
    output_plaintext_verifie_message(StipNotSupported);
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
                                                SLICE_NEXT2(si),
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
    slice_insertion_insert_contextually(si,st->context,&prototype,1);
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
    move_insert_slices(si,st->context,&prototype,1);
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
      slice_index const guard = branch_find_slice(STSelfCheckGuard,SLICE_NEXT2(si),st->context);
      if (guard!=no_slice)
      {
        slice_index const prototype = alloc_pipe(STExclusiveChessGoalReachingMoveCounterSelfCheckGuard);
        goal_branch_insert_slices(SLICE_NEXT2(si),&prototype,1);
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
void solving_insert_exclusive_chess(slice_index si)
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
  stip_structure_traversal_override_single(&st,
                                           STKingCaptureLegalityTester,
                                           &stip_traverse_structure_children_pipe);
  stip_traverse_structure(si,&st);

  reset_tables();

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
void exclusive_chess_legality_tester_solve(slice_index si)
{
  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParamListEnd();

  if ((table_length(exclusive_chess_undecidable_continuations[parent_ply[nbply]])
       +exclusive_chess_nr_continuations_reaching_goal[parent_ply[nbply]])
      >1)
  {
    if (is_current_move_in_table(exclusive_chess_undecidable_continuations[parent_ply[nbply]]))
      solve_result = this_move_is_illegal;
    else
      switch (conditional_pipe_solve_delegate(temporary_hack_mate_tester[advers(trait[nbply])]))
      {
        case this_move_is_illegal:
          solve_result = this_move_is_illegal;
          break;

        case previous_move_has_not_solved:
          pipe_solve_delegate(si);
          break;

        default:
          solve_result = previous_move_has_solved;
          break;
      }
  }
  else
    pipe_solve_delegate(si);

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
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
static void detect_exclusivity_and_solve_accordingly(slice_index si)
{
  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParamListEnd();

  exclusive_chess_nr_continuations_reaching_goal[nbply] = 0;
  nr_decidable_continuations_not_reaching_goal[nbply] = 0;

  conditional_pipe_solve_delegate(temporary_hack_exclusive_mating_move_counter[SLICE_STARTER(si)]);

  TraceValue("%u",nbply);
  TraceValue("%u",nr_decidable_continuations_not_reaching_goal[nbply]);
  TraceValue("%u",exclusive_chess_nr_continuations_reaching_goal[nbply]);
  TraceValue("%u",table_length(exclusive_chess_undecidable_continuations[nbply]));
  TraceEOL();

  ply_horizon = maxply;

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
void exclusive_chess_exclusivity_detector_solve(slice_index si)
{
  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParamListEnd();

  assert(ply_horizon==maxply);
  ply_horizon = nbply+6;

  exclusive_chess_undecidable_continuations[nbply] = allocate_table();

  detect_exclusivity_and_solve_accordingly(si);

  free_table(exclusive_chess_undecidable_continuations[nbply]);

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

static void remember_previous_move_as_undecidable(void)
{
  append_to_table(exclusive_chess_undecidable_continuations[parent_ply[nbply]]);
  TraceValue("%u",nbply);
  TraceValue("%u",parent_ply[nbply]);
  TraceValue("%u",table_length(exclusive_chess_undecidable_continuations[parent_ply[nbply]]));
  TraceEOL();
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
void exclusive_chess_nested_exclusivity_detector_solve(slice_index si)
{
  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParamListEnd();

  assert(ply_horizon<maxply);

  if (nbply>ply_horizon)
  {
    TraceText("stopping recursion");
    remember_previous_move_as_undecidable();
    solve_result = previous_move_is_illegal;
  }
  else
  {
    ply const save_ply_horizon = ply_horizon;

    exclusive_chess_undecidable_continuations[nbply] = allocate_table();

    detect_exclusivity_and_solve_accordingly(si);

    TraceValue("%u",nbply);
    TraceValue("%u",exclusive_chess_nr_continuations_reaching_goal[nbply]);
    TraceValue("%u",nr_decidable_continuations_not_reaching_goal[nbply]);
    TraceValue("%u",table_length(exclusive_chess_undecidable_continuations[nbply]));
    TraceEOL();

    if (nr_decidable_continuations_not_reaching_goal[nbply]==0
        && exclusive_chess_nr_continuations_reaching_goal[nbply]<=1
        && table_length(exclusive_chess_undecidable_continuations[nbply])+exclusive_chess_nr_continuations_reaching_goal[nbply]>1)
      remember_previous_move_as_undecidable();

    free_table(exclusive_chess_undecidable_continuations[nbply]);

    ply_horizon = save_ply_horizon;
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
void exclusive_chess_goal_reaching_move_counter_solve(slice_index si)
{
  unsigned int const nr_undecidable_before = table_length(exclusive_chess_undecidable_continuations[parent_ply[nbply]]);

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParamListEnd();

  pipe_solve_delegate(si);

  if (table_length(exclusive_chess_undecidable_continuations[parent_ply[nbply]])==nr_undecidable_before)
  {
    if (solve_result==MOVE_HAS_SOLVED_LENGTH())
    {
      ++exclusive_chess_nr_continuations_reaching_goal[parent_ply[nbply]];

      TraceValue("%u",nbply);
      TraceValue("%u",parent_ply[nbply]);
      TraceValue("%u",exclusive_chess_nr_continuations_reaching_goal[parent_ply[nbply]]);
      TraceEOL();

      if (exclusive_chess_nr_continuations_reaching_goal[parent_ply[nbply]]==1)
        /* look for one more */
        solve_result = MOVE_HAS_NOT_SOLVED_LENGTH();
    }
    else if (solve_result==MOVE_HAS_NOT_SOLVED_LENGTH())
    {
      ++nr_decidable_continuations_not_reaching_goal[parent_ply[nbply]];
      TraceText("remembering defined continuation");
      TraceValue("%u",nbply);
      TraceValue("%u",parent_ply[nbply]);
      TraceValue("%u",nr_decidable_continuations_not_reaching_goal[parent_ply[nbply]]);
      TraceEOL();
    }
  }
  else if (solve_result==MOVE_HAS_SOLVED_LENGTH())
    solve_result = MOVE_HAS_NOT_SOLVED_LENGTH();

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}
