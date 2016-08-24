#include "options/maxsolutions/maxsolutions.h"
#include "solving/has_solution_type.h"
#include "solving/pipe.h"
#include "stipulation/slice_insertion.h"
#include "stipulation/pipe.h"
#include "stipulation/branch.h"
#include "stipulation/battle_play/branch.h"
#include "stipulation/help_play/branch.h"
#include "options/maxsolutions/initialiser.h"
#include "options/maxsolutions/guard.h"
#include "solving/incomplete.h"
#include "debugging/trace.h"
#include "debugging/assert.h"

#include <limits.h>

/* number of solutions found in the current phase */
static unsigned int nr_solutions_found_in_phase;

/* maximum number of allowed solutions found in the current phase */
static unsigned int max_nr_solutions_per_phase = UINT_MAX;

typedef struct
{
    boolean inserted;
    slice_index incomplete;
} insertion_struct;

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
  insertion_struct * const insertion = st->param;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParamListEnd();

  {
    slice_index const prototypes[] =
    {
      alloc_maxsolutions_guard_slice(),
      alloc_maxsolutions_counter_slice(insertion->incomplete)
    };
    enum { nr_prototypes = sizeof prototypes / sizeof prototypes[0] };
    slice_insertion_insert(si,prototypes,nr_prototypes);
  }

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

static void insert_counter(slice_index si, stip_structure_traversal *st)
{
  insertion_struct * const insertion = st->param;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParamListEnd();

  stip_traverse_structure_children(si,st);

  if (st->context==stip_traversal_context_help && !insertion->inserted)
  {
    slice_index const prototype = alloc_maxsolutions_counter_slice(insertion->incomplete);
    slice_insertion_insert(SLICE_NEXT2(si),&prototype,1);
    insertion->inserted = true;
  }

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

static void insert_counter_at_goal(slice_index si, stip_structure_traversal *st)
{
  insertion_struct * const insertion = st->param;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParamListEnd();

  stip_traverse_structure_children(si,st);

  if (st->context==stip_traversal_context_help && !insertion->inserted)
  {
    slice_index const prototype = alloc_maxsolutions_counter_slice(insertion->incomplete);
    help_branch_insert_slices_behind_proxy(SLICE_NEXT2(si),&prototype,1,si);
    insertion->inserted = true;
  }

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

static void insert_counter_in_forced_branch(slice_index si, stip_structure_traversal *st)
{
  insertion_struct * const insertion = st->param;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParamListEnd();

  stip_traverse_structure_children_pipe(si,st);

  if (st->context==stip_traversal_context_help && !insertion->inserted)
  {
    slice_index const prototype = alloc_maxsolutions_counter_slice(insertion->incomplete);
    slice_insertion_insert(SLICE_NEXT2(si),&prototype,1);
    insertion->inserted = true;
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
static void instrument_solvers(slice_index si)
{
  stip_structure_traversal st;
  insertion_struct insertion = { false, SLICE_NEXT2(si) };

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

  stip_structure_traversal_init(&st,&insertion);
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

/* Instrument the solving machinery with option maxsolutions
 * @param si identifies the slice where to start instrumenting
 */
void maxsolutions_solving_instrumenter_solve(slice_index si)
{
  TraceFunctionEntry(__func__);
  TraceFunctionParamListEnd();

  instrument_solvers(si);

  pipe_solve_delegate(si);

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

/* Instrument the solving machinery with option maxsolutions
 * @param si identifies the slice where to start instrumenting
 */
void maxsolutions_problem_instrumenter_solve(slice_index si)
{
  TraceFunctionEntry(__func__);
  TraceFunctionParamListEnd();

  {
    slice_index const interruption = SLICE_NEXT2(si);
    slice_index const prototype = alloc_pipe(STMaxSolutionsSolvingInstrumenter);
    SLICE_NEXT2(prototype) = interruption;
    assert(interruption!=no_slice);
    slice_insertion_insert(si,&prototype,1);
  }

  pipe_solve_delegate(si);

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

/* Instrument the current problem with option maxsolutions
 * @param si identifies the slice where to start instrumenting
 * @param max_nr_solutions_per_phase
 */
void maxsolutions_instrument_problem(slice_index si, unsigned int i)
{
  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParam("%u",i);
  TraceFunctionParamListEnd();

  {
    slice_index const interruption = branch_find_slice(STPhaseSolvingIncomplete,
                                                       si,
                                                       stip_traversal_context_intro);
    slice_index const prototype = alloc_pipe(STMaxSolutionsProblemInstrumenter);
    SLICE_NEXT2(prototype) = interruption;
    assert(interruption!=no_slice);
    slice_insertion_insert(si,&prototype,1);
    max_nr_solutions_per_phase = i;
  }

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
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
void increase_nr_found_solutions(slice_index si)
{
  TraceFunctionEntry(__func__);
  TraceFunctionParamListEnd();

  ++nr_solutions_found_in_phase;
  TraceValue("->%u",nr_solutions_found_in_phase);
  TraceEOL();

  if (max_nr_solutions_found_in_phase())
    phase_solving_remember_incompleteness(si,solving_interrupted);

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
