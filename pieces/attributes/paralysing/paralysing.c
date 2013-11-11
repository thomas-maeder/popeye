#include "pieces/attributes/paralysing/paralysing.h"
#include "pieces/attributes/paralysing/mate_filter.h"
#include "pieces/attributes/paralysing/stalemate_special.h"
#include "solving/observation.h"
#include "solving/move_generator.h"
#include "stipulation/proxy.h"
#include "stipulation/branch.h"
#include "stipulation/boolean/or.h"
#include "stipulation/has_solution_type.h"
#include "stipulation/temporary_hacks.h"
#include "debugging/trace.h"
#include "pieces/pieces.h"

#include <assert.h>

/* Allow paralysis by paralysing pieces to be temporarily suspended
 */
static boolean paralysis_suspended = false;

static boolean validating_paralysis_observation_geometry = false;

/* Validate an observation geometry according to Paralysing pieces
 * @return true iff the observation is valid
 */
boolean paralysing_validate_observation_geometry(slice_index si)
{
  boolean result;

  TraceFunctionEntry(__func__);
  TraceFunctionParamListEnd();

  result = validate_observation_recursive(slices[si].next1);

  if (result && validating_paralysis_observation_geometry)
    result = TSTFLAG(spec[move_generation_stack[CURRMOVE_OF_PLY(nbply)].departure],Paralysing);

  TraceFunctionExit(__func__);
  TraceFunctionResult("%u",result);
  TraceFunctionResultEnd();
  return result;
}

