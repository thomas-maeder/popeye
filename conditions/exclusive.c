#include "conditions/exclusive.h"
#include "stipulation/stipulation.h"
#include "optimisations/goals/mate/neutralretractable.h"
#include "pymsg.h"
#include "pydata.h"
#include "stipulation/pipe.h"
#include "stipulation/has_solution_type.h"
#include "stipulation/temporary_hacks.h"
#include "stipulation/branch.h"
#include "solving/legal_move_counter.h"
#include "debugging/trace.h"

#include <assert.h>

static Goal exclusive_goal;

static boolean is_reaching_goal_allowed[maxply+1];

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
                                           STExclusiveChessMatingMoveCounter,
                                           &remove_guard);
  stip_traverse_structure(si,&st);

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

static void avoid_instrumenting_exclusivity_detecting_move(slice_index si,
                                                           stip_structure_traversal *st)
{
  boolean * const is_this_mating_move_played_for_testing_exclusivity = st->param;
  boolean const save_is_this_mating_move_played_for_testing_exclusivity = *is_this_mating_move_played_for_testing_exclusivity;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParamListEnd();

  stip_traverse_structure_children_pipe(si,st);

  *is_this_mating_move_played_for_testing_exclusivity = true;
  stip_traverse_structure_conditional_pipe_tester(si,st);
  *is_this_mating_move_played_for_testing_exclusivity = save_is_this_mating_move_played_for_testing_exclusivity;

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

static void insert_exclusivity_detector(slice_index si,
                                        stip_structure_traversal *st)
{
  boolean const * const is_this_mating_move_played_for_testing_exclusivity = st->param;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParamListEnd();

  stip_traverse_structure_children_pipe(si,st);

  if (!*is_this_mating_move_played_for_testing_exclusivity)
  {
    slice_index const prototype = alloc_pipe(STExclusiveChessExclusivityDetector);
    branch_insert_slices_contextual(si,st->context,&prototype,1);
  }

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

static void insert_legality_tester(slice_index si,
                                   stip_structure_traversal *st)
{
  boolean * const is_this_mating_move_played_for_testing_exclusivity = st->param;
  boolean const save_is_this_mating_move_played_for_testing_exclusivity = *is_this_mating_move_played_for_testing_exclusivity;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParamListEnd();

  *is_this_mating_move_played_for_testing_exclusivity = false;
  stip_traverse_structure_children_pipe(si,st);
  *is_this_mating_move_played_for_testing_exclusivity = save_is_this_mating_move_played_for_testing_exclusivity;

  if (!*is_this_mating_move_played_for_testing_exclusivity)
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
  boolean is_this_mating_move_played_for_testing_exclusivity = false;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParamListEnd();

  stip_structure_traversal_init(&st,&is_this_mating_move_played_for_testing_exclusivity);
  stip_structure_traversal_override_single(&st,
                                           STExclusiveChessMatingMoveCounter,
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

  result = (!is_reaching_goal_allowed[parent_ply[nbply]]
            && solve(slices[temporary_hack_mate_tester[advers(trait[nbply])]].next2,slack_length)!=slack_length+2);

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

  if (is_exclusivity_violated())
    result = previous_move_is_illegal;
  else
    result = solve(next,n);

  TraceFunctionExit(__func__);
  TraceFunctionResult("%u",result);
  TraceFunctionResultEnd();
  return result;
}

static void detect_exclusivity(Side side)
{
#if defined(DOTRACE)
  /* empirically determined at 1 workstation */
  ply const stop_at_ply = 250;
#else
  ply const stop_at_ply = maxply;
#endif

  TraceFunctionEntry(__func__);
  TraceEnumerator(Side,side,"");
  TraceFunctionParamListEnd();

  assert(exclusive_goal.type==goal_mate);

  if (nbply>stop_at_ply)
    FtlMsg(ChecklessUndecidable);

  /* avoid concurrent counts */
  assert(legal_move_counter_count[nbply]==0);

  /* stop counting once we have found >1 mating moves */
  legal_move_counter_interesting[nbply] = 1;

  solve(slices[temporary_hack_exclusive_mating_move_counter[side]].next2,length_unspecified);

  is_reaching_goal_allowed[nbply] = legal_move_counter_count[nbply]<2;
  TraceValue("%u",legal_move_counter_count[nbply]);
  TraceValue("%u\n",is_reaching_goal_allowed[nbply]);

  /* clean up after ourselves */
  legal_move_counter_count[nbply] = 0;

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

  detect_exclusivity(slices[si].starter);

  result = solve(slices[si].next1,n);

  TraceFunctionExit(__func__);
  TraceFunctionResult("%u",result);
  TraceFunctionResultEnd();
  return result;
}
