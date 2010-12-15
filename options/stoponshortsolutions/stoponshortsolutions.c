#include "optimisations/stoponshortsolutions/stoponshortsolutions.h"
#include "optimisations/stoponshortsolutions/root_solvable_filter.h"
#include "optimisations/stoponshortsolutions/help_filter.h"
#include "optimisations/stoponshortsolutions/series_filter.h"
#include "pystip.h"
#include "pypipe.h"
#include "stipulation/branch.h"
#include "trace.h"

#include <assert.h>

static boolean short_solution_found_in_problem;
static boolean short_solution_found_in_phase;

/* Inform the stoponshortsolutions module that a short solution has
 * been found
 */
void short_solution_found(void)
{
  TraceFunctionEntry(__func__);
  TraceFunctionParamListEnd();

  short_solution_found_in_problem = true;
  short_solution_found_in_phase = true;

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

/* Reset the internal state to "no short solution found"
 */
void reset_short_solution_found_in_problem(void)
{
  TraceFunctionEntry(__func__);
  TraceFunctionParamListEnd();

  short_solution_found_in_problem = false;
  short_solution_found_in_phase = false;

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

/* Has a short solution been found in the current problem?
 */
boolean has_short_solution_been_found_in_problem(void)
{
  boolean const result = short_solution_found_in_problem;

  TraceFunctionEntry(__func__);
  TraceFunctionParamListEnd();

  TraceFunctionExit(__func__);
  TraceFunctionResult("%u",result);
  TraceFunctionResultEnd();
  return result;
}

/* Reset the internal state to "no short solution found" in the
 * current phase
 */
void reset_short_solution_found_in_phase(void)
{
  TraceFunctionEntry(__func__);
  TraceFunctionParamListEnd();

  short_solution_found_in_phase = false;

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

/* Reset the internal state to "no short solution found" in the
 * current phase
 */
boolean has_short_solution_been_found_in_phase(void)
{
  boolean const result = short_solution_found_in_phase;

  TraceFunctionEntry(__func__);
  TraceFunctionParamListEnd();

  TraceFunctionExit(__func__);
  TraceFunctionResult("%u",result);
  TraceFunctionResultEnd();
  return result;
}

/* Insert STStopOnShortSolutions*Filter starting at a slice
 * @param si identifies slice where to start
 * @note this is an indirectly recursive function
 */
static void insert_filters(slice_index si);

/* Insert a STStopOnShortSolutionsHelpFilter slice
 */
static void insert_stoponshortsolutions_help_root(slice_index si,
                                                  stip_structure_traversal *st)
{
  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParamListEnd();

  if (slices[slices[si].u.pipe.next].type!=STStopOnShortSolutionsHelpFilter)
  {
    stip_length_type const length = slices[si].u.branch.length;
    stip_length_type const min_length = slices[si].u.branch.min_length;
    pipe_append(si,alloc_stoponshortsolutions_help_filter(length,min_length));
  }

  {
    slice_index const fork = branch_find_slice(STHelpFork,si);
    if (fork!=no_slice)
      insert_filters(slices[fork].u.branch_fork.towards_goal);
  }

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

/* Insert a STStopOnShortSolutionsHelpFilter slice
 */
static void insert_stoponshortsolutions_help_move(slice_index si,
                                                  stip_structure_traversal *st)
{
  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParamListEnd();

  if (slices[slices[si].u.pipe.next].type!=STStopOnShortSolutionsHelpFilter)
  {
    stip_length_type const length = slices[si].u.branch.length-1;
    stip_length_type const min_length = slices[si].u.branch.min_length-1;
    pipe_append(si,alloc_stoponshortsolutions_help_filter(length,min_length));
  }

  {
    slice_index const fork = branch_find_slice(STHelpFork,si);
    if (fork!=no_slice)
      insert_filters(slices[fork].u.branch_fork.towards_goal);
  }

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

/* Insert a STStopOnShortSolutionsSeriesFilter slice
 */
static
void insert_stoponshortsolutions_series_root(slice_index si,
                                             stip_structure_traversal *st)
{
  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParamListEnd();

  if (slices[slices[si].u.pipe.next].type!=STStopOnShortSolutionsSeriesFilter)
  {
    stip_length_type const length = slices[si].u.branch.length;
    stip_length_type const min_length = slices[si].u.branch.min_length;
    pipe_append(si,alloc_stoponshortsolutions_series_filter(length,min_length));
  }

  {
    slice_index const fork = branch_find_slice(STSeriesFork,si);
    if (fork!=no_slice)
      insert_filters(slices[fork].u.branch_fork.towards_goal);
  }

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

/* Insert a STStopOnShortSolutionsSeriesFilter slice
 */
static
void insert_stoponshortsolutions_series_move(slice_index si,
                                             stip_structure_traversal *st)
{
  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParamListEnd();

  if (slices[slices[si].u.pipe.next].type!=STStopOnShortSolutionsSeriesFilter)
  {
    stip_length_type const length = slices[si].u.branch.length-1;
    stip_length_type const min_length = slices[si].u.branch.min_length-1;
    pipe_append(si,alloc_stoponshortsolutions_series_filter(length,min_length));
  }

  {
    slice_index const fork = branch_find_slice(STSeriesFork,si);
    if (fork!=no_slice)
      insert_filters(slices[fork].u.branch_fork.towards_goal);
  }

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

static structure_traversers_visitors stoponshortsolutions_filter_inserters[] =
{
  { STHelpRoot,                         &insert_stoponshortsolutions_help_root   },
  { STHelpMove,                         &insert_stoponshortsolutions_help_move   },
  { STHelpMoveToGoal,                   &insert_stoponshortsolutions_help_move   },
  { STSeriesRoot,                       &insert_stoponshortsolutions_series_root },
  { STSeriesMove,                       &insert_stoponshortsolutions_series_move },
  { STSeriesMoveToGoal,                 &insert_stoponshortsolutions_series_move },
  { STStopOnShortSolutionsRootSolvableFilter, &stip_structure_visitor_noop  },
  { STStopOnShortSolutionsHelpFilter,         &stip_structure_visitor_noop  },
  { STStopOnShortSolutionsSeriesFilter,       &stip_structure_visitor_noop  },
  { STGoalReachedTesting,                     &stip_structure_visitor_noop  }
};

enum
{
  nr_stoponshortsolutions_filter_inserters =
  (sizeof stoponshortsolutions_filter_inserters
   / sizeof stoponshortsolutions_filter_inserters[0])
};

/* Insert STStopOnShortSolutions*Filter starting at a slice
 * @param si identifies slice where to start
 * @note this is an indirectly recursive function
 */
static void insert_filters(slice_index si)
{
  stip_structure_traversal st;

  TraceFunctionEntry(__func__);
  TraceFunctionParamListEnd();

  stip_structure_traversal_init(&st,0);
  stip_structure_traversal_override(&st,
                                    stoponshortsolutions_filter_inserters,
                                    nr_stoponshortsolutions_filter_inserters);
  stip_traverse_structure(si,&st);

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

/* Insert a STStopOnShortSolutionsRootSolvableFilter slice before some slice
 */
static void insert_root_solvable_filter(slice_index si,
                                        stip_structure_traversal *st)
{
  boolean * const inserted = st->param;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParamListEnd();

  pipe_append(slices[si].prev,
              alloc_stoponshortsolutions_root_solvable_filter());
  insert_filters(si);

  *inserted = true;

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

static
structure_traversers_visitors stoponshortsolutions_initialiser_inserters[] =
{
  { STHelpRoot,   &insert_root_solvable_filter },
  { STSeriesRoot, &insert_root_solvable_filter }
};

enum
{
  nr_stoponshortsolutions_initialiser_inserters =
  (sizeof stoponshortsolutions_initialiser_inserters
   / sizeof stoponshortsolutions_initialiser_inserters[0])
};

/* Instrument a stipulation with STStopOnShortSolutions*Filter slices
 * @param si identifies slice where to start
 * @return true iff the option stoponshort applies
 */
boolean stip_insert_stoponshortsolutions_filters(slice_index si)
{
  boolean result = false;
  stip_structure_traversal st;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParamListEnd();

  TraceStipulation(si);

  stip_structure_traversal_init(&st,&result);
  stip_structure_traversal_override(&st,
                                    stoponshortsolutions_initialiser_inserters,
                                    nr_stoponshortsolutions_initialiser_inserters);
  stip_traverse_structure(si,&st);

  TraceFunctionExit(__func__);
  TraceFunctionResult("%u",result);
  TraceFunctionResultEnd();
  return result;
}
