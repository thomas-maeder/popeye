#include "pypipe.h"
#include "pykeepmt.h"
#include "pyhelp.h"
#include "pyleaf.h"
#include "trace.h"

#include <assert.h>

/* Initialise a STKeepMatingGuard slice
 * @param si identifies slice to be initialised
 * @param side mating side
 */
static void init_keepmating_guard_slice(slice_index si, Side mating)
{
  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",mating);
  TraceFunctionParam("%u",si);
  TraceFunctionParamListEnd();

  slices[si].type = STKeepMatingGuard; 
  slices[si].starter = no_side; 
  slices[si].u.pipe.u.keepmating_guard.mating = mating;

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

/* Solve in a number of half-moves
 * @param si identifies slice
 * @param n number of half moves until end state has to be reached
 * @return true iff >=1 solution was found
 */
boolean keepmating_guard_solve_in_n(slice_index si, stip_length_type n)
{
  Side const mating = slices[si].u.pipe.u.keepmating_guard.mating;
  boolean result;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParam("%u",n);
  TraceFunctionParamListEnd();

  assert(n>=slack_length_help);

  result = (is_a_mating_piece_left(mating)
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
boolean keepmating_guard_has_solution_in_n(slice_index si, stip_length_type n)
{
  Side const mating = slices[si].u.pipe.u.keepmating_guard.mating;
  boolean result;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParam("%u",n);
  TraceFunctionParamListEnd();

  assert(n>=slack_length_help);

  result = (is_a_mating_piece_left(mating)
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
void keepmating_guard_solve_continuations_in_n(table continuations,
                                          slice_index si,
                                          stip_length_type n)
{
  Side const mating = slices[si].u.pipe.u.keepmating_guard.mating;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParam("%u",n);
  TraceFunctionParamListEnd();

  assert(n>=slack_length_help);

  if (is_a_mating_piece_left(mating))
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
boolean keepmating_guard_impose_starter(slice_index si, slice_traversal *st)
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

typedef boolean keepmating_type[nr_sides];

static boolean keepmating_guards_inserter_leaf(slice_index si,
                                               slice_traversal *st)
{
  boolean const result = true;
  keepmating_type * const km = st->param;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParamListEnd();

  (*km)[slices[si].starter] = true;
  
  TraceFunctionExit(__func__);
  TraceFunctionResult("%u",result);
  TraceFunctionResultEnd();
  return result;
}

static boolean keepmating_guards_inserter_quodlibet(slice_index si,
                                                    slice_traversal *st)
{
  boolean const result = true;
  keepmating_type * const km = st->param;
  keepmating_type km1 = { false, false };
  keepmating_type km2 = { false, false };

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParamListEnd();

  st->param = &km1;
  traverse_slices(slices[si].u.fork.op1,st);

  st->param = &km2;
  traverse_slices(slices[si].u.fork.op2,st);

  (*km)[White] = km1[White] && km2[White];
  (*km)[Black] = km1[Black] && km2[Black];

  st->param = km;
  
  TraceFunctionExit(__func__);
  TraceFunctionResult("%u",result);
  TraceFunctionResultEnd();
  return result;
}

static boolean keepmating_guards_inserter_reciprocal(slice_index si,
                                                     slice_traversal *st)
{
  boolean const result = true;
  keepmating_type * const km = st->param;
  keepmating_type km1 = { false, false };
  keepmating_type km2 = { false, false };

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParamListEnd();

  st->param = &km1;
  traverse_slices(slices[si].u.fork.op1,st);

  st->param = &km2;
  traverse_slices(slices[si].u.fork.op2,st);

  (*km)[White] = km1[White] || km2[White];
  (*km)[Black] = km1[Black] || km2[Black];

  st->param = km;
  
  TraceFunctionExit(__func__);
  TraceFunctionResult("%u",result);
  TraceFunctionResultEnd();
  return result;
}

static void insert_keepmating_guards_help(slice_index help_adapter, Side side)
{
  slice_index const anchor = slices[help_adapter].u.pipe.next;
  slice_index curr = anchor;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",help_adapter);
  TraceFunctionParam("%u",side);
  TraceFunctionParamListEnd();

  pipe_insert_after(help_adapter);
  init_keepmating_guard_slice(slices[help_adapter].u.pipe.next,side);

  do
  {
    if (slices[curr].type==STBranchHelp)
    {
      pipe_insert_after(curr);
      init_keepmating_guard_slice(slices[curr].u.pipe.next,side);
    }
    curr = slices[curr].u.pipe.next;
  } while (curr!=no_slice && curr!=anchor);
  
  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

static boolean keepmating_guards_inserter_help(slice_index si,
                                               slice_traversal *st)
{
  boolean const result = true;
  keepmating_type const * const km = st->param;
  slice_index const fork = slices[si].u.pipe.u.help_adapter.fork;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParamListEnd();

  traverse_slices(slices[fork].u.pipe.u.branch_fork.towards_goal,st);

  if ((*km)[White])
    insert_keepmating_guards_help(si,White);
  if ((*km)[Black])
    insert_keepmating_guards_help(si,Black);
  
  TraceFunctionExit(__func__);
  TraceFunctionResult("%u",result);
  TraceFunctionResultEnd();
  return result;
}

static slice_operation const keepmating_guards_inserters[] =
{
  &slice_traverse_children,               /* STBranchDirect */
  &slice_traverse_children,               /* STBranchDirectDefender */
  0,                                      /* STBranchHelp */
  &slice_traverse_children,               /* STBranchSeries */
  0,                                      /* STBranchFork */
  &keepmating_guards_inserter_leaf,       /* STLeafDirect */
  &keepmating_guards_inserter_leaf,       /* STLeafHelp */
  &slice_traverse_children,               /* STLeafSelf */
  &keepmating_guards_inserter_leaf,       /* STLeafForced */
  &keepmating_guards_inserter_reciprocal, /* STReciprocal */
  &keepmating_guards_inserter_quodlibet,  /* STQuodlibet */
  &slice_traverse_children,               /* STNot */
  &slice_traverse_children,               /* STMoveInverter */
  &keepmating_guards_inserter_help,       /* STHelpRoot */
  &keepmating_guards_inserter_help,       /* STHelpAdapter */
  0,                                      /* STHelpHashed */
  0,                                      /* STSelfCheckGuard */
  0,                                      /* STReflexGuard */
  0                                       /* STKeepMatingGuard */
};

/* Instrument stipulation with STKeepMatingGuard slices
 */
void stip_insert_keepmating_guards(void)
{
  keepmating_type km = { false, false };
  slice_traversal st;

  TraceFunctionEntry(__func__);
  TraceFunctionParamListEnd();

  slice_traversal_init(&st,&keepmating_guards_inserters,&km);
  traverse_slices(root_slice,&st);

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}
