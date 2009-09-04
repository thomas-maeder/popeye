#include "pypipe.h"
#include "pykeepmt.h"
#include "pyhelp.h"
#include "pyseries.h"
#include "pyleaf.h"
#include "trace.h"

#include <assert.h>


/* **************** Initialisation ***************
 */

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


/* **************** Implementation of interface Direct ***************
 */

/* Determine whether there is a solution in n half moves.
 * @param si slice index of slice being solved
 * @param n maximum number of half moves until end state has to be reached
 * @param curr_max_nr_nontrivial remaining maximum number of
 *                               allowed non-trivial variations
 * @return whether there is a solution and (to some extent) why not
 */
has_solution_type
keepmating_guard_direct_has_solution_in_n(slice_index si,
                                          stip_length_type n,
                                          unsigned int curr_max_nr_nontrivial)
{
  Side const mating = slices[si].u.pipe.u.keepmating_guard.mating;
  has_solution_type result;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParamListEnd();

  TraceEnumerator(Side,mating,"\n");

  if (is_a_mating_piece_left(mating))
    result = direct_has_solution_in_n(slices[si].u.pipe.next,
                                      n,
                                      curr_max_nr_nontrivial);
  else
    result = has_no_solution;

  TraceFunctionExit(__func__);
  TraceEnumerator(has_solution_type,result,"");
  TraceFunctionResultEnd();
  return result;
}

/* Determine whether the defense just played defends against the threats.
 * @param threats table containing the threats
 * @param len_threat length of threat(s) in table threats
 * @param si slice index
 * @param n maximum number of moves until goal
 * @param curr_max_nr_nontrivial remaining maximum number of
 *                               allowed non-trivial variations
 * @return true iff the defense defends against at least one of the
 *         threats
 */
boolean keepmating_guard_are_threats_refuted_in_n(table threats,
                                                  stip_length_type len_threat,
                                                  slice_index si,
                                                  stip_length_type n,
                                                  unsigned int curr_max_nr_nontrivial)
{
  Side const mating = slices[si].u.pipe.u.keepmating_guard.mating;
  boolean result;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",len_threat);
  TraceFunctionParam("%u",si);
  TraceFunctionParam("%u",n);
  TraceFunctionParam("%u",curr_max_nr_nontrivial);
  TraceFunctionParamListEnd();

  TraceEnumerator(Side,mating,"\n");

  if (is_a_mating_piece_left(mating))
    result = direct_are_threats_refuted_in_n(threats,
                                             len_threat,
                                             slices[si].u.pipe.next,
                                             n,
                                             curr_max_nr_nontrivial);
  else
    result = true;

  TraceFunctionExit(__func__);
  TraceFunctionResult("%u",result);
  TraceFunctionResultEnd();
  return result;
}

/* Determine and write solution(s): add first moves to table (as
 * threats for the parent slice. First consult hash table.
 * @param continuations table where to add first moves
 * @param si slice index of slice being solved
 * @param n maximum number of half moves until end state has to be reached
 */
void keepmating_guard_direct_solve_continuations_in_n(table continuations,
                                                      slice_index si,
                                                      stip_length_type n)
{
  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParam("%u",n);
  TraceFunctionParamListEnd();

  direct_solve_continuations_in_n(continuations,slices[si].u.pipe.next,n);

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}


/* **************** Implementation of interface DirectDefender **********
 */

/* Try to defend after an attempted key move at root level
 * @param si slice index
 * @return true iff the defender can successfully defend
 */
boolean keepmating_guard_root_defend(slice_index si)
{
  Side const mating = slices[si].u.pipe.u.keepmating_guard.mating;
  boolean result;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParamListEnd();

  TraceEnumerator(Side,mating,"\n");

  if (is_a_mating_piece_left(mating))
    result = direct_defender_root_defend(slices[si].u.pipe.next);
  else
    result = true;

  TraceFunctionExit(__func__);
  TraceFunctionResult("%u",result);
  TraceFunctionResultEnd();
  return result;
}

/* Try to defend after an attempted key move at non-root level
 * @param si slice index
 * @param n maximum number of half moves until end state has to be reached
 * @param curr_max_nr_nontrivial remaining maximum number of
 *                               allowed non-trivial variations
 * @return true iff the defender can successfully defend
 */
