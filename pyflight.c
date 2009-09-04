#include "pyflight.h"
#include "pydata.h"
#include "pypipe.h"
#include "pydirect.h"
#include "trace.h"

#include <assert.h>
#include <stdlib.h>

static unsigned int max_nr_flights;

/* Reset the max flights setting to off
 */
void reset_max_flights(void)
{
  max_nr_flights = INT_MAX;
}

/* Read the requested max flight setting from a text token entered by
 * the user
 * @param textToken text token from which to read
 * @return true iff max flight setting was successfully read
 */
boolean read_max_flights(const char *textToken)
{
  boolean result;
  char *end;
  unsigned long const requested_max_nr_flights = strtoul(textToken,&end,10);

  if (textToken!=end && requested_max_nr_flights<=UINT_MAX)
  {
    max_nr_flights = (unsigned int)requested_max_nr_flights;
    result = true;
  }
  else
    result = false;

  return result;
}

/* Retrieve the current max flights setting
 * @return current max flights setting
 *         UINT_MAX if max flights option is not active
 */
unsigned int get_max_flights(void)
{
  return max_nr_flights;
}

/* **************** Private helpers ***************
 */

/* Determine whether the defending side has more flights than allowed
 * by the user.
 * @param defender defending side
 * @return true iff the defending side has too many flights.
 */
static boolean has_too_many_flights(Side defender)
{
  boolean result;
  square const save_rbn = defender==Black ? rn : rb;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",defender);
  TraceFunctionParamListEnd();

  if (save_rbn==initsquare)
    result = false;
  else
  {
    unsigned int number_flights_left = max_nr_flights+1;

    genmove(defender);

    while (encore() && number_flights_left>0)
    {
      if (jouecoup(nbply,first_play))
      {
        square const rbn = defender==Black ? rn : rb;
        if (save_rbn!=rbn && !echecc(nbply,defender))
          --number_flights_left;
      }

      repcoup();
    }

    finply();

    result = number_flights_left==0;
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
static void init_maxflight_guard_slice(slice_index si)
{
  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParamListEnd();

  slices[si].type = STMaxFlightsquares; 
  slices[si].starter = no_side; 

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}


/* **************** Implementation of interface DirectDefender **********
 */

/* Try to defend after an attempted key move at root level
 * @param si slice index
 * @return true iff the defender can successfully defend
 */
boolean maxflight_guard_root_defend(slice_index si)
{
  boolean result;
  Side const defender = slices[si].starter;
  stip_length_type const n = slices[si].u.pipe.u.branch.length;
  slice_index const next = slices[si].u.pipe.next;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParamListEnd();

  if (n-1>slack_length_direct+2 && has_too_many_flights(defender))
    result = true;
  else
    result = direct_defender_root_defend(next);

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
boolean maxflight_guard_defend_in_n(slice_index si,
                                     stip_length_type n,
                                     unsigned int curr_max_nr_nontrivial)
{
  Side const defender = slices[si].starter;
  slice_index const next = slices[si].u.pipe.next;
  boolean result;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParam("%u",n);
  TraceFunctionParamListEnd();

  assert(n%2==slices[si].u.pipe.u.branch.length%2);

  if (n>slack_length_direct+2 && has_too_many_flights(defender))
    result = true;
  else
    result = direct_defender_defend_in_n(next,n,curr_max_nr_nontrivial);

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
boolean maxflight_guard_can_defend_in_n(slice_index si,
                                        stip_length_type n,
                                        unsigned int curr_max_nr_nontrivial)
{
  Side const defender = slices[si].starter;
  slice_index const next = slices[si].u.pipe.next;
  boolean result;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParam("%u",n);
  TraceFunctionParam("%d",curr_max_nr_nontrivial);
  TraceFunctionParamListEnd();

  if (n>slack_length_direct+2 && has_too_many_flights(defender))
    result = true;
  else
    result = direct_defender_can_defend_in_n(next,n,curr_max_nr_nontrivial);

  TraceFunctionExit(__func__);
  TraceFunctionResult("%u",result);
  TraceFunctionResultEnd();
  return result;
}


/* **************** Stipulation instrumentation ***************
 */

static boolean maxflight_guard_inserter(slice_index si,slice_traversal *st)
{
  boolean const result = true;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParamListEnd();

  slice_traverse_children(si,st);

  pipe_insert_before(si);
  init_maxflight_guard_slice(si);

  TraceFunctionExit(__func__);
  TraceFunctionResult("%u",result);
  TraceFunctionResultEnd();
  return result;
}

static slice_operation const maxflight_guards_inserters[] =
{
  &slice_traverse_children,  /* STBranchDirect */
  &maxflight_guard_inserter, /* STBranchDirectDefender */
  &slice_traverse_children,  /* STBranchHelp */
  &slice_traverse_children,  /* STBranchSeries */
  &slice_traverse_children,  /* STBranchFork */
  &slice_traverse_children,  /* STLeafDirect */
  &slice_traverse_children,  /* STLeafHelp */
  &slice_traverse_children,  /* STLeafForced */
  &slice_traverse_children,  /* STReciprocal */
  &slice_traverse_children,  /* STQuodlibet */
  &slice_traverse_children,  /* STNot */
  &slice_traverse_children,  /* STMoveInverter */
  &slice_traverse_children,  /* STDirectRoot */
  &maxflight_guard_inserter, /* STDirectDefenderRoot */
  &slice_traverse_children,  /* STDirectHashed */
  &slice_traverse_children,  /* STHelpRoot */
  &slice_traverse_children,  /* STHelpAdapter */
  &slice_traverse_children,  /* STHelpHashed */
  &slice_traverse_children,  /* STSeriesRoot */
  &slice_traverse_children,  /* STSeriesAdapter */
  &slice_traverse_children,  /* STSeriesHashed */
  &slice_traverse_children,  /* STSelfCheckGuard */
  &slice_traverse_children,  /* STDirectAttack */
  &slice_traverse_children,  /* STDirectDefense */
  &slice_traverse_children,  /* STReflexGuard */
  &slice_traverse_children,  /* STSelfAttack */
  &slice_traverse_children,  /* STSelfDefense */
  &slice_traverse_children,  /* STRestartGuard */
  &slice_traverse_children,  /* STGoalReachableGuard */
  &slice_traverse_children,  /* STKeepMatingGuard */
  &slice_traverse_children,  /* STMaxFlightsquares */
  &slice_traverse_children,  /* STMaxNrNonTrivial */
  &slice_traverse_children   /* STMaxThreatLength */
};

/* Instrument stipulation with STKeepMatingGuard slices
 */
void stip_insert_maxflight_guards(void)
{
  slice_traversal st;

  TraceFunctionEntry(__func__);
  TraceFunctionParamListEnd();

  TraceStipulation();

  slice_traversal_init(&st,&maxflight_guards_inserters,0);
  traverse_slices(root_slice,&st);

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}
