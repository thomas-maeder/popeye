#include "pyselfcg.h"
#include "pyhelp.h"
#include "pyleaf.h"
#include "pyproc.h"
#include "pydata.h"
#include "trace.h"

#include <assert.h>

/* Allocate a STSelfCheckGuard slice
 * @param next identifies next slice
 */
static void insert_selfcheck_guard_slice(slice_index next)
{
  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",next);
  TraceFunctionParamListEnd();

  slices[next].u.pipe.next = copy_slice(next);
  slices[next].type = STSelfCheckGuard;

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

/* Solve in a number of half-moves
 * @param si identifies slice
 * @param n number of half moves until end state has to be reached
 * @return true iff >=1 solution was found
 */
boolean selfcheck_guard_solve_in_n(slice_index si, stip_length_type n)
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
boolean selfcheck_guard_has_solution_in_n(slice_index si, stip_length_type n)
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
void selfcheck_guard_solve_continuations_in_n(table continuations,
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

/* Impose the starting side on a stipulation
 * @param si identifies branch
 * @param st address of structure that holds the state of the traversal
 * @return true iff the operation is successful in the subtree of
 *         which si is the root
 */
boolean selfcheck_guard_impose_starter(slice_index si, slice_traversal *st)
{
  boolean const result = true;
  Side const * const starter = st->param;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParam("%u",*starter);
  TraceFunctionParamListEnd();

  slices[si].starter = *starter;
  slice_traverse_children(si,st);

  TraceFunctionExit(__func__);
  TraceFunctionResult("%u",result);
  TraceFunctionResultEnd();
  return result;
}

static boolean selfcheck_guards_inserter_help(slice_index si,
                                              slice_traversal *st)
{
  boolean const result = true;
  slice_index const next = slices[si].u.pipe.next;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParamListEnd();

  slice_traverse_children(si,st);

  if (slices[next].type!=STSelfCheckGuard)
    insert_selfcheck_guard_slice(next);

  TraceFunctionExit(__func__);
  TraceFunctionResult("%u",result);
  TraceFunctionResultEnd();
  return result;
}

static boolean selfcheck_guards_inserter_help_root(slice_index si,
                                                   slice_traversal *st)
{
  boolean const result = true;
  slice_index const next = slices[si].u.pipe.next;
  slice_index * const fork = &slices[si].u.pipe.u.help_adapter.fork;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParamListEnd();

  slice_traverse_children(si,st);

  if (slices[si].u.pipe.u.help_adapter.length-slack_length_help<=2)
  {
    if (slices[next].type!=STSelfCheckGuard)
      insert_selfcheck_guard_slice(next);
  }
  else
    assert(slices[next].type==STSelfCheckGuard);

  TraceEnumerator(SliceType,slices[*fork].type," -> ");
  assert(slices[*fork].type==STSelfCheckGuard);

  *fork = slices[*fork].u.pipe.next;
  TraceEnumerator(SliceType,slices[*fork].type,"\n");
  assert(slices[*fork].type==STBranchFork);

  TraceFunctionExit(__func__);
  TraceFunctionResult("%u",result);
  TraceFunctionResultEnd();
  return result;
}

static boolean selfcheck_guards_inserter_help_adapter(slice_index si,
                                                      slice_traversal *st)
{
  boolean const result = true;
  slice_index * const fork = &slices[si].u.pipe.u.help_adapter.fork;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParamListEnd();

  slice_traverse_children(si,st);

  assert(slices[*fork].type==STSelfCheckGuard);
  *fork = slices[*fork].u.pipe.next;
  assert(slices[*fork].type==STBranchFork);

  TraceFunctionExit(__func__);
  TraceFunctionResult("%u",result);
  TraceFunctionResultEnd();
  return result;
}

static slice_operation const selfcheck_guards_inserters[] =
{
  &slice_traverse_children,                /* STBranchDirect */
  &slice_traverse_children,                /* STBranchDirectDefender */
  &selfcheck_guards_inserter_help,         /* STBranchHelp */
  &slice_traverse_children,                /* STBranchSeries */
  &slice_traverse_children,                /* STBranchFork */
  &slice_operation_noop,                   /* STLeafDirect */
  &slice_operation_noop,                   /* STLeafHelp */
  &slice_traverse_children,                /* STLeafSelf */
  &slice_operation_noop,                   /* STLeafForced */
  &slice_traverse_children,                /* STReciprocal */
  &slice_traverse_children,                /* STQuodlibet */
  &slice_traverse_children,                /* STNot */
  &slice_traverse_children,                /* STMoveInverter */
  &selfcheck_guards_inserter_help_root,    /* STHelpRoot */
  &selfcheck_guards_inserter_help_adapter, /* STHelpAdapter */
  &slice_traverse_children,                /* STHelpHashed */
  &slice_traverse_children,                /* STSelfCheckGuard */
  &slice_traverse_children,                /* STReflexGuard */
  0                                        /* STKeepMatingGuard */
};

/* Instrument stipulation with STSelfCheckGuard slices
 */
void stip_insert_selfcheck_guards(void)
{
  slice_traversal st;

  TraceFunctionEntry(__func__);
  TraceFunctionParamListEnd();

  slice_traversal_init(&st,&selfcheck_guards_inserters,0);
  traverse_slices(root_slice,&st);

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}