boolean keepmating_guard_defend_in_n(slice_index si,
                                     stip_length_type n,
                                     unsigned int curr_max_nr_nontrivial)
{
  Side const mating = slices[si].u.pipe.u.keepmating_guard.mating;
  slice_index const next = slices[si].u.pipe.next;
  boolean result;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParamListEnd();

  TraceEnumerator(Side,mating,"\n");

  if (is_a_mating_piece_left(mating))
    result = direct_defender_defend_in_n(next,n,curr_max_nr_nontrivial);
  else
    result = true;

  TraceFunctionExit(__func__);
  TraceFunctionResult("%u",result);
  TraceFunctionResultEnd();
  return result;
}

/* Determine whether there is a defense after an attempted key move at
 * non-root level 
 * @param si slice index
 * @param n maximum number of half moves until end state has to be reached
 * @param curr_max_nr_nontrivial remaining maximum number of
 *                               allowed non-trivial variations
 * @return true iff the defender can successfully defend
 */
boolean keepmating_guard_can_defend_in_n(slice_index si,
                                         stip_length_type n,
                                         unsigned int curr_max_nr_nontrivial)
{
  Side const mating = slices[si].u.pipe.u.keepmating_guard.mating;
  slice_index const next = slices[si].u.pipe.next;
  boolean result;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParamListEnd();

  TraceEnumerator(Side,mating,"\n");

  if (is_a_mating_piece_left(mating))
    result = direct_defender_can_defend_in_n(next,n,curr_max_nr_nontrivial);
  else
    result = true;

  TraceFunctionExit(__func__);
  TraceFunctionResult("%u",result);
  TraceFunctionResultEnd();
  return result;
}

/* Solve postkey play at root level.
 * @param refutations table containing the refutations (if any)
 * @param si slice index
 * @return true iff >=1 solution was found
 */
boolean keepmating_guard_root_solve_postkey(table refutations, slice_index si)
{
  Side const mating = slices[si].u.pipe.u.keepmating_guard.mating;
  boolean result;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParamListEnd();

  TraceEnumerator(Side,mating,"\n");

  if (is_a_mating_piece_left(mating))
    result = slice_root_solve_postkey(refutations,slices[si].u.pipe.next);
  else
    result = false;

  TraceFunctionExit(__func__);
  TraceFunctionResult("%u",result);
  TraceFunctionResultEnd();
  return result;
}

/* Solve postkey play play after the move that has just
 * been played in the current ply.
 * @param si slice index
 * @param n maximum number of half moves until goal
 */
boolean keepmating_guard_solve_postkey_in_n(slice_index si, stip_length_type n)
{
  Side const mating = slices[si].u.pipe.u.keepmating_guard.mating;
  boolean result;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParamListEnd();

  TraceEnumerator(Side,mating,"\n");

  if (is_a_mating_piece_left(mating))
    result = direct_defender_solve_postkey_in_n(slices[si].u.pipe.next,n);
  else
    result = true;

  TraceFunctionExit(__func__);
  TraceFunctionResult("%u",result);
  TraceFunctionResultEnd();
  return result;
}


/* **************** Implementation of interface Help ***************
 */

/* Solve in a number of half-moves
 * @param si identifies slice
 * @param n exact number of half moves until end state has to be reached
 * @return true iff >=1 solution was found
 */
boolean keepmating_guard_help_solve_in_n(slice_index si, stip_length_type n)
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
 * @param n exact number of half moves until end state has to be reached
 * @return true iff >= 1 solution has been found
 */
boolean keepmating_guard_help_has_solution_in_n(slice_index si,
                                                stip_length_type n)
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
 * @param n exact number of half moves until end state has to be reached
 */
