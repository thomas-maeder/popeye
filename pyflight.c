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
 * @param table table where to add refutations
 * @param si slice index
 * @return success of key move
 */
attack_result_type maxflight_guard_root_defend(table refutations, slice_index si)
{
  attack_result_type result;
  Side const defender = slices[si].starter;
  stip_length_type const n = slices[si].u.pipe.u.branch.length;
  slice_index const next = slices[si].u.pipe.next;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParamListEnd();

  if (n-1>slack_length_direct+2 && has_too_many_flights(defender))
    result = attack_has_reached_deadend;
  else
    result = direct_defender_root_defend(refutations,next);

  TraceFunctionExit(__func__);
  TraceEnumerator(attack_result_type,result,"");
  TraceFunctionResultEnd();
  return result;
}

/* Try to defend after an attempted key move at non-root level
 * @param si slice index
 * @param n maximum number of half moves until end state has to be reached
 * @return success of key move
 */
attack_result_type maxflight_guard_defend_in_n(slice_index si,
                                               stip_length_type n)
{
  Side const defender = slices[si].starter;
  slice_index const next = slices[si].u.pipe.next;
  attack_result_type result;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParam("%u",n);
  TraceFunctionParamListEnd();

  assert(n%2==slices[si].u.pipe.u.branch.length%2);

  if (n>slack_length_direct+2 && has_too_many_flights(defender))
    result = attack_has_reached_deadend;
  else
    result = direct_defender_defend_in_n(next,n);

  TraceFunctionExit(__func__);
  TraceEnumerator(attack_result_type,result,"");
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

/* Solve threats after an attacker's move
 * @param threats table where to add threats
 * @param si slice index
 * @param n maximum number of half moves until end state has to be reached
 * @return length of threats
 *         (n-slack_length_direct)%2 if the attacker has something
 *           stronger than threats (i.e. has delivered check)
 *         n+2 if there is no threat
 */
stip_length_type maxflight_guard_solve_threats(table threats,
                                               slice_index si,
                                               stip_length_type n)
{
  stip_length_type result;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParamListEnd();

  result = direct_defender_solve_threats(threats,slices[si].u.pipe.next,n);

  TraceFunctionExit(__func__);
  TraceFunctionResult("%u",result);
  TraceFunctionResultEnd();
  return result;
}

/* Solve variations after the move that has just been played at root level
 * @param threats table containing threats
 * @param len_threat length of threats
 * @param si slice index
 * @param n maximum length of variations to be solved
 * @return true iff >= 1 variation was found
 */
boolean maxflight_guard_solve_variations_in_n(table threats,
                                              stip_length_type len_threat,
                                              slice_index si,
                                              stip_length_type n)
{
  boolean result;
  slice_index const next = slices[si].u.pipe.next;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParamListEnd();

  result = direct_defender_solve_variations_in_n(threats,len_threat,next,n);

  TraceFunctionExit(__func__);
  TraceFunctionResult("%u",result);
  TraceFunctionResultEnd();
  return result;
}

/* Solve variations after the move that has just been played at root level
 * @param threats table containing threats
 * @param len_threat length of threats
 * @param refutations table containing refutations to move just played
 * @param si slice index
 */
void maxflight_guard_root_solve_variations(table threats,
                                           stip_length_type len_threat,
                                           table refutations,
                                           slice_index si)
{
  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParamListEnd();

  direct_defender_root_solve_variations(threats,len_threat,
                                        refutations,
                                        slices[si].u.pipe.next);

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
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
  &slice_traverse_children,  /* STDegenerateTree */
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
