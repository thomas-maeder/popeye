#include "pydegent.h"
#include "pydata.h"
#include "pypipe.h"
#include "pydirect.h"
#include "pyoutput.h"
#include "trace.h"

#include <assert.h>
#include <stdlib.h>

static stip_length_type max_length_short_solutions;

/* Reset the max threats setting to off
 */
void reset_max_nr_nontrivial_length(void)
{
  max_length_short_solutions = no_stip_length;
}

/* Read the requested max threat length setting from a text token
 * entered by the user
 * @param textToken text token from which to read
 * @return true iff max threat setting was successfully read
 */
void init_degenerate_tree(stip_length_type max_length_short)
{
  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",max_length_short);
  TraceFunctionParamListEnd();

  max_length_short_solutions = max_length_short;

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

/* **************** Initialisation ***************
 */

/* Initialise a STMaxFlightsquares slice
 * @param si identifies slice to be initialised
 * @param side mating side
 */
static void init_degenerate_tree_guard_slice(slice_index si)
{
  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParamListEnd();

  slices[si].type = STDegenerateTree; 
  slices[si].starter = no_side; 

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}


/* **************** Implementation of interface Direct **********
 */

/* Determine whether there is a solution in n half moves.
 * @param si slice index of slice being solved
 * @param n maximum number of half moves until end state has to be reached
 * @param n_min minimal number of half moves to try
 * @return length of solution found, i.e.:
 *            0 defense put defender into self-check
 *            n_min..n length of shortest solution found
 *            >n no solution found
 *         (the second case includes the situation in self
 *         stipulations where the defense just played has reached the
 *         goal (in which case n_min<slack_length_direct and we return
 *         n_min)
 */
stip_length_type
degenerate_tree_direct_has_solution_in_n(slice_index si,
                                         stip_length_type n,
                                         stip_length_type n_min)
{
  stip_length_type result = n+2;
  stip_length_type const parity = n%2;
  slice_index const next = slices[si].u.pipe.next;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParam("%u",n);
  TraceFunctionParam("%u",n_min);
  TraceFunctionParamListEnd();

  if (n>max_length_short_solutions+parity)
  {
    if (max_length_short_solutions>=slack_length_direct+2)
    {
      stip_length_type const n_interm = max_length_short_solutions-2+parity;
      result = direct_has_solution_in_n(next,n_interm,n_min);
      if (result>n_interm)
        result = direct_has_solution_in_n(next,n,n);
    }
    else
      result = direct_has_solution_in_n(next,n,n);
  }
  else
    result = direct_has_solution_in_n(next,n,n_min);

  TraceFunctionExit(__func__);
  TraceFunctionResult("%u",result);
  TraceFunctionResultEnd();
  return result;
}

/* Determine and write continuations after the defense just played.
 * We know that there is at least 1 continuation to the defense.
 * Only continuations of minimal length are looked for and written.
 * @param si slice index of slice being solved
 * @param n maximum number of half moves until end state has to be reached
 * @param n_min minimal number of half moves to try
 */
void degenerate_tree_direct_solve_continuations_in_n(slice_index si,
                                                     stip_length_type n,
                                                     stip_length_type n_min)
{
  slice_index const next = slices[si].u.pipe.next;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParam("%u",n);
  TraceFunctionParam("%u",n_min);
  TraceFunctionParamListEnd();

  /* don't increase n_min, or we may write continuations of
   * full length even in the presence of a short continuation
   */
  direct_solve_continuations_in_n(next,n,n_min);

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

/* Determine and write the threats after the move that has just been
 * played.
 * @param threats table where to add threats
 * @param si slice index
 * @param n maximum number of half moves until goal
 * @param n_min minimal number of half moves to try
 * @return length of threats
 *         (n-slack_length_direct)%2 if the attacker has something
 *           stronger than threats (i.e. has delivered check)
 *         n+2 if there is no threat
 */
stip_length_type
degenerate_tree_direct_solve_threats_in_n(table threats,
                                          slice_index si,
                                          stip_length_type n,
                                          stip_length_type n_min)
{
  stip_length_type result;
  slice_index const next = slices[si].u.pipe.next;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParam("%u",n);
  TraceFunctionParam("%u",n_min);
  TraceFunctionParamListEnd();

  result = direct_solve_threats_in_n(threats,next,n,n_min);

  TraceFunctionExit(__func__);
  TraceFunctionResult("%u",result);
  TraceFunctionResultEnd();
  return result;
}

/* Determine whether the defense just played defends against the threats.
 * @param threats table containing the threats
 * @param len_threat length of threat(s) in table threats
 * @param si slice index
 * @param n maximum number of moves until goal
 * @return true iff the defense defends against at least one of the
 *         threats
 */
boolean degenerate_tree_are_threats_refuted_in_n(table threats,
                                                 stip_length_type len_threat,
                                                 slice_index si,
                                                 stip_length_type n)
{
  boolean result = false;
  slice_index const next = slices[si].u.pipe.next;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",len_threat);
  TraceFunctionParam("%u",table_length(threats));
  TraceFunctionParam("%u",si);
  TraceFunctionParam("%u",n);
  TraceFunctionParamListEnd();

  result = direct_are_threats_refuted_in_n(threats,len_threat,next,n);

  TraceFunctionExit(__func__);
  TraceFunctionResult("%u",result);
  TraceFunctionResultEnd();
  return result;
}

/* **************** Stipulation instrumentation ***************
 */

static boolean degenerate_tree_inserter_branch_direct(slice_index si,
                                                      slice_traversal *st)
{
  boolean const result = true;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParamListEnd();

  slice_traverse_children(si,st);

  pipe_insert_before(si);
  init_degenerate_tree_guard_slice(si);

  TraceFunctionExit(__func__);
  TraceFunctionResult("%u",result);
  TraceFunctionResultEnd();
  return result;
}

static slice_operation const degenerate_tree_guards_inserters[] =
{
  &degenerate_tree_inserter_branch_direct,  /* STBranchDirect */
  &slice_traverse_children,                 /* STBranchDirectDefender */
  &slice_traverse_children,                 /* STBranchHelp */
  &slice_traverse_children,                 /* STBranchSeries */
  &slice_traverse_children,                 /* STBranchFork */
  &slice_traverse_children,                 /* STLeafDirect */
  &slice_traverse_children,                 /* STLeafHelp */
  &slice_traverse_children,                 /* STLeafForced */
  &slice_traverse_children,                 /* STReciprocal */
  &slice_traverse_children,                 /* STQuodlibet */
  &slice_traverse_children,                 /* STNot */
  &slice_traverse_children,                 /* STMoveInverter */
  &slice_traverse_children,                 /* STDirectRoot */
  &slice_traverse_children,                 /* STDirectDefenderRoot */
  &slice_traverse_children,                 /* STDirectHashed */
  &slice_traverse_children,                 /* STHelpRoot */
  &slice_traverse_children,                 /* STHelpAdapter */
  &slice_traverse_children,                 /* STHelpHashed */
  &slice_traverse_children,                 /* STSeriesRoot */
  &slice_traverse_children,                 /* STSeriesAdapter */
  &slice_traverse_children,                 /* STSeriesHashed */
  &slice_traverse_children,                 /* STSelfCheckGuard */
  &slice_traverse_children,                 /* STDirectDefense */
  &slice_traverse_children,                 /* STReflexGuard */
  &slice_traverse_children,                 /* STSelfAttack */
  &slice_traverse_children,                 /* STSelfDefense */
  &slice_traverse_children,                 /* STRestartGuard */
  &slice_traverse_children,                 /* STGoalReachableGuard */
  &slice_traverse_children,                 /* STKeepMatingGuard */
  &slice_traverse_children,                 /* STMaxFlightsquares */
  &slice_traverse_children,                 /* STDegenerateTree */
  &slice_traverse_children,                 /* STMaxNrNonTrivial */
  &slice_traverse_children                  /* STMaxThreatLength */
};

/* Instrument stipulation with STKeepMatingGuard slices
 */
void stip_insert_degenerate_tree_guards(void)
{
  slice_traversal st;

  TraceFunctionEntry(__func__);
  TraceFunctionParamListEnd();

  slice_traversal_init(&st,&degenerate_tree_guards_inserters,0);
  traverse_slices(root_slice,&st);

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}
