#include "pynontrv.h"
#include "pydata.h"
#include "pypipe.h"
#include "pydirect.h"
#include "pyoutput.h"
#include "stipulation/branch.h"
#include "trace.h"

#include <assert.h>
#include <stdlib.h>

static stip_length_type min_length_nontrivial;
unsigned int max_nr_nontrivial;

/* Reset the non-trivial optimisation setting to off
 */
void reset_nontrivial_settings(void)
{
  max_nr_nontrivial = UINT_MAX;
  min_length_nontrivial = 2*maxply+slack_length_direct;
}

/* Read the requested non-trivial optimisation settings from user input
 * @param tok text token from which to read maximum number of
 *            acceptable non-trivial variations (apart from main variation)
 * @return true iff setting was successfully read
 */
boolean read_max_nr_nontrivial(char const *tok)
{
  boolean result;
  char *end;
  unsigned long const requested_max_nr_nontrivial = strtoul(tok,&end,10);

  TraceFunctionEntry(__func__);
  TraceFunctionParamListEnd();

  TraceValue("%s\n",tok);

  if (tok!=end && requested_max_nr_nontrivial<=UINT_MAX)
  {
    result = true;
    max_nr_nontrivial = (unsigned int)requested_max_nr_nontrivial;
  }
  else
    result = false;

  TraceFunctionExit(__func__);
  TraceFunctionResult("%u",result);
  TraceFunctionResultEnd();
  return result;
}

/* Read the requested non-trivial optimisation settings from user input
 * @param tok text token from which to read minimimal length of what
 *            is to be considered a non-trivial variation
 * @return true iff setting was successfully read
 */
boolean read_min_length_nontrivial(char const *tok)
{
  boolean result;
  char *end;
  unsigned long const requested_min_length_nontrivial = strtoul(tok,&end,10);

  TraceFunctionEntry(__func__);
  TraceFunctionParamListEnd();

  TraceValue("%s\n",tok);

  if (tok!=end && requested_min_length_nontrivial<=UINT_MAX)
  {
    result = true;
    min_length_nontrivial = (2*(unsigned int)requested_min_length_nontrivial
                             +slack_length_direct);
  }
  else
    result = false;

  TraceFunctionExit(__func__);
  TraceFunctionResult("%u",result);
  TraceFunctionResultEnd();
  return result;
}

/* Retrieve the current minimum length (in full moves) of what is to
 * be considered a non-trivial variation
 * @return maximum acceptable number of non-trivial variations
 */
stip_length_type get_min_length_nontrivial(void)
{
  return (min_length_nontrivial-slack_length_direct)/2;
}


/* **************** Private helpers ***************
 */

/* Count non-trivial moves of the defending side. Whether a
 * particular move is non-trivial is determined by user input.
 * Stop counting when more than max_nr_nontrivial have been found
 * @return number of defender's non-trivial moves
 */
static unsigned int count_nontrivial_defenses(slice_index si)
{
  unsigned int result;
  slice_index const next = slices[si].u.pipe.next;
  stip_length_type const parity = slices[si].u.branch.length%2;
  unsigned int const nr_refutations_allowed = max_nr_nontrivial+1;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParamListEnd();

  if (min_length_nontrivial+parity==slack_length_direct)
  {
    /* TODO can this be moved between leaf and goal? */
    /* special case: just check for non-selfchecking moves
     */
    Side const attacker = slices[si].starter; 

    result = 0;

    genmove(attacker);

    while (encore() && result<=nr_refutations_allowed)
    {
      if (jouecoup(nbply,first_play) && !echecc(nbply,attacker))
        ++result;

      repcoup();
    }

    finply();
  }
  else
    result = direct_defender_can_defend_in_n(next,
                                             min_length_nontrivial+parity,
                                             nr_refutations_allowed);

  TraceFunctionExit(__func__);
  TraceFunctionResult("%u",result);
  TraceFunctionResultEnd();
  return result;
}

/* **************** Initialisation ***************
 */

/* Allocate a STMaxFlightsquares slice
 * @param length maximum number of half moves until goal
 * @return identifier of allocated slice
 */
