#include "options/maxsolutions/maxsolutions.h"
#include "solving/has_solution_type.h"
#include "solving/pipe.h"
#include "stipulation/slice_insertion.h"
#include "stipulation/battle_play/branch.h"
#include "stipulation/help_play/branch.h"
#include "options/maxsolutions/initialiser.h"
#include "options/maxsolutions/guard.h"
#include "options/maxsolutions/guard.h"
#include "options/maxsolutions/guard.h"
#include "options/maxsolutions/guard.h"
#include "options/interruption.h"
#include "debugging/trace.h"
#include "debugging/assert.h"

#include <limits.h>
#include <stdlib.h>

/* number of solutions found in the current phase */
static unsigned int nr_solutions_found_in_phase;

/* maximum number of allowed solutions found in the current phase */
static unsigned int max_nr_solutions_per_phase = UINT_MAX;

/* has the maximum number of allowed solutions been reached? */
static boolean allowed_nr_solutions_reached;

/* Reset the value of the maxsolutions option
 */
void maxsolutions_resetter_solve(slice_index si)
{
  TraceFunctionEntry(__func__);
  TraceFunctionParamListEnd();

  max_nr_solutions_per_phase = UINT_MAX;
  allowed_nr_solutions_reached = false;

  pipe_solve_delegate(si);

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

/* Propagage our findings to STOptionInterruption
 * @param si identifies the slice where to start instrumenting
 */
void maxsolutions_propagator_solve(slice_index si)
{
  TraceFunctionEntry(__func__);
  TraceFunctionParamListEnd();

  pipe_solve_delegate(si);

  if (max_solutions_reached())
    option_interruption_remember(SLICE_NEXT2(si))

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

/* Instrument the solving machinery with option maxsolutions
 * @param si identifies the slice where to start instrumenting
 * @param max_nr_solutions_per_phase
 */
void maxsolutions_instrument_solving(slice_index si, unsigned int i)
{
  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",interruption);
  TraceFunctionParamListEnd();

  {
    slice_index const interruption = branch_find_slice(STOptionInterruption,
                                                       si,
                                                       stip_traversal_context_intro);
    slice_index const prototype = alloc_pipe(STMaxSolutionsPropagator);
    SLICE_NEXT2(prototype) = interruption;
    assert(interruption!=no_slice);
    slice_insertion_insert(si,&prototype,1);
    max_nr_solutions_per_phase = i;
  }

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

/* Have we found the maxmimum allowed number of solutions since the
 * last invokation of reset_max_solutions()/read_max_solutions()?
 * @true iff we have found the maxmimum allowed number of solutions
 */
boolean max_solutions_reached(void)
{
  TraceFunctionEntry(__func__);
  TraceFunctionParamListEnd();

  TraceFunctionExit(__func__);
  TraceFunctionResult("%u",allowed_nr_solutions_reached);
  TraceFunctionResultEnd();
  return allowed_nr_solutions_reached;
}

/* Reset the number of found solutions
 */
void reset_nr_found_solutions_per_phase(void)
{
  TraceFunctionEntry(__func__);
  TraceFunctionParamListEnd();

  nr_solutions_found_in_phase = 0;

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

/* Increase the number of found solutions by 1
 */
void increase_nr_found_solutions(void)
{
  TraceFunctionEntry(__func__);
  TraceFunctionParamListEnd();

  ++nr_solutions_found_in_phase;
  TraceValue("->%u\n",nr_solutions_found_in_phase);

  if (max_nr_solutions_found_in_phase())
    allowed_nr_solutions_reached = true;

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

/* Have we found the maximum allowed number of solutions since the
 * last invokation of reset_nr_found_solutions()?
 * @return true iff the allowed maximum number of solutions have been found
 */
boolean max_nr_solutions_found_in_phase(void)
{
  boolean result;

  TraceFunctionEntry(__func__);
  TraceFunctionParamListEnd();

  result = nr_solutions_found_in_phase>=max_nr_solutions_per_phase;

  TraceFunctionExit(__func__);
  TraceFunctionResult("%u",result);
  TraceFunctionResultEnd();
  return result;
}

static void insert_maxsolutions_help_filter(slice_index si,
                                            stip_structure_traversal *st)
{
  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParamListEnd();

  stip_traverse_structure_children_pipe(si,st);

  {
    slice_index const prototype = alloc_maxsolutions_guard_slice();
    help_branch_insert_slices(si,&prototype,1);
  }

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

static void insert_guard_and_counter(slice_index si, stip_structure_traversal *st)
{
  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParamListEnd();

  {
    slice_index const prototypes[] =
    {
      alloc_maxsolutions_guard_slice(),
      alloc_maxsolutions_counter_slice()
    };
    enum { nr_prototypes = sizeof prototypes / sizeof prototypes[0] };
    slice_insertion_insert(si,prototypes,nr_prototypes);
  }

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

static void insert_counter(slice_index si, stip_structure_traversal *st)
{
  boolean * const inserted = st->param;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParamListEnd();

  stip_traverse_structure_children(si,st);

  if (st->context==stip_traversal_context_help && !*inserted)
  {
    slice_index const prototype = alloc_maxsolutions_counter_slice();
    slice_insertion_insert(SLICE_NEXT2(si),&prototype,1);
    *inserted = true;
  }

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

static void insert_counter_at_goal(slice_index si, stip_structure_traversal *st)
{
  boolean * const inserted = st->param;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParamListEnd();

  stip_traverse_structure_children(si,st);

  if (st->context==stip_traversal_context_help && !*inserted)
  {
    slice_index const prototype = alloc_maxsolutions_counter_slice();
    help_branch_insert_slices_behind_proxy(SLICE_NEXT2(si),&prototype,1,si);
    *inserted = true;
  }

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

static void insert_counter_in_forced_branch(slice_index si, stip_structure_traversal *st)
{
  boolean * const inserted = st->param;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParamListEnd();

  stip_traverse_structure_children_pipe(si,st);

  if (st->context==stip_traversal_context_help && !*inserted)
  {
    slice_index const prototype = alloc_maxsolutions_counter_slice();
    slice_insertion_insert(SLICE_NEXT2(si),&prototype,1);
    *inserted = true;
  }

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

static structure_traversers_visitor maxsolutions_filter_inserters[] =
{
  { STMaxSolutionsGuard,       &stip_structure_visitor_noop     },
  { STReadyForHelpMove,        &insert_maxsolutions_help_filter },
  { STAttackAdapter,           &insert_guard_and_counter        },
  { STDefenseAdapter,          &stip_structure_visitor_noop     },
  { STEndOfBranch,             &insert_counter                  },
  { STEndOfBranchGoal,         &insert_counter_at_goal          },
  { STEndOfBranchGoalImmobile, &insert_counter_at_goal          },
  { STEndOfBranchForced,       &insert_counter_in_forced_branch }
};

enum
{
  nr_maxsolutions_filter_inserters =
  (sizeof maxsolutions_filter_inserters
   / sizeof maxsolutions_filter_inserters[0])
};

/* Instrument a stipulation with STMaxSolutions*Filter slices
 * @param si identifies slice where to start
 */
void solving_insert_maxsolutions_filters(slice_index si)
{
  stip_structure_traversal st;
  boolean inserted = false;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParamListEnd();

  TraceStipulation(si);

  {
    slice_index const prototypes[] =
    {
      alloc_maxsolutions_initialiser_slice()
    };
    enum
    {
      nr_prototypes = sizeof prototypes / sizeof prototypes[0]
    };
    slice_insertion_insert(si,prototypes,nr_prototypes);
  }

  stip_structure_traversal_init(&st,&inserted);
  stip_structure_traversal_override_by_contextual(&st,
                                                  slice_contextual_conditional_pipe,
                                                  &stip_traverse_structure_children_pipe);
  stip_structure_traversal_override(&st,
                                    maxsolutions_filter_inserters,
                                    nr_maxsolutions_filter_inserters);
  stip_traverse_structure(si,&st);

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}
