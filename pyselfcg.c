#include "pypipe.h"
#include "pyselfcg.h"
#include "pyhelp.h"
#include "pyproc.h"
#include "pydata.h"
#include "pymsg.h"
#include "trace.h"

#include <assert.h>

/* Initialise a STSelfCheckGuard slice into an allocated and wired
 * pipe slice 
 * @param si identifies slice
 */
static void init_selfcheck_guard_slice(slice_index si)
{
  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParamListEnd();

  slices[si].type = STSelfCheckGuard;
  slices[si].starter = slices[slices[si].u.pipe.next].starter;

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

/* Solve a slice at root level
 * @param si slice index
 * @return true iff >=1 solution was found
 */
boolean selfcheck_guard_root_solve(slice_index si)
{
  boolean result;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParamListEnd();

  if (echecc(nbply,advers(slices[si].starter)))
  {
    ErrorMsg(KingCapture);
    result = false;
  }
  else
    result = slice_root_solve(slices[si].u.pipe.next);

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
boolean selfcheck_guard_help_solve_in_n(slice_index si, stip_length_type n)
{
  boolean result;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParam("%u",n);
  TraceFunctionParamListEnd();

  assert(n>=slack_length_help);

  result = (!echecc(nbply,advers(slices[si].starter))
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
boolean selfcheck_guard_help_has_solution_in_n(slice_index si, stip_length_type n)
{
  boolean result;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParam("%u",n);
  TraceFunctionParamListEnd();

  assert(n>=slack_length_help);

  result = (!echecc(nbply,advers(slices[si].starter))
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
void selfcheck_guard_help_solve_continuations_in_n(table continuations,
                                              slice_index si,
                                              stip_length_type n)
{
  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParam("%u",n);
  TraceFunctionParamListEnd();

  assert(n>=slack_length_help);

  if (!echecc(nbply,advers(slices[si].starter)))
    help_solve_continuations_in_n(continuations,slices[si].u.pipe.next,n);

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

static boolean selfcheck_guards_inserter_help(slice_index si,
                                              slice_traversal *st)
{
  boolean const result = true;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParamListEnd();

  slice_traverse_children(si,st);

  pipe_insert_after(si);
  init_selfcheck_guard_slice(slices[si].u.pipe.next);

  TraceFunctionExit(__func__);
  TraceFunctionResult("%u",result);
  TraceFunctionResultEnd();
  return result;
}

static boolean selfcheck_guards_inserter_root(slice_index si,
                                              slice_traversal *st)
{
  boolean const result = true;
  branch_level * const level = st->param;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParamListEnd();

  TraceValue("%u\n",*level);
  if (*level==toplevel_branch)
  {
    *level = nested_branch;
    slice_traverse_children(si,st);
    *level = toplevel_branch;

    pipe_insert_before(si);
    init_selfcheck_guard_slice(si);
  }
  else
    slice_traverse_children(si,st);

  TraceFunctionExit(__func__);
  TraceFunctionResult("%u",result);
  TraceFunctionResultEnd();
  return result;
}

static boolean selfcheck_guards_inserter_help_root(slice_index si,
                                                   slice_traversal *st)
{
  boolean const result = true;
  branch_level * const level = st->param;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParamListEnd();

  TraceValue("%u\n",*level);
  if (*level==toplevel_branch)
  {
    *level = nested_branch;
    slice_traverse_children(si,st);
    *level = toplevel_branch;

    pipe_insert_after(si);
    init_selfcheck_guard_slice(slices[si].u.pipe.next);

    pipe_insert_before(si);
    init_selfcheck_guard_slice(si);
  }
  else
  {
    slice_traverse_children(si,st);

    pipe_insert_after(si);
    init_selfcheck_guard_slice(slices[si].u.pipe.next);
  }

  TraceFunctionExit(__func__);
  TraceFunctionResult("%u",result);
  TraceFunctionResultEnd();
  return result;
}

static slice_operation const selfcheck_guards_inserters[] =
{
  &slice_traverse_children,             /* STBranchDirect */
  &slice_traverse_children,             /* STBranchDirectDefender */
  &selfcheck_guards_inserter_help,      /* STBranchHelp */
  &slice_traverse_children,             /* STBranchSeries */
  &slice_traverse_children,             /* STBranchFork */
  &slice_operation_noop,                /* STLeafDirect */
  &slice_operation_noop,                /* STLeafHelp */
  &slice_operation_noop,                /* STLeafSelf */
  &slice_operation_noop,                /* STLeafForced */
  &slice_traverse_children,             /* STReciprocal */
  &slice_traverse_children,             /* STQuodlibet */
  &slice_traverse_children,             /* STNot */
  &slice_traverse_children,             /* STMoveInverter */
  &selfcheck_guards_inserter_help_root, /* STHelpRoot */
  &slice_traverse_children,             /* STHelpAdapter */
  &slice_traverse_children,             /* STHashed */
  0,                                    /* STSelfCheckGuard */
  &selfcheck_guards_inserter_root,      /* STReflexGuard */
  0,                                    /* STRestartGuard */
  0,                                    /* STGoalReachableGuard */
  0                                     /* STKeepMatingGuard */
};

/* Instrument stipulation with STSelfCheckGuard slices
 */
void stip_insert_selfcheck_guards(void)
{
  slice_traversal st;
  branch_level level = toplevel_branch;

  TraceFunctionEntry(__func__);
  TraceFunctionParamListEnd();

  slice_traversal_init(&st,&selfcheck_guards_inserters,&level);
  traverse_slices(root_slice,&st);

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}