static slice_index alloc_max_nr_nontrivial_guard(stip_length_type length)
{
  slice_index result;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",length);
  TraceFunctionParamListEnd();

  result = alloc_branch(STMaxNrNonTrivial,length,0);

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
boolean max_nr_nontrivial_guard_root_defend(slice_index si)
{
  boolean result;
  stip_length_type const n = slices[si].u.branch.length;
  slice_index const next = slices[si].u.pipe.next;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParamListEnd();

  if (n>min_length_nontrivial)
  {
    unsigned int const nr_nontrivial = count_nontrivial_defenses(si);
    if (max_nr_nontrivial+1>=nr_nontrivial)
    {
      ++max_nr_nontrivial;
      max_nr_nontrivial -= nr_nontrivial;
      result = direct_defender_root_defend(next);
      max_nr_nontrivial += nr_nontrivial;
      --max_nr_nontrivial;
    }
    else
      result = true;
  }
  else
    result = direct_defender_root_defend(next);

  TraceFunctionExit(__func__);
  TraceFunctionResult("%u",result);
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
boolean max_nr_nontrivial_guard_defend_in_n(slice_index si, stip_length_type n)
{
  slice_index const next = slices[si].u.pipe.next;
  boolean result;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParam("%u",n);
  TraceFunctionParamListEnd();

  if (n>min_length_nontrivial)
  {
    unsigned int const nr_nontrivial = count_nontrivial_defenses(si);
    if (max_nr_nontrivial+1>=nr_nontrivial)
    {
      ++max_nr_nontrivial;
      max_nr_nontrivial -= nr_nontrivial;
      result = direct_defender_defend_in_n(next,n);
      max_nr_nontrivial += nr_nontrivial;
      --max_nr_nontrivial;
    }
    else
      result = true;
  }
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
unsigned int max_nr_nontrivial_guard_can_defend_in_n(slice_index si,
                                                     stip_length_type n,
                                                     unsigned int max_result)
{
  slice_index const next = slices[si].u.pipe.next;
  unsigned int result;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParam("%u",n);
  TraceFunctionParamListEnd();

  if (n>min_length_nontrivial)
  {
    unsigned int const nr_nontrivial = count_nontrivial_defenses(si);
    if (max_nr_nontrivial+1>=nr_nontrivial)
    {
      ++max_nr_nontrivial;
      max_nr_nontrivial -= nr_nontrivial;
      result = direct_defender_can_defend_in_n(next,n,max_result);
      max_nr_nontrivial += nr_nontrivial;
      --max_nr_nontrivial;
    }
    else
      result = max_result+1;
  }
  else
    result = direct_defender_can_defend_in_n(next,n,max_result);

  TraceFunctionExit(__func__);
  TraceFunctionResult("%u",result);
  TraceFunctionResultEnd();
  return result;
}


/* **************** Stipulation instrumentation ***************
 */

static boolean nontrivial_guard_inserter_branch_direct(slice_index si,
                                                       slice_traversal *st)
{
  boolean const result = true;
  slice_index guard;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParamListEnd();

  slice_traverse_children(si,st);

  guard = alloc_max_nr_nontrivial_guard(slices[si].u.branch.length-1);
  pipe_link(guard,slices[si].u.pipe.next);
  pipe_link(si,guard);

  TraceFunctionExit(__func__);
  TraceFunctionResult("%u",result);
  TraceFunctionResultEnd();
  return result;
}

static slice_operation const max_nr_nontrivial_guards_inserters[] =
{
  &slice_traverse_children,                 /* STProxy */
  &nontrivial_guard_inserter_branch_direct, /* STBranchDirect */
  &slice_traverse_children,                 /* STBranchDirectDefender */
  &slice_traverse_children,                 /* STBranchHelp */
  &slice_traverse_children,                 /* STHelpFork */
  &slice_traverse_children,                 /* STSeriesMove */
  &slice_traverse_children,                 /* STSeriesFork */
  &slice_traverse_children,                 /* STLeafDirect */
  &slice_traverse_children,                 /* STLeafHelp */
  &slice_traverse_children,                 /* STLeafForced */
  &slice_traverse_children,                 /* STReciprocal */
  &slice_traverse_children,                 /* STQuodlibet */
  &slice_traverse_children,                 /* STNot */
  &slice_traverse_children,                 /* STMoveInverterRootSolvableFilter */
  &slice_traverse_children,                 /* STMoveInverterSolvableFilter */
  &slice_traverse_children,                 /* STMoveInverterSeriesFilter */
  &nontrivial_guard_inserter_branch_direct, /* STDirectRoot */
  &slice_traverse_children,                 /* STDirectDefenderRoot */
  &slice_traverse_children,                 /* STDirectHashed */
  &slice_traverse_children,                 /* STHelpRoot */
  &slice_traverse_children,                 /* STHelpShortcut */
  &slice_traverse_children,                 /* STHelpHashed */
  &slice_traverse_children,                 /* STSeriesRoot */
  &slice_traverse_children,                 /* STSeriesShortcut */
  &slice_traverse_children,                 /* STParryFork */
  &slice_traverse_children,                 /* STSeriesHashed */
  &slice_traverse_children,                 /* STSelfCheckGuardRootSolvableFilter */
  &slice_traverse_children,                 /* STSelfCheckGuardSolvableFilter */
  &slice_traverse_children,                 /* STSelfCheckGuardRootDefenderFilter */
  &slice_traverse_children,                 /* STSelfCheckGuardAttackerFilter */
  &slice_traverse_children,                 /* STSelfCheckGuardDefenderFilter */
  &slice_traverse_children,                 /* STSelfCheckGuardHelpFilter */
  &slice_traverse_children,                 /* STSelfCheckGuardSeriesFilter */
  &slice_traverse_children,                 /* STDirectDefense */
  &slice_traverse_children,                 /* STReflexHelpFilter */
  &slice_traverse_children,                 /* STReflexSeriesFilter */
  &slice_traverse_children,                 /* STReflexAttackerFilter */
  &slice_traverse_children,                 /* STReflexDefenderFilter */
  &slice_traverse_children,                 /* STSelfAttack */
  &slice_traverse_children,                 /* STSelfDefense */
  &slice_traverse_children,                 /* STRestartGuardRootDefenderFilter */
  &slice_traverse_children,                 /* STRestartGuardHelpFilter */
  &slice_traverse_children,                 /* STRestartGuardSeriesFilter */
  &slice_traverse_children,                 /* STIntelligentHelpFilter */
  &slice_traverse_children,                 /* STIntelligentSeriesFilter */
  &slice_traverse_children,                 /* STGoalReachableGuardHelpFilter */
  &slice_traverse_children,                 /* STGoalReachableGuardSeriesFilter */
  &slice_traverse_children,                 /* STKeepMatingGuardRootDefenderFilter */
  &slice_traverse_children,                 /* STKeepMatingGuardAttackerFilter */
  &slice_traverse_children,                 /* STKeepMatingGuardDefenderFilter */
  &slice_traverse_children,                 /* STKeepMatingGuardHelpFilter */
  &slice_traverse_children,                 /* STKeepMatingGuardSeriesFilter */
  &slice_traverse_children,                 /* STMaxFlightsquares */
  &slice_traverse_children,                 /* STDegenerateTree */
  &slice_traverse_children,                 /* STMaxNrNonTrivial */
  &slice_traverse_children,                 /* STMaxThreatLength */
  &slice_traverse_children,                 /* STMaxTimeRootDefenderFilter */
  &slice_traverse_children,                 /* STMaxTimeDefenderFilter */
  &slice_traverse_children,                 /* STMaxTimeHelpFilter */
  &slice_traverse_children,                 /* STMaxTimeSeriesFilter */
  &slice_traverse_children,                 /* STMaxSolutionsRootSolvableFilter */
  &slice_traverse_children,                 /* STMaxSolutionsRootDefenderFilter */
  &slice_traverse_children,                 /* STMaxSolutionsHelpFilter */
  &slice_traverse_children,                 /* STMaxSolutionsSeriesFilter */
  &slice_traverse_children,                 /* STStopOnShortSolutionsRootSolvableFilter */
  &slice_traverse_children,                 /* STStopOnShortSolutionsHelpFilter */
  &slice_traverse_children                  /* STStopOnShortSolutionsSeriesFilter */
};

/* Instrument stipulation with STMaxNrNonTrivial slices
 */
void stip_insert_max_nr_nontrivial_guards(void)
{
  slice_traversal st;

  TraceFunctionEntry(__func__);
  TraceFunctionParamListEnd();

  TraceStipulation(root_slice);

  slice_traversal_init(&st,&max_nr_nontrivial_guards_inserters,0);
  traverse_slices(root_slice,&st);

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}
