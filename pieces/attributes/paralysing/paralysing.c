#include "pieces/attributes/paralysing/paralysing.h"
#include "pydata.h"
#include "stipulation/proxy.h"
#include "stipulation/branch.h"
#include "stipulation/boolean/or.h"
#include "pieces/attributes/paralysing/mate_filter.h"
#include "pieces/attributes/paralysing/stalemate_special.h"
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
 boolean encore_with_paralysis;
 boolean encore_without_paralysis;

 TraceFunctionEntry(__func__);
 TraceEnumerator(Side,side,"");
 TraceFunctionParamListEnd();

 move_generation_mode = move_generation_not_optimized;

 paralysis_suspended = true;
 genmove(side);
 encore_without_paralysis = encore();
 finply();
 paralysis_suspended = false;

 genmove(side);
 encore_with_paralysis = encore();
 finply();

 result = !encore_with_paralysis && encore_without_paralysis;

 TraceFunctionExit(__func__);
 TraceFunctionResult("%u",result);
 TraceFunctionResultEnd();
 return result;
}

/* Determine whether a piece is paralysed
 * @param s position of piece
 * @return true iff the piece on square s is paralysed
 */
boolean paralysiert(square s)
{
  boolean result;

  TraceFunctionEntry(__func__);
  TraceSquare(s);
  TraceFunctionParamListEnd();

  if (paralysis_suspended)
    result = false;
  else if (e[s]>obs)
  {
    square const roi = king_square[White];
    king_square[White] = s;
    result = rbechec(nbply,testparalyse);
    king_square[White] = roi;
  }
  else {
    square const roi = king_square[Black];
    king_square[Black] = s;
    result = rnechec(nbply,testparalyse);
    king_square[Black] = roi;
  }

  TraceFunctionExit(__func__);
  TraceFunctionResult("%u",result);
  TraceFunctionResultEnd();
  return result;
}

static void instrument_mate(slice_index si, stip_structure_traversal *st)
{
  boolean const * const testing = st->param;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParamListEnd();

  stip_traverse_structure_children(si,st);

  if (*testing)
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

  stip_traverse_structure_children(si,st);

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

  stip_traverse_structure_children(si,st);

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
  boolean * const testing = st->param;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParamListEnd();

  {
    stip_structure_traversal st_nested;
    stip_structure_traversal_init_nested(&st_nested,st,testing);
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
  boolean const * const testing = st->param;
  goal_applies_to_starter_or_adversary const who = slices[si].u.goal_filter.applies_to_who;
  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParamListEnd();

  stip_traverse_structure_children(si,st);

  if (*testing)
    pipe_append(slices[si].prev,alloc_paralysing_mate_filter_tester_slice(who));
  else
    pipe_append(slices[si].prev,alloc_paralysing_mate_filter_slice(who));

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

static void instrument_doublemate(slice_index si, stip_structure_traversal *st)
{
  boolean * const testing = st->param;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParamListEnd();

  {
    stip_structure_traversal st_nested;
    stip_structure_traversal_init_nested(&st_nested,st,testing);
    stip_structure_traversal_override_single(&st_nested,
                                             STGoalCheckReachedTester,
                                             &instrument_half_doublemate);
    stip_traverse_structure(si,&st_nested);
  }

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

static void remember_testing_testing_pipe(slice_index si, stip_structure_traversal *st)
{
  boolean * const testing = st->param;
  boolean const save_testing = *testing;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParamListEnd();

  stip_traverse_structure(slices[si].next2,st);

  *testing = true;
  stip_traverse_structure_next_branch(si,st);
  *testing = save_testing;

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

static void remember_testing_conditional_pipe(slice_index si, stip_structure_traversal *st)
{
  boolean * const testing = st->param;
  boolean const save_testing = *testing;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParamListEnd();

  stip_traverse_structure_children_pipe(si,st);

  *testing = true;
  stip_traverse_structure_next_branch(si,st);
  *testing = save_testing;

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

static structure_traversers_visitors goal_filter_inserters[] =
{
  { STGoalMateReachedTester,            &instrument_mate            },
  { STGoalStalemateReachedTester,       &instrument_stalemate       },
  { STGoalAutoStalemateReachedTester,   &instrument_autostalemate   },
  { STGoalDoubleStalemateReachedTester, &instrument_doublestalemate },
  { STGoalDoubleMateReachedTester,      &instrument_doublemate      },
  { STGoalCounterMateReachedTester,     &instrument_doublemate      }
};

enum
{
  nr_goal_filter_inserters = (sizeof goal_filter_inserters
                              / sizeof goal_filter_inserters[0])
};

/* Instrument a stipulation with goal filter slices
 * @param si root of branch to be instrumented
 */
void stip_insert_paralysing_goal_filters(slice_index si)
{
  stip_structure_traversal st;
  boolean testing = false;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParamListEnd();

  TraceStipulation(si);

  stip_structure_traversal_init(&st,&testing);
  stip_structure_traversal_override_by_function(&st,
                                                slice_function_conditional_pipe,
                                                &remember_testing_conditional_pipe);
  stip_structure_traversal_override_by_function(&st,
                                                slice_function_testing_pipe,
                                                &remember_testing_testing_pipe);
  stip_structure_traversal_override(&st,
                                    goal_filter_inserters,
                                    nr_goal_filter_inserters);
  stip_traverse_structure(si,&st);

  TraceStipulation(si);

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}