void keepmating_guard_help_solve_continuations_in_n(table continuations,
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


/* **************** Implementation of interface Series ***************
 */

/* Solve in a number of half-moves
 * @param si identifies slice
 * @param n exact number of half moves until end state has to be reached
 * @return true iff >=1 solution was found
 */
boolean keepmating_guard_series_solve_in_n(slice_index si, stip_length_type n)
{
  Side const mating = slices[si].u.pipe.u.keepmating_guard.mating;
  boolean result;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParam("%u",n);
  TraceFunctionParamListEnd();

  assert(n>=slack_length_series);

  result = (is_a_mating_piece_left(mating)
            && series_solve_in_n(slices[si].u.pipe.next,n));

  TraceFunctionExit(__func__);
  TraceFunctionResult("%u",result);
  TraceFunctionResultEnd();
  return result;
}

/* Determine whether there is a solution in n half moves.
 * @param si slice index of slice being solved
 * @param n exact number of half moves until end state has to be reached
 * @return true iff >= 1 solution has been found
 */
boolean keepmating_guard_series_has_solution_in_n(slice_index si,
                                                  stip_length_type n)
{
  Side const mating = slices[si].u.pipe.u.keepmating_guard.mating;
  boolean result;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParam("%u",n);
  TraceFunctionParamListEnd();

  assert(n>=slack_length_series);

  result = (is_a_mating_piece_left(mating)
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
 * @param n exact number of half moves until end state has to be reached
 */
void keepmating_guard_series_solve_continuations_in_n(table continuations,
                                                      slice_index si,
                                                      stip_length_type n)
{
  Side const mating = slices[si].u.pipe.u.keepmating_guard.mating;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParam("%u",n);
  TraceFunctionParamListEnd();

  assert(n>=slack_length_series);

  if (is_a_mating_piece_left(mating))
    series_solve_continuations_in_n(continuations,slices[si].u.pipe.next,n);

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}


/* **************** Implementation of interface Slice ***************
 */


/* **************** Stipulation instrumentation ***************
 */

/* Data structure for remembering the side(s) that needs to keep >= 1
 * piece that could deliver mate
 */
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

static boolean keepmating_guards_inserter_branch_fork(slice_index si,
                                                      slice_traversal *st)
{
  boolean const result = true;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParamListEnd();

  /* we can't rely on the (arbitrary) order slice_traverse_children()
   * would use; instead make sure that we first traverse towards the
   * goal(s).
   */
  traverse_slices(slices[si].u.pipe.u.branch.towards_goal,st);
  traverse_slices(slices[si].u.pipe.next,st);
  
  TraceFunctionExit(__func__);
  TraceFunctionResult("%u",result);
  TraceFunctionResultEnd();
  return result;
}

static boolean keepmating_guards_inserter_branch(slice_index si,
                                                 slice_traversal *st)
{
  boolean const result = true;
  keepmating_type const * const km = st->param;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParamListEnd();

  slice_traverse_children(si,st);

  if ((*km)[White])
  {
    pipe_insert_after(si);
    init_keepmating_guard_slice(slices[si].u.pipe.next,White);
  }

  if ((*km)[Black])
  {
    pipe_insert_after(si);
    init_keepmating_guard_slice(slices[si].u.pipe.next,Black);
  }
  
  TraceFunctionExit(__func__);
  TraceFunctionResult("%u",result);
  TraceFunctionResultEnd();
  return result;
}

static slice_operation const keepmating_guards_inserters[] =
{
  &keepmating_guards_inserter_branch,      /* STBranchDirect */
  &keepmating_guards_inserter_branch,      /* STBranchDirectDefender */
  &keepmating_guards_inserter_branch,      /* STBranchHelp */
  &slice_traverse_children,                /* STBranchSeries */
  &keepmating_guards_inserter_branch_fork, /* STBranchFork */
  &keepmating_guards_inserter_leaf,        /* STLeafDirect */
  &keepmating_guards_inserter_leaf,        /* STLeafHelp */
  &keepmating_guards_inserter_leaf,        /* STLeafForced */
  &keepmating_guards_inserter_reciprocal,  /* STReciprocal */
  &keepmating_guards_inserter_quodlibet,   /* STQuodlibet */
  &slice_traverse_children,                /* STNot */
  &slice_traverse_children,                /* STMoveInverter */
  &keepmating_guards_inserter_branch,      /* STDirectRoot */
  &keepmating_guards_inserter_branch,      /* STDirectDefenderRoot */
  &slice_traverse_children,                /* STDirectHashed */
  &keepmating_guards_inserter_branch,      /* STHelpRoot */
  &slice_traverse_children,                /* STHelpAdapter */
  &slice_traverse_children,                /* STHelpHashed */
  &keepmating_guards_inserter_branch,      /* STSeriesRoot */
  &slice_traverse_children,                /* STSeriesAdapter */
  &slice_traverse_children,                /* STSeriesHashed */
  &slice_traverse_children,                /* STSelfCheckGuard */
  &slice_traverse_children,                /* STDirectAttack */
  &slice_traverse_children,                /* STDirectDefense */
  &slice_traverse_children,                /* STReflexGuard */
  &slice_traverse_children,                /* STSelfAttack */
  &slice_traverse_children,                /* STSelfDefense */
  &slice_traverse_children,                /* STRestartGuard */
  0,                                       /* STGoalReachableGuard */
  0,                                       /* STKeepMatingGuard */
  0,                                       /* STMaxFlightsquares */
  0                                        /* STMaxThreatLength */
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
