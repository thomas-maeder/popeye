#include "conditions/mummer.h"
#include "conditions/singlebox/type3.h"
#include "solving/has_solution_type.h"
#include "stipulation/stipulation.h"
#include "stipulation/testing_pipe.h"
#include "stipulation/proxy.h"
#include "stipulation/boolean/true.h"
#include "stipulation/structure_traversal.h"
#include "stipulation/branch.h"
#include "stipulation/slice_insertion.h"
#include "stipulation/battle_play/branch.h"
#include "stipulation/help_play/branch.h"
#include "solving/machinery/slack_length.h"
#include "solving/temporary_hacks.h"
#include "solving/post_move_iteration.h"
#include "solving/observation.h"
#include "solving/move_generator.h"
#include "solving/conditional_pipe.h"
#include "position/move_diff_code.h"
#include "position/position.h"
#include "solving/pipe.h"
#include "solving/fork.h"
#include "debugging/trace.h"

#include "debugging/assert.h"
#include <stdlib.h>

/* mum length found so far */
static int mum_length[maxply+1];

/* index of last move with mum length */
static numecoup last_candidate[maxply+1];

static mummer_length_measurer_type mummer_measure_length[nr_sides];

mummer_strictness_type mummer_strictness[nr_sides];

mummer_strictness_type mummer_strictness_default_side;

/* Determine the length of a move for the Maximummer condition; the higher the
 * value the more likely the move is going to be played.
 * @return a value expressing the precedence of this move
 */
int maximummer_measure_length(void)
{
  square const sq_departure = move_generation_stack[CURRMOVE_OF_PLY(nbply)].departure;
  square const sq_arrival = move_generation_stack[CURRMOVE_OF_PLY(nbply)].arrival;
  square const sq_capture = move_generation_stack[CURRMOVE_OF_PLY(nbply)].capture;

  switch (sq_capture)
  {
    case messigny_exchange:
      return 0;

    case kingside_castling:
      return 16;

    case queenside_castling:
      return 25;

    case offset_platzwechsel_rochade:
      return 2 * move_diff_code[abs(sq_arrival-sq_departure)];

    default:
      switch (get_walk_of_piece_on_square(sq_departure))
      {
        case Mao:    /* special MAO move.*/
          return 6;

        case Moa:    /* special MOA move.*/
          return 6;

        default:
          if (sq_capture>offset_platzwechsel_rochade)
            return (move_diff_code[abs(sq_arrival-sq_departure)]) +
              (move_diff_code[abs((sq_capture-maxsquare)-(sq_departure+sq_arrival)/2)]);
          else
           return (move_diff_code[abs(sq_arrival-sq_departure)]);
      }
      break;
  }
}

/* Determine the length of a move for the Minimummer condition; the higher the
 * value the more likely the move is going to be played.
 * @return a value expressing the precedence of this move
 */
int minimummer_measure_length(void)
{
  return -maximummer_measure_length();
}

