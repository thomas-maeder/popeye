#include "pieces/attributes/paralysing/paralysing.h"
#include "pydata.h"
#include "stipulation/proxy.h"
#include "stipulation/branch.h"
#include "stipulation/boolean/or.h"
#include "pieces/attributes/paralysing/mate_filter.h"
#include "pieces/attributes/paralysing/stalemate_special.h"
#include "solving/observation.h"
#include "debugging/trace.h"

#include <assert.h>

/* Allow paralysis by paralysing pieces to be temporarily suspended
 */
static boolean paralysis_suspended = false;

/* Determine whether a side is "suffocated by paralysis", i.e. would the side
 * have moves (possibly exposing the side to self check) if no piece were
 * paralysing.
 * @param side side for which to determine whether it is suffocated
 * @return true iff side is suffocated by paralysis
 */
boolean suffocated_by_paralysis(Side side)
{
  boolean result;
  boolean found_move_from_unparalysed = false;
  boolean found_move_from_paralysed = false;

  TraceFunctionEntry(__func__);
  TraceEnumerator(Side,side,"");
  TraceFunctionParamListEnd();

  nextply();
  paralysis_suspended = true;
  genmove(side);
  paralysis_suspended = false;

  {
    numecoup curr;
    for (curr = current_move[nbply-1]+1; curr<=current_move[nbply];)
    {
      square const sq_departure = move_generation_stack[curr].departure;
      if (is_piece_paralysed_on(sq_departure))
      {
        found_move_from_paralysed = true;

        do
        {
          ++curr;
        } while (move_generation_stack[curr].departure==sq_departure);
      }
      else
      {
        found_move_from_unparalysed = true;
        break;
      }
    }
  }

  finply();

  if (found_move_from_unparalysed)
    result = false;
  else if (found_move_from_paralysed)
    result = true;
  else
    result = false;

  TraceFunctionExit(__func__);
  TraceFunctionResult("%u",result);
  TraceFunctionResultEnd();
  return result;
}

/* Validate an observer according to paralysing pieces
 * @param sq_observer position of the observer
 * @param sq_landing landing square of the observer (normally==sq_observee)
 * @param sq_observee position of the piece to be observed
 * @return true iff the observation is valid
 */
static boolean paralysing_validate_observer(square sq_observer,
                                            square sq_landing,
                                            square sq_observee)
{
  boolean result;

  TraceFunctionEntry(__func__);
  TraceSquare(sq_observer);
  TraceSquare(sq_landing);
  TraceSquare(sq_observee);
  TraceFunctionParamListEnd();

  if (TSTFLAG(spec[sq_observer],Paralysing))
    result = false;
  else
    result = validate_observation_geometry(sq_observer,sq_landing,sq_observee);

  TraceFunctionExit(__func__);
  TraceFunctionResult("%u",result);
  TraceFunctionResultEnd();
  return result;
}

/* Validate an observation according to paralysing pieces
 * @param sq_observer position of the observer
 * @param sq_landing landing square of the observer (normally==sq_observee)
 * @param sq_observee position of the piece to be observed
 * @return true iff the observation is valid
 */
static boolean paralysing_validate_observation(square sq_observer,
                                               square sq_landing,
                                               square sq_observee)
{
  boolean result;

  TraceFunctionEntry(__func__);
  TraceSquare(sq_observer);
  TraceSquare(sq_landing);
  TraceSquare(sq_observee);
  TraceFunctionParamListEnd();

  if (TSTFLAG(spec[sq_observer],Paralysing))
    result = false;
  else
    result = !is_piece_paralysed_on(sq_observer);

  TraceFunctionExit(__func__);
  TraceFunctionResult("%u",result);
  TraceFunctionResultEnd();
  return result;
}

static boolean validate_paralyser(square sq_paralyser,
                                  square sq_landing,
                                  square sq_paralysee)
{
  boolean result;

  TraceFunctionEntry(__func__);
  TraceSquare(sq_paralyser);
  TraceSquare(sq_landing);
  TraceSquare(sq_paralysee);
  TraceFunctionParamListEnd();

  result = (TSTFLAG(spec[sq_paralyser],Paralysing)
            && validate_observation_geometry(sq_paralyser,sq_landing,sq_paralysee));

  TraceFunctionExit(__func__);
  TraceFunctionResult("%u",result);
  TraceFunctionResultEnd();
  return result;
}

/* Determine whether a piece is paralysed
 * @param s position of piece
 * @return true iff the piece on square s is paralysed
 */
boolean is_piece_paralysed_on(square s)
{
  boolean result;

  TraceFunctionEntry(__func__);
  TraceSquare(s);
  TraceFunctionParamListEnd();

  if (paralysis_suspended)
    result = false;
  else
  {
    nextply();
    trait[nbply] = advers(trait[parent_ply[nbply]]);

    result = is_square_attacked(s,&validate_paralyser);

    finply();
  }

  TraceFunctionExit(__func__);
  TraceFunctionResult("%u",result);
  TraceFunctionResultEnd();
  return result;
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

  register_observer_validator(&paralysing_validate_observer);
  register_observation_validator(&paralysing_validate_observation);

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}