static boolean is_paralysed(numecoup n)
{
  boolean result;

  TraceFunctionEntry(__func__);
  TraceFunctionParamListEnd();

  if (paralysis_suspended)
    result = false;
  else
  {
    assert(!validating_paralysis_observation_geometry);
    siblingply(advers(trait[nbply]));
    ++current_move[nbply];
    move_generation_stack[CURRMOVE_OF_PLY(nbply)].capture = move_generation_stack[n].departure;
    validating_paralysis_observation_geometry = true;
    result = is_square_observed(&validate_observation_geometry);
    validating_paralysis_observation_geometry = false;
    finply();
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
stip_length_type paralysing_suffocation_finder_solve(slice_index si,
                                                     stip_length_type n)
{
  stip_length_type result;
  numecoup curr;
  square sq_departure = initsquare;
  boolean found_move_from_unparalysed = false;
  boolean found_move_from_paralysed = false;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParam("%u",n);
  TraceFunctionParamListEnd();

  paralysis_suspended = false;

  for (curr = MOVEBASE_OF_PLY(nbply)+1; curr<=CURRMOVE_OF_PLY(nbply); ++curr)
    if (move_generation_stack[curr].departure!=sq_departure)
    {
      sq_departure = move_generation_stack[curr].departure;
      if (is_paralysed(curr))
        found_move_from_paralysed = true;
      else
      {
        found_move_from_unparalysed = true;
        break;
      }
    }

  if (found_move_from_unparalysed)
    result = next_move_has_no_solution;
  else if (found_move_from_paralysed)
    result = next_move_has_solution;
  else
    result = next_move_has_no_solution;

  paralysis_suspended = true;

  TraceFunctionExit(__func__);
  TraceEnumerator(has_solution_type,result,"");
  TraceFunctionResultEnd();
  return result;
}

/* Determine whether a side is "suffocated by paralysis", i.e. would the side
 * have moves (possibly exposing the side to self check) if no piece were
 * paralysing.
 * @param side side for which to determine whether it is suffocated
 * @return true iff side is suffocated by paralysis
 */
boolean suffocated_by_paralysis(Side side)
{
  boolean result;

  TraceFunctionEntry(__func__);
  TraceEnumerator(Side,side,"");
  TraceFunctionParamListEnd();

  paralysis_suspended = true;

  result = (solve(slices[temporary_hack_suffocation_by_paralysis_finder[side]].next2,
                  length_unspecified)
            ==next_move_has_solution);

  paralysis_suspended = false;

  TraceFunctionExit(__func__);
  TraceFunctionResult("%u",result);
  TraceFunctionResultEnd();
  return result;
}

/* Validate an observer according to Paralysing pieces
 * @return true iff the observation is valid
 */
boolean paralysing_validate_observer(slice_index si)
{
  boolean result;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParamListEnd();

  /* we are not validating a paralysis, but an observation (e.g. check or
   * Patrol Chess) in the presence of paralysing pieces */
  result = (!TSTFLAG(spec[move_generation_stack[CURRMOVE_OF_PLY(nbply)].departure],
                     Paralysing)
            && !is_paralysed(CURRMOVE_OF_PLY(nbply))
            &&  validate_observation_recursive(slices[si].next1));

  TraceFunctionExit(__func__);
  TraceFunctionResult("%u",result);
  TraceFunctionResultEnd();
  return result;
}

/* Generate moves for a single piece
 * @param identifies generator slice
 * @param p walk to be used for generating
 */
void paralysing_generate_moves_for_piece(slice_index si, PieNam p)
{
  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TracePiece(p);
  TraceFunctionParamListEnd();

  if (!is_paralysed(current_generation))
    generate_moves_for_piece(slices[si].next1,p);

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

static void instrument_mate(slice_index si, stip_structure_traversal *st)
{
  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParamListEnd();

  stip_traverse_structure_children_pipe(si,st);

  if (st->activity==stip_traversal_activity_testing)
    pipe_append(slices[si].prev,alloc_paralysing_mate_filter_tester_slice(goal_applies_to_starter));
  else
    pipe_append(slices[si].prev,alloc_paralysing_mate_filter_slice(goal_applies_to_starter));

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

static void instrument_stalemate(slice_index si,
                                 stip_structure_traversal *st)
{
  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParamListEnd();

  stip_traverse_structure_children_pipe(si,st);

  pipe_append(si,alloc_paralysing_stalemate_special_slice(goal_applies_to_starter));

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

static void instrument_autostalemate(slice_index si,
                                     stip_structure_traversal *st)
{
  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParamListEnd();

  stip_traverse_structure_children_pipe(si,st);

  pipe_append(si,alloc_paralysing_stalemate_special_slice(goal_applies_to_adversary));

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

static void prepend_stalemate_special_starter(slice_index si,
                                              stip_structure_traversal *st)
{
  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParamListEnd();

  pipe_append(slices[si].prev,
              alloc_paralysing_stalemate_special_slice(goal_applies_to_starter));

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

static void prepend_stalemate_special_other(slice_index si,
                                            stip_structure_traversal *st)
{
  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParamListEnd();

  pipe_append(slices[si].prev,
              alloc_paralysing_stalemate_special_slice(goal_applies_to_adversary));

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

static void instrument_doublestalemate(slice_index si,
                                       stip_structure_traversal *st)
{
  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParamListEnd();

  {
    stip_structure_traversal st_nested;
    stip_structure_traversal_init_nested(&st_nested,st,0);
    stip_structure_traversal_override_single(&st_nested,
                                             STGoalNotCheckReachedTester,
                                             &prepend_stalemate_special_starter);
    stip_structure_traversal_override_single(&st_nested,
                                             STGoalImmobileReachedTester,
                                             &prepend_stalemate_special_other);
    stip_traverse_structure(si,&st_nested);
  }

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

static void instrument_half_doublemate(slice_index si,
                                       stip_structure_traversal *st)
{
  goal_applies_to_starter_or_adversary const who = slices[si].u.goal_filter.applies_to_who;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParamListEnd();

  stip_traverse_structure_children_pipe(si,st);

  if (st->activity==stip_traversal_activity_testing)
    pipe_append(slices[si].prev,alloc_paralysing_mate_filter_tester_slice(who));
  else
    pipe_append(slices[si].prev,alloc_paralysing_mate_filter_slice(who));

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

static void instrument_doublemate(slice_index si, stip_structure_traversal *st)
{
  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParamListEnd();

  {
    stip_structure_traversal st_nested;
    stip_structure_traversal_init_nested(&st_nested,st,0);
    stip_structure_traversal_override_single(&st_nested,
                                             STGoalCheckReachedTester,
                                             &instrument_half_doublemate);
    stip_traverse_structure(si,&st_nested);
  }

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

static void insert_captures_remover(slice_index si,
                                    stip_structure_traversal *st)
{
  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParamListEnd();

  stip_traverse_structure_children(si,st);

  {
    slice_index const prototype = alloc_pipe(STPiecesParalysingRemoveCaptures);
    branch_insert_slices_contextual(si,st->context,&prototype,1);
  }

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

static structure_traversers_visitor goal_filter_inserters[] =
{
  { STGoalMateReachedTester,            &instrument_mate            },
  { STGoalStalemateReachedTester,       &instrument_stalemate       },
  { STGoalAutoStalemateReachedTester,   &instrument_autostalemate   },
  { STGoalDoubleStalemateReachedTester, &instrument_doublestalemate },
  { STGoalDoubleMateReachedTester,      &instrument_doublemate      },
  { STGoalCounterMateReachedTester,     &instrument_doublemate      },
  { STDoneGeneratingMoves,              &insert_captures_remover    }
};

enum
{
  nr_goal_filter_inserters = (sizeof goal_filter_inserters
                              / sizeof goal_filter_inserters[0])
};

/* Initialise solving with paralysing pieces
 * @param si root of branch to be instrumented
 */
void paralysing_initialise_solving(slice_index si)
{
  stip_structure_traversal st;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParamListEnd();

  TraceStipulation(si);

  stip_structure_traversal_init(&st,0);
  stip_structure_traversal_override(&st,
                                    goal_filter_inserters,
                                    nr_goal_filter_inserters);
  stip_traverse_structure(si,&st);

  TraceStipulation(si);

  solving_instrument_move_generation(si,nr_sides,STParalysingMovesForPieceGenerator);

  stip_instrument_observer_validation(si,nr_sides,STParalysingObserverValidator);
  stip_instrument_observation_geometry_validation(si,nr_sides,STParalysingObservationGeometryValidator);

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}