/* Forget previous mummer activations and definition of length measurers */
void mummer_reset_length_measurers(void)
{
  TraceFunctionEntry(__func__);
  TraceFunctionParamListEnd();

  mummer_measure_length[White] = 0;
  mummer_measure_length[Black] = 0;

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

/* Activate mummer for a side and define the length measurer to be used.
 * @param side Side for which to activate mummer
 * @param measurer length measurer to be used
 * @return false iff mummer was already activated for side
 */
boolean mummer_set_length_measurer(Side side,
                                   mummer_length_measurer_type measurer)
{
  boolean result;

  TraceFunctionEntry(__func__);
  TraceFunctionParamListEnd();

  if (mummer_measure_length[side]==0)
  {
    mummer_measure_length[side] = measurer;
    result = true;
  }
  else
    result = false;

  TraceFunctionExit(__func__);
  TraceFunctionResult("%u",result);
  TraceFunctionResultEnd();
  return result;
}

/* Reset the list of accepted moves
 */
static void reset_accepted_moves(ply ply)
{
  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",ply);
  TraceFunctionParamListEnd();

  last_candidate[ply] = CURRMOVE_OF_PLY(ply-1);
  TraceValue("%u",last_candidate[ply]);
  TraceEOL();

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

/* Accept a move for being played eventually
 */
static void accept_move(ply ply, numecoup id)
{
  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",ply);
  TraceFunctionParam("%u",id);
  TraceFunctionParamListEnd();

  ++last_candidate[ply];
  TraceValue("%u",last_candidate[ply]);
  TraceEOL();
  move_generation_stack[last_candidate[ply]] = move_generation_stack[id];

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
void mummer_orchestrator_solve(slice_index si)
{
  ply const save_nbply = nbply;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParamListEnd();

  mum_length[parent_ply[nbply]] = INT_MIN;
  reset_accepted_moves(nbply);

  copyply();
  move_generator_invert_move_order(nbply);
  fork_solve_delegate(si);
  finply();

  /* in some very obscure situations (cf. bug #142), we would continue with
   * e.g. knight promotion if queen promotion was played while measuring lengths
   */
  ++post_move_iteration_id[nbply];

  nbply = save_nbply;
  SET_CURRMOVE(nbply,last_candidate[nbply]);

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
void mummer_bookkeeper_solve(slice_index si)
{
  int current_length;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParamListEnd();

  current_length = (*mummer_measure_length[SLICE_STARTER(si)])();
  TraceValue("%d",current_length);
  TraceValue("%d",mum_length[parent_ply[nbply]]);
  TraceEOL();

  if (current_length<mum_length[parent_ply[nbply]])
  {
    /* this move will not be played */
  }
  else if (current_length==mum_length[parent_ply[nbply]])
    /* this move may be legal, but can't increase the maximum length */
    accept_move(nbply-1,CURRMOVE_OF_PLY(nbply));
  else
  {
    pipe_solve_delegate(si);

    if (solve_result>=slack_length)
    {
      /* we have a new mum */
      mum_length[parent_ply[nbply]] = current_length;
      TraceValue("%u",mum_length[parent_ply[nbply]]);
      TraceEOL();

      reset_accepted_moves(nbply-1);
      accept_move(nbply-1,CURRMOVE_OF_PLY(nbply));

      /* no need to try other flavours of the same move */
      post_move_iteration_locked[nbply] = false;
    }
  }

  solve_result = this_move_is_illegal;

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

typedef struct
{
    boolean ultra_capturing_king;
    Side current_side;
} instrumentation_state_type;

static void instrument_move_generator(slice_index si, stip_structure_traversal *st)
{
  instrumentation_state_type * const state = st->param;
  Side const save_current_side = state->current_side;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParamListEnd();

  if (mummer_measure_length[SLICE_STARTER(si)])
  {
    if (state->ultra_capturing_king)
    {
      slice_index const prototypes[] =
      {
          alloc_testing_pipe(STMummerOrchestrator),
          alloc_pipe(STUltraMummerMeasurerDeadend)
      };
      enum { nr_prototypes = sizeof prototypes / sizeof prototypes[0] };

      assert(st->activity==stip_traversal_activity_testing);
      assert(st->context==stip_traversal_context_help);

      slice_insertion_insert_contextually(si,st->context,prototypes,nr_prototypes);
    }
    else
    {
      slice_index const prototypes[] =
      {
          alloc_testing_pipe(STMummerOrchestrator),
          alloc_pipe(STMummerDeadend)
      };
      enum { nr_prototypes = sizeof prototypes / sizeof prototypes[0] };
      slice_insertion_insert_contextually(si,st->context,prototypes,nr_prototypes);
    }
  }

  state->current_side = SLICE_STARTER(si);
  stip_traverse_structure_children_pipe(si,st);
  state->current_side = save_current_side;

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

static stip_structure_visitor regular_deep_copy_end_of_branch_goal;

static void copy_end_of_branch_goal_if_necessary(slice_index si, stip_structure_traversal *st)
{
  stip_deep_copies_type * const copies = st->param;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParamListEnd();

  {
    slice_index const tester = branch_find_slice(STGoalReachedTester,SLICE_NEXT2(si),st->context);
    if (tester==no_slice
        /* avoid considering moves that lead to self-check illegal if they reach the goal: */
        || branch_find_slice(STSelfCheckGuard,SLICE_NEXT2(tester),st->context)==no_slice)
      regular_deep_copy_end_of_branch_goal(si,st);
    else
    {
      /* Rely on the tests in the goal reached tester: */
      (*copies)[si] = alloc_proxy_slice();
      stip_traverse_structure_children_pipe(si,st);
      pipe_link((*copies)[si],(*copies)[SLICE_NEXT1(si)]);
    }
  }

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

static void stop_copying(slice_index si, stip_structure_traversal *st)
{
  stip_deep_copies_type * const copies = st->param;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParamListEnd();

  (*copies)[si] = alloc_true_slice();

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

static void skip_copying(slice_index si, stip_structure_traversal *st)
{
  stip_deep_copies_type * const copies = st->param;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParamListEnd();

  (*copies)[si] = alloc_proxy_slice();
  stip_traverse_structure_children_pipe(si,st);
  pipe_link((*copies)[si],(*copies)[SLICE_NEXT1(si)]);

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

static void spin_off_measuring_branch(slice_index si, stip_structure_traversal *st)
{
  instrumentation_state_type const * const state = st->param;

  stip_deep_copies_type copies;
  stip_structure_traversal st_nested;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParamListEnd();

  if (SLICE_NEXT2(si)==no_slice)
  {
    SLICE_NEXT2(si) = alloc_proxy_slice();

    init_deep_copy(&st_nested,st,&copies);
    stip_structure_traversal_override_single(&st_nested,
                                             STMummerDeadend,
                                             &stop_copying);
    stip_structure_traversal_override_single(&st_nested,
                                             STRestartGuard,
                                             &skip_copying);
    stip_structure_traversal_override_single(&st_nested,
                                             STDeadEndGoal,
                                             &skip_copying);
    stip_structure_traversal_override_single(&st_nested,
                                             STGoalReachableGuardFilterMate,
                                             &skip_copying);
    stip_structure_traversal_override_single(&st_nested,
                                             STGoalReachableGuardFilterStalemate,
                                             &skip_copying);
    stip_structure_traversal_override_single(&st_nested,
                                             STGoalReachableGuardFilterProof,
                                             &skip_copying);
    stip_structure_traversal_override_single(&st_nested,
                                             STGoalReachableGuardFilterProofFairy,
                                             &skip_copying);
    stip_structure_traversal_override_by_function(&st_nested,
                                                  slice_function_move_removing_optimiser,
                                                  &skip_copying);
    if (mummer_strictness[state->current_side]!=mummer_strictness_regular)
    {
      stip_structure_traversal_override_single(&st_nested,
                                               STEndOfBranchGoal,
                                               &skip_copying);
      stip_structure_traversal_override_single(&st_nested,
                                               STSelfCheckGuard,
                                               &skip_copying);
    }
    else
    {
      regular_deep_copy_end_of_branch_goal = st_nested.map.visitors[STEndOfBranchGoal];
      stip_structure_traversal_override_single(&st_nested,
                                               STEndOfBranchGoal,
                                               &copy_end_of_branch_goal_if_necessary);
      stip_structure_traversal_override_single(&st_nested,
                                               STEndOfBranchGoalImmobile,
                                               &copy_end_of_branch_goal_if_necessary);
    }

    stip_traverse_structure(SLICE_NEXT1(si),&st_nested);

    link_to_branch(SLICE_NEXT2(si),copies[SLICE_NEXT1(si)]);

    {
      slice_index const prototype = alloc_pipe(STMummerBookkeeper);
      switch (st->context)
      {
        case stip_traversal_context_attack:
          attack_branch_insert_slices_behind_proxy(SLICE_NEXT2(si),&prototype,1,si);
          break;

        case stip_traversal_context_defense:
          defense_branch_insert_slices_behind_proxy(SLICE_NEXT2(si),&prototype,1,si);
          break;

        case stip_traversal_context_help:
          help_branch_insert_slices_behind_proxy(SLICE_NEXT2(si),&prototype,1,si);
          break;

        default:
          assert(0);
          break;
      }
    }

    if (mummer_strictness[state->current_side]!=mummer_strictness_regular)
    {
      slice_index const prototype = alloc_pipe(STUltraMummerMeasurerDeadend);
      slice_insertion_insert_contextually(copies[SLICE_NEXT1(si)],st->context,&prototype,1);
    }
  }

  stip_traverse_structure_children(si,st);

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

static void instrument_ultra_mummer_measurer_fork(slice_index si,
                                                  stip_structure_traversal *st)
{
  instrumentation_state_type * const state = st->param;
  boolean const save_ultra_mummer_capturing_king = state->ultra_capturing_king;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParamListEnd();

  stip_traverse_structure_children_pipe(si,st);

  if (mummer_strictness[SLICE_STARTER(si)]==mummer_strictness_ultra)
  {
    state->ultra_capturing_king = true;
    stip_traverse_structure_conditional_pipe_tester(si,st);
    state->ultra_capturing_king = save_ultra_mummer_capturing_king;
  }

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

static void connect_solver_to_tester(slice_index si, stip_structure_traversal *st)
{
  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParamListEnd();

  if (st->activity==stip_traversal_activity_solving)
  {
    assert(SLICE_TESTER(SLICE_PREV(si))!=no_slice);
    SLICE_TESTER(si) = SLICE_NEXT1(SLICE_TESTER(SLICE_PREV(si)));
  }

  stip_traverse_structure_children_pipe(si,st);

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
void ultra_mummer_measurer_deadend_solve(slice_index si)
{
  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParamListEnd();

  solve_result = MOVE_HAS_SOLVED_LENGTH();
  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

/* Validate an observation according to Ultra-Mummer
 * @return true iff the observation is valid
 */
boolean ultra_mummer_validate_observation(slice_index si)
{
  Side const side_observing = trait[nbply];
  boolean result;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParamListEnd();

  conditional_pipe_solve_delegate(temporary_hack_ultra_mummer_length_measurer[side_observing]);

  result = (*mummer_measure_length[side_observing])()==mum_length[nbply];

  if (result)
    result = pipe_validate_observation_recursive_delegate(si);

  TraceFunctionExit(__func__);
  TraceFunctionResult("%u",result);
  TraceFunctionResultEnd();
  return result;
}

/* Instrument a stipulation
 * @param si identifies root slice of stipulation
 */
void mummer_initialise_solving(slice_index si)
{
  instrumentation_state_type state = { false, no_side };
  stip_structure_traversal st;

  TraceFunctionEntry(__func__);
  TraceFunctionParamListEnd();

  TraceStipulation(si);

  solving_impose_starter(si,SLICE_STARTER(si));

  stip_structure_traversal_init(&st,&state);

  /* instrumentation for regular mummer */
  stip_structure_traversal_override_single(&st,
                                           STMoveGenerator,
                                           &instrument_move_generator);
  stip_structure_traversal_override_single(&st,
                                           STMummerOrchestrator,
                                           &spin_off_measuring_branch);
  stip_structure_traversal_override_single(&st,
                                           STMummerDeadend,
                                           &connect_solver_to_tester);
  stip_structure_traversal_override_single(&st,
                                           STExecutingKingCapture,
                                           &stip_structure_visitor_noop);

  if (mummer_strictness[White]!=mummer_strictness_ultra
      && mummer_strictness[Black]!=mummer_strictness_ultra)
    stip_structure_traversal_override_single(&st,
                                             STImmobilityTester,
                                             &stip_structure_visitor_noop);

  /* additional instrumentation for ultra mummer */
  stip_structure_traversal_override_single(&st,
                                           STUltraMummerMeasurerFork,
                                           &instrument_ultra_mummer_measurer_fork);

  stip_traverse_structure(si,&st);

  if (mummer_strictness[White]==mummer_strictness_ultra)
  {
    stip_instrument_observation_validation(si,White,STValidatingObservationUltraMummer);
    stip_instrument_check_validation(si,White,STValidatingObservationUltraMummer);
  }

  if (mummer_strictness[Black]==mummer_strictness_ultra)
  {
    stip_instrument_observation_validation(si,Black,STValidatingObservationUltraMummer);
    stip_instrument_check_validation(si,Black,STValidatingObservationUltraMummer);
  }

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}
