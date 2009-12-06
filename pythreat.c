#include "pythreat.h"
#include "pydata.h"
#include "pypipe.h"
#include "pydirect.h"
#include "trace.h"

#include <assert.h>
#include <stdlib.h>

static stip_length_type max_len_threat;

/* Reset the max threats setting to off
 */
void reset_max_threat_length(void)
{
  max_len_threat = no_stip_length;
}

/* Read the requested max threat length setting from a text token
 * entered by the user
 * @param textToken text token from which to read
 * @return true iff max threat setting was successfully read
 */
boolean read_max_threat_length(const char *textToken)
{
  boolean result;
  char *end;
  unsigned long const requested_max_threat_length = strtoul(textToken,&end,10);

  if (textToken!=end && requested_max_threat_length<=UINT_MAX)
  {
    max_len_threat = (stip_length_type)requested_max_threat_length;
    result = true;
  }
  else
    result = false;

  return result;
}

/* Retrieve the current max threat length setting
 * @return current max threat length setting
 *         no_stip_length if max threats option is not active
 */
stip_length_type get_max_threat_length(void)
{
  TraceFunctionEntry(__func__);
  TraceFunctionParamListEnd();

  TraceFunctionExit(__func__);
  TraceFunctionResult("%u",max_len_threat);
  TraceFunctionResultEnd();
  return max_len_threat;
}

/* **************** Private helpers ***************
 */

/* Determine whether the threat after the attacker's move just played
 * is too long respective to user input.
 * @param si slice index
 * @param n maximum number of half moves until goal
 * @return true iff threat is too long
 */
static boolean is_threat_too_long(slice_index si, stip_length_type n)
{
  boolean result;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParam("%u",n);
  TraceFunctionParamListEnd();

  assert(n%2==slices[si].u.pipe.u.branch.length%2);

  if (max_len_threat==0)
    result = !echecc(nbply,slices[si].starter);
  else
  {
    slice_index const
        to_attacker = slices[si].u.pipe.u.maxthreatlength_guard.to_attacker;
    stip_length_type const parity = (n-1)%2;
    stip_length_type const n_max = 2*max_len_threat+parity;

    if (n-1>=n_max)
    {
      stip_length_type const length = slices[si].u.pipe.u.branch.length;
      stip_length_type const
          min_length = slices[si].u.pipe.u.branch.min_length;
      stip_length_type n_min;

      if (n_max+min_length>slack_length_direct+length)
        n_min = n_max-(length-min_length);
      else
        n_min = slack_length_direct-parity;

      result = direct_has_solution_in_n(to_attacker,n_max,n_min)>n_max;
    }
    else
      /* remainder of play is too short for max_len_threat to apply */
      result = false;
  }

  TraceFunctionExit(__func__);
  TraceFunctionResult("%u",result);
  TraceFunctionResultEnd();
  return result;
}

/* **************** Initialisation ***************
 */

/* Initialise a STMaxFlightsquares slice
 * @param si identifies slice to be initialised
 * @param side mating side
 */
static void init_maxthreatlength_guard_slice(slice_index si,
                                             slice_index to_attacker)
{
  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParamListEnd();

  slices[si].type = STMaxThreatLength; 
  slices[si].starter = no_side; 
  slices[si].u.pipe.u.maxthreatlength_guard.to_attacker = to_attacker; 

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}


/* **************** Implementation of interface DirectDefender **********
 */

/* Solve a slice at root level
 * @param si slice index
 * @return true iff >=1 solution was found and written
 */
boolean maxthreatlength_guard_root_solve(slice_index si)
{
  boolean result;
  stip_length_type const max_threat_length = slices[si].u.pipe.u.maxthreatlength_guard.length;
  slice_index const next = slices[si].u.pipe.next;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParamListEnd();

  if (is_threat_too_long(si,max_threat_length))
    result = false;
  else
    result = direct_root_solve_in_n(next);

  TraceFunctionExit(__func__);
  TraceFunctionResult("%u",result);
  TraceFunctionResultEnd();
  return result;
}

/* Try to defend after an attempted key move at root level
 * @param si slice index
 * @return true iff the defending side can successfully defend
 */
boolean maxthreatlength_guard_root_defend(slice_index si)
{
  boolean result;
  stip_length_type const n = slices[si].u.pipe.u.maxthreatlength_guard.length;
  slice_index const next = slices[si].u.pipe.next;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParamListEnd();

  if (is_threat_too_long(si,n))
    result = true;
  else
    result = direct_defender_root_defend(next);

  TraceFunctionExit(__func__);
  TraceValue("%u",result);
  TraceFunctionResultEnd();
  return result;
}

