#include "pyreflxg.h"
#include "pyhelp.h"
#include "pyseries.h"
#include "pynot.h"
#include "pypipe.h"
#include "trace.h"

#include <assert.h>

/* Initialise a STReflexGuard slice
 * @param si identifies slice to be initialised
 * @param to_be_avoided prototype of slice that must not be solvable
 */
void init_reflex_guard_slice(slice_index si, slice_index to_be_avoided)
{
  slice_index not_slice;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParam("%u",to_be_avoided);
  TraceFunctionParamListEnd();

  slices[si].type = STReflexGuard; 
  slices[si].starter = no_side; 

  /* don't link not_slice to to_be_avoided: to_be_avoided and
   * to_be_avoided_copy will have different starters!
   * TODO deep copy needed in general
   */
  not_slice = alloc_not_slice(copy_slice(to_be_avoided));
  slices[si].u.pipe.u.reflex_guard.not_slice = not_slice;

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

/* Solve a slice at root level
 * @param si slice index
 * @return true iff >=1 solution was found
 */
boolean reflex_guard_root_solve(slice_index si)
{
  boolean result;
  slice_index const not_slice = slices[si].u.pipe.u.reflex_guard.not_slice;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParamListEnd();

  if (not_has_solution(not_slice))
    result = slice_root_solve(slices[si].u.pipe.next);
  else
  {
    result = false;
    slice_write_unsolvability(not_slice);
  }

  TraceFunctionExit(__func__);
  TraceFunctionResult("%u",result);
  TraceFunctionResultEnd();
  return result;
}

/* Solve in a number of half-moves
 * @param si identifies slice
 * @param n number of half moves until end state has to be reached
 * @return true iff >=1 solution was found
 */
boolean reflex_guard_help_solve_in_n(slice_index si, stip_length_type n)
{
  boolean result;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParam("%u",n);
  TraceFunctionParamListEnd();

  assert(n>slack_length_help);

  result = (not_has_solution(slices[si].u.pipe.u.reflex_guard.not_slice)
            && help_solve_in_n(slices[si].u.pipe.next,n));

  TraceFunctionExit(__func__);
  TraceFunctionResult("%u",result);
  TraceFunctionResultEnd();
  return result;
}

/* Determine whether there is a solution in n half moves.
 * @param si slice index of slice being solved
 * @param n number of half moves until end state has to be reached
 * @return true iff >= 1 solution has been found
 */
boolean reflex_guard_help_has_solution_in_n(slice_index si, stip_length_type n)
{
  boolean result;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParam("%u",n);
  TraceFunctionParamListEnd();

  assert(n>slack_length_help);

  result = (not_has_solution(slices[si].u.pipe.u.reflex_guard.not_slice)
            && help_has_solution_in_n(slices[si].u.pipe.next,n));

  TraceFunctionExit(__func__);
  TraceFunctionResult("%u",result);
  TraceFunctionResultEnd();
  return result;
}

/* Determine and write solution(s): add first moves to table (as
 * threats for the parent slice. First consult hash table.
 * @param continuations table where to add first moves
 * @param si slice index of slice being solved
 * @param n number of half moves until end state has to be reached
 */
void reflex_guard_help_solve_continuations_in_n(table continuations,
                                          slice_index si,
                                          stip_length_type n)
{
  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParam("%u",n);
  TraceFunctionParamListEnd();

  assert(n>slack_length_help);

  if (not_has_solution(slices[si].u.pipe.u.reflex_guard.not_slice))
    help_solve_continuations_in_n(continuations,slices[si].u.pipe.next,n);

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

/* Solve in a number of half-moves
 * @param si identifies slice
 * @param n number of half moves until end state has to be reached
 * @return true iff >=1 solution was found
 */
boolean reflex_guard_series_solve_in_n(slice_index si, stip_length_type n)
{
  boolean result;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParam("%u",n);
  TraceFunctionParamListEnd();

  assert(n>slack_length_series);

  result = (not_has_solution(slices[si].u.pipe.u.reflex_guard.not_slice)
            && series_solve_in_n(slices[si].u.pipe.next,n));

  TraceFunctionExit(__func__);
  TraceFunctionResult("%u",result);
  TraceFunctionResultEnd();
  return result;
}

/* Determine whether there is a solution in n half moves.
 * @param si slice index of slice being solved
 * @param n number of half moves until end state has to be reached
 * @return true iff >= 1 solution has been found
 */
boolean reflex_guard_series_has_solution_in_n(slice_index si, stip_length_type n)
{
  boolean result;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParam("%u",n);
  TraceFunctionParamListEnd();

  assert(n>slack_length_series);

  result = (not_has_solution(slices[si].u.pipe.u.reflex_guard.not_slice)
            && series_has_solution_in_n(slices[si].u.pipe.next,n));

  TraceFunctionExit(__func__);
  TraceFunctionResult("%u",result);
  TraceFunctionResultEnd();
  return result;
}

/* Determine and write solution(s): add first moves to table (as
 * threats for the parent slice. First consult hash table.
 * @param continuations table where to add first moves
 * @param si slice index of slice being solved
 * @param n number of half moves until end state has to be reached
 */
void reflex_guard_series_solve_continuations_in_n(table continuations,
                                                  slice_index si,
                                                  stip_length_type n)
{
  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParam("%u",n);
  TraceFunctionParamListEnd();

  assert(n>slack_length_series);

  if (not_has_solution(slices[si].u.pipe.u.reflex_guard.not_slice))
    series_solve_continuations_in_n(continuations,slices[si].u.pipe.next,n);

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

static boolean reflex_guards_inserter_branch(slice_index si,
                                             slice_traversal *st)
{
  boolean const result = true;
  slice_index const * const tobeavoided = st->param;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParamListEnd();

  slice_traverse_children(si,st);

  pipe_insert_before(si);
  init_reflex_guard_slice(si,*tobeavoided);

  TraceFunctionExit(__func__);
  TraceFunctionResult("%u",result);
  TraceFunctionResultEnd();
  return result;
}

static boolean reflex_guards_inserter_branch_fork(slice_index si,
                                                  slice_traversal *st)
{
  boolean const result = true;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParamListEnd();

  /* don't traverse .towards_goal! */
  traverse_slices(slices[si].u.pipe.next,st);

  TraceFunctionExit(__func__);
  TraceFunctionResult("%u",result);
  TraceFunctionResultEnd();
  return result;
}


static slice_operation const reflex_guards_inserters[] =
{
  &slice_traverse_children,            /* STBranchDirect */
  &slice_traverse_children,            /* STBranchDirectDefender */
  &reflex_guards_inserter_branch,      /* STBranchHelp */
  &reflex_guards_inserter_branch,      /* STBranchSeries */
  &reflex_guards_inserter_branch_fork, /* STBranchFork */
  &slice_operation_noop,               /* STLeafDirect */
  &slice_operation_noop,               /* STLeafHelp */
  &slice_operation_noop,               /* STLeafSelf */
  &slice_operation_noop,               /* STLeafForced */
  &slice_traverse_children,            /* STReciprocal */
  &slice_traverse_children,            /* STQuodlibet */
  &slice_traverse_children,            /* STNot */
  &slice_traverse_children,            /* STMoveInverter */
  &slice_traverse_children,            /* STDirectRoot */
  &slice_traverse_children,            /* STDirectAdapter */
  &reflex_guards_inserter_branch,      /* STHelpRoot */
  &slice_traverse_children,            /* STHelpAdapter */
  &slice_traverse_children,            /* STHelpHashed */
  &reflex_guards_inserter_branch,      /* STSeriesRoot */
  &slice_traverse_children,            /* STSeriesAdapter */
  &slice_traverse_children,            /* STSeriesHashed */
  0,                                   /* STSelfCheckGuard */
  0,                                   /* STReflex_guard */
  0,                                   /* STRestartGuard */
  0,                                   /* STGoalReachableGuard */
  0                                    /* STKeepMatingGuard */
};

/* Instrument a branch with STReflexGuard slices
 * @param si root of branch to be instrumented
 * @param tobeavoided identifies what branch needs to be guarded from
 */
void slice_insert_reflex_guards(slice_index si, slice_index tobeavoided)
{
  slice_traversal st;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParam("%u",tobeavoided);
  TraceFunctionParamListEnd();

  slice_traversal_init(&st,&reflex_guards_inserters,&tobeavoided);
  traverse_slices(si,&st);

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}
