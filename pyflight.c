#include "pyflight.h"
#include "pydata.h"
#include "pypipe.h"
#include "pydirect.h"
#include "stipulation/branch.h"
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

  if (textToken!=end && requested_max_nr_flights<=nr_squares_on_board)
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
 * @param length maximum number of half moves until end of branch
 * @return identifier of allocated slice
 */
static slice_index alloc_maxflight_guard_slice(stip_length_type length)
{
  slice_index result;
  stip_length_type const parity = (length-slack_length_direct)%2;

  TraceFunctionEntry(__func__);
  TraceFunctionParamListEnd();

  result = alloc_branch(STMaxFlightsquares,length,slack_length_direct+parity);

  TraceFunctionExit(__func__);
  TraceFunctionResult("%u",result);
  TraceFunctionResultEnd();
  return result;
}


/* **************** Implementation of interface DirectDefender **********
 */

/* Try to defend after an attempted key move at root level
 * @param si slice index
 * @return true iff the defending side can successfully defend
 */
boolean maxflight_guard_root_defend(slice_index si)
{
  boolean result;
  Side const defender = slices[si].starter;
  stip_length_type const n = slices[si].u.branch.length;
  slice_index const next = slices[si].u.pipe.next;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParamListEnd();

  if (n-1>slack_length_direct+2 && has_too_many_flights(defender))
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
boolean maxflight_guard_defend_in_n(slice_index si, stip_length_type n)
{
  Side const defender = slices[si].starter;
  slice_index const next = slices[si].u.pipe.next;
  boolean result;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParam("%u",n);
  TraceFunctionParamListEnd();

  assert(n%2==slices[si].u.branch.length%2);

  if (n>slack_length_direct+2 && has_too_many_flights(defender))
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
unsigned int maxflight_guard_can_defend_in_n(slice_index si,
                                             stip_length_type n,
                                             unsigned int max_result)
{
  Side const defender = slices[si].starter;
  slice_index const next = slices[si].u.pipe.next;
  unsigned int result;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParam("%u",n);
  TraceFunctionParamListEnd();

  if (n>slack_length_direct+2 && has_too_many_flights(defender))
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

/* Insert a STMaxFlightsquares slice before each defender slice
 * @param si identifier defender slice
 * @param st address of structure representing the traversal
 */
static boolean maxflight_guard_inserter(slice_index si,slice_traversal *st)
{
  boolean const result = true;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParamListEnd();

  {
    slice_index const prev = slices[si].prev;
    stip_length_type const length = slices[si].u.branch.length;
    slice_index const guard = alloc_maxflight_guard_slice(length);
    pipe_link(prev,guard);
    pipe_link(guard,si);
    slice_traverse_children(si,st);
  }

  TraceFunctionExit(__func__);
  TraceFunctionResult("%u",result);
  TraceFunctionResultEnd();
  return result;
}

static slice_operation const maxflight_guards_inserters[] =
{
  &slice_traverse_children,  /* STProxy */
  &slice_traverse_children,  /* STBranchDirect */
  &maxflight_guard_inserter, /* STBranchDirectDefender */
  &slice_traverse_children,  /* STBranchHelp */
  &slice_traverse_children,  /* STHelpFork */
  &slice_traverse_children,  /* STBranchSeries */
  &slice_traverse_children,  /* STSeriesFork */
  &slice_traverse_children,  /* STLeafDirect */
  &slice_traverse_children,  /* STLeafHelp */
  &slice_traverse_children,  /* STLeafForced */
  &slice_traverse_children,  /* STReciprocal */
  &slice_traverse_children,  /* STQuodlibet */
  &slice_traverse_children,  /* STNot */
  &slice_traverse_children,  /* STMoveInverterRootSolvableFilter */
  &slice_traverse_children,  /* STMoveInverterSolvableFilter */
  &slice_traverse_children,  /* STMoveInverterSeriesFilter */
  &slice_traverse_children,  /* STDirectRoot */
  &maxflight_guard_inserter, /* STDirectDefenderRoot */
  &slice_traverse_children,  /* STDirectHashed */
  &slice_traverse_children,  /* STHelpRoot */
  &slice_traverse_children,  /* STHelpShortcut */
  &slice_traverse_children,  /* STHelpHashed */
  &slice_traverse_children,  /* STSeriesRoot */
  &slice_traverse_children,  /* STSeriesShortcut */
  &slice_traverse_children,  /* STParryFork */
  &slice_traverse_children,  /* STSeriesHashed */
  &slice_traverse_children,  /* STSelfCheckGuardRootSolvableFilter */
  &slice_traverse_children,  /* STSelfCheckGuardSolvableFilter */
  &slice_traverse_children,  /* STSelfCheckGuardRootDefenderFilter */
  &slice_traverse_children,  /* STSelfCheckGuardAttackerFilter */
  &slice_traverse_children,  /* STSelfCheckGuardDefenderFilter */
  &slice_traverse_children,  /* STSelfCheckGuardHelpFilter */
  &slice_traverse_children,  /* STSelfCheckGuardSeriesFilter */
  &slice_traverse_children,  /* STDirectDefense */
  &slice_traverse_children,  /* STReflexHelpFilter */
  &slice_traverse_children,  /* STReflexSeriesFilter */
  &slice_traverse_children,  /* STReflexAttackerFilter */
  &slice_traverse_children,  /* STReflexDefenderFilter */
  &slice_traverse_children,  /* STSelfAttack */
  &slice_traverse_children,  /* STSelfDefense */
  &slice_traverse_children,  /* STRestartGuardRootDefenderFilter */
  &slice_traverse_children,  /* STRestartGuardHelpFilter */
  &slice_traverse_children,  /* STRestartGuardSeriesFilter */
  &slice_traverse_children,  /* STGoalReachableGuardHelpFilter */
  &slice_traverse_children,  /* STGoalReachableGuardSeriesFilter */
  &slice_traverse_children,  /* STKeepMatingGuardRootDefenderFilter */
  &slice_traverse_children,  /* STKeepMatingGuardAttackerFilter */
  &slice_traverse_children,  /* STKeepMatingGuardDefenderFilter */
  &slice_traverse_children,  /* STKeepMatingGuardHelpFilter */
  &slice_traverse_children,  /* STKeepMatingGuardSeriesFilter */
  &slice_traverse_children,  /* STMaxFlightsquares */
  &slice_traverse_children,  /* STDegenerateTree */
  &slice_traverse_children,  /* STMaxNrNonTrivial */
  &slice_traverse_children   /* STMaxThreatLength */
};

/* Instrument stipulation with STMaxFlightsquares slices
 */
void stip_insert_maxflight_guards(void)
{
  slice_traversal st;

  TraceFunctionEntry(__func__);
  TraceFunctionParamListEnd();

  TraceStipulation(root_slice);

  slice_traversal_init(&st,&maxflight_guards_inserters,0);
  traverse_slices(root_slice,&st);

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}