/* Try to defend after an attempted key move at non-root level.
 * When invoked with some n, the function assumes that the key doesn't
 * solve in less than n half moves.
 * @param si slice index
 * @param n maximum number of half moves until end state has to be reached
 * @return true iff the defender can defend
 */
boolean maxthreatlength_guard_defend_in_n(slice_index si, stip_length_type n)
{
  slice_index const next = slices[si].u.pipe.next;
  boolean result;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParam("%u",n);
  TraceFunctionParamListEnd();

  if (is_threat_too_long(si,n))
    result = true;
  else
    result = direct_defender_defend_in_n(next,n);

  TraceFunctionExit(__func__);
  TraceFunctionResult("%u",result);
  TraceFunctionResultEnd();
  return result;
}

/* Determine whether there are refutations after an attempted key move
 * at non-root level
 * @param si slice index
 * @param n maximum number of half moves until end state has to be reached
 * @param max_result how many refutations should we look for
 * @return number of refutations found (0..max_result+1)
 */
unsigned int maxthreatlength_guard_can_defend_in_n(slice_index si,
                                                   stip_length_type n,
                                                   unsigned int max_result)
{
  slice_index const next = slices[si].u.pipe.next;
  unsigned int result;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParam("%u",n);
  TraceFunctionParamListEnd();

  if (is_threat_too_long(si,n))
    result = max_result+1;
  else
    result = direct_defender_can_defend_in_n(next,n,max_result);

  TraceFunctionExit(__func__);
  TraceFunctionResult("%u",result);
  TraceFunctionResultEnd();
  return result;
}


/* **************** Stipulation instrumentation ***************
 */

static boolean maxthreatlength_guard_inserter(slice_index si,slice_traversal *st)
{
  boolean const result = true;
  slice_index const to_attacker = slices[si].u.pipe.next;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParamListEnd();

  slice_traverse_children(si,st);

  pipe_insert_before(si);
  init_maxthreatlength_guard_slice(si,to_attacker);

  TraceFunctionExit(__func__);
  TraceFunctionResult("%u",result);
  TraceFunctionResultEnd();
  return result;
}

static slice_operation const maxthreatlength_guards_inserters[] =
{
  &slice_traverse_children,        /* STBranchDirect */
  &maxthreatlength_guard_inserter, /* STBranchDirectDefender */
  &slice_traverse_children,        /* STBranchHelp */
  &slice_traverse_children,        /* STBranchSeries */
  &slice_traverse_children,        /* STBranchFork */
  &slice_traverse_children,        /* STLeafDirect */
  &slice_traverse_children,        /* STLeafHelp */
  &slice_traverse_children,        /* STLeafForced */
  &slice_traverse_children,        /* STReciprocal */
  &slice_traverse_children,        /* STQuodlibet */
  &slice_traverse_children,        /* STNot */
  &slice_traverse_children,        /* STMoveInverter */
  &slice_traverse_children,        /* STDirectRoot */
  &maxthreatlength_guard_inserter, /* STDirectDefenderRoot */
  &slice_traverse_children,        /* STDirectHashed */
  &slice_traverse_children,        /* STHelpRoot */
  &slice_traverse_children,        /* STHelpHashed */
  &slice_traverse_children,        /* STSeriesRoot */
  &slice_traverse_children,        /* STSeriesHashed */
  &slice_traverse_children,        /* STSelfCheckGuard */
  &slice_traverse_children,        /* STDirectDefense */
  &slice_traverse_children,        /* STReflexGuard */
  &slice_traverse_children,        /* STSelfAttack */
  &slice_traverse_children,        /* STSelfDefense */
  &slice_traverse_children,        /* STRestartGuard */
  &slice_traverse_children,        /* STGoalReachableGuard */
  &slice_traverse_children,        /* STKeepMatingGuard */
  &slice_traverse_children,        /* STMaxFlightsquares */
  &slice_traverse_children,        /* STDegenerateTree */
  &slice_traverse_children,        /* STMaxNrNonTrivial */
  &slice_traverse_children         /* STMaxThreatLength */
};

/* Instrument stipulation with STKeepMatingGuard slices
 */
void stip_insert_maxthreatlength_guards(void)
{
  slice_traversal st;

  TraceFunctionEntry(__func__);
  TraceFunctionParamListEnd();

  TraceStipulation();

  slice_traversal_init(&st,&maxthreatlength_guards_inserters,0);
  traverse_slices(root_slice,&st);

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}
