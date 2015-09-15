#include "options/stoponshortsolutions/stoponshortsolutions.h"
#include "options/stoponshortsolutions/initialiser.h"
#include "options/stoponshortsolutions/filter.h"
#include "stipulation/slice_insertion.h"
#include "stipulation/help_play/branch.h"
#include "solving/pipe.h"
#include "debugging/trace.h"
#include "debugging/assert.h"

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
void stoponshortsolutions_resetter_solve(slice_index si)
{
  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParamListEnd();

  pipe_solve_delegate(si);

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

/* Has a short solution been found in the current phase?
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

static void insert_filter(slice_index si, stip_structure_traversal *st)
{
  boolean * const inserted = st->param;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParamListEnd();

  stip_traverse_structure_children_pipe(si,st);

  {
    stip_length_type const length = SLICE_U(si).branch.length;
    stip_length_type const min_length = SLICE_U(si).branch.min_length;
    slice_index const prototype = alloc_stoponshortsolutions_filter(length,
                                                                    min_length);
    slice_insertion_insert(si,&prototype,1);
  }

  *inserted = true;

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

/* Instrument a stipulation with STStopOnShortSolutions*Filter slices
 * @param si identifies slice where to start
 * @return true iff the option stoponshort applies
 */
boolean solving_insert_stoponshortsolutions_filters(slice_index si)
{
  boolean result = false;
  stip_structure_traversal st;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParamListEnd();

  TraceStipulation(si);

  stip_structure_traversal_init(&st,&result);
  stip_structure_traversal_override_by_contextual(&st,
                                                  slice_contextual_conditional_pipe,
                                                  &stip_traverse_structure_children_pipe);
  stip_structure_traversal_override_single(&st,STHelpAdapter,&insert_filter);
  stip_traverse_structure(si,&st);

  if (result)
  {
    slice_index const prototype = alloc_stoponshortsolutions_initialiser_slice();
    slice_insertion_insert(si,&prototype,1);
  }

  TraceFunctionExit(__func__);
  TraceFunctionResult("%u",result);
  TraceFunctionResultEnd();
  return result;
}
