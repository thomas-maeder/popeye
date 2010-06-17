#include "pythreat.h"
#include "pydata.h"
#include "pypipe.h"
#include "stipulation/branch.h"
#include "stipulation/battle_play/branch.h"
#include "stipulation/battle_play/attack_play.h"
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

/* Determine the maximum number of half moves of the threat
 * @param n number of half moves remaining in the regular stipulation
 * @return maximum number of half moves of the threat
 */
static stip_length_type get_n_max(stip_length_type n)
{
  stip_length_type const result = 2*(max_len_threat-1)+slack_length_battle+2;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",n);
  TraceFunctionParamListEnd();

  assert(max_len_threat>0);

  TraceFunctionExit(__func__);
  TraceFunctionResult("%u",result);
  TraceFunctionResultEnd();
  return result;
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
  stip_length_type const parity = (n-slack_length_battle)%2;
  stip_length_type n_max_unsolvable;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParam("%u",n);
  TraceFunctionParamListEnd();

  n_max_unsolvable = slack_length_battle-parity;

  if (max_len_threat==0)
    result = !echecc(nbply,slices[si].starter);
  else
  {
    stip_length_type const n_max = get_n_max(n);
    if (n>=n_max)
    {
      slice_index const
          to_attacker = slices[si].u.maxthreatlength_guard.to_attacker;
      stip_length_type n_min = battle_branch_calc_n_min(si,n_max);
      stip_length_type const
          nr_moves_needed = attack_has_solution_in_n(to_attacker,
                                                     n_max-1,n_min-1,
                                                     n_max_unsolvable-1);
      result = nr_moves_needed>n_max;
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

/* Allocate a STMaxThreatLength slice
 * @param length maximum number of half moves until goal
 * @param to_attacker identifies slice leading to attacker
 */
static slice_index alloc_maxthreatlength_guard(stip_length_type length,
                                               slice_index to_attacker)
{
  slice_index result;
  stip_length_type const parity = (length-slack_length_battle)%2;
  stip_length_type const min_length = slack_length_battle+parity;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",length);
  TraceFunctionParam("%u",to_attacker);
  TraceFunctionParamListEnd();

  result = alloc_branch(STMaxThreatLength,length,min_length);
  slices[result].u.maxthreatlength_guard.to_attacker = to_attacker;

  TraceFunctionExit(__func__);
  TraceFunctionResult("%u",result);
  TraceFunctionResultEnd();
  return result;
}


/* **************** Implementation of interface DirectDefender **********
 */

/* Try to defend after an attacking move
 * When invoked with some n, the function assumes that the key doesn't
 * solve in less than n half moves.
 * @param si slice index
 * @param n maximum number of half moves until end state has to be reached
 * @param n_min minimum number of half-moves of interesting variations
 *              (slack_length_battle <= n_min <= slices[si].u.branch.length)
 * @param n_max_unsolvable maximum number of half-moves that we
 *                         know have no solution
 * @return <=n solved  - return value is maximum number of moves
 *                       (incl. defense) needed
 *         n+2 refuted - acceptable number of refutations found
 *         n+4 refuted - more refutations found than acceptable
 */
stip_length_type
maxthreatlength_guard_defend_in_n(slice_index si,
                                  stip_length_type n,
                                  stip_length_type n_min,
                                  stip_length_type n_max_unsolvable)
{
  slice_index const next = slices[si].u.pipe.next;
  stip_length_type result;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParam("%u",n);
  TraceFunctionParam("%u",n_min);
  TraceFunctionParam("%u",n_max_unsolvable);
  TraceFunctionParamListEnd();

  assert(!is_threat_too_long(si,n));
  result = defense_defend_in_n(next,n,n_min,n_max_unsolvable);

  TraceFunctionExit(__func__);
  TraceFunctionResult("%u",result);
  TraceFunctionResultEnd();
  return result;
}

/* Determine whether there are defenses after an attacking move
 * @param si slice index
 * @param n maximum number of half moves until end state has to be reached
 * @param n_max_unsolvable maximum number of half-moves that we
 *                         know have no solution
 * @param max_nr_refutations how many refutations should we look for
 * @return <=n solved  - return value is maximum number of moves
 *                       (incl. defense) needed
 *         n+2 refuted - <=max_nr_refutations refutations found
 *         n+4 refuted - >max_nr_refutations refutations found
 */
stip_length_type
maxthreatlength_guard_can_defend_in_n(slice_index si,
                                      stip_length_type n,
                                      stip_length_type n_max_unsolvable,
                                      unsigned int max_nr_refutations)
{
  slice_index const next = slices[si].u.pipe.next;
  unsigned int result;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParam("%u",n);
  TraceFunctionParam("%u",n_max_unsolvable);
  TraceFunctionParam("%u",max_nr_refutations);
  TraceFunctionParamListEnd();

  if (is_threat_too_long(si,n))
    result = n+4;
  else
    result = defense_can_defend_in_n(next,
                                     n,n_max_unsolvable,
                                     max_nr_refutations);

  TraceFunctionExit(__func__);
  TraceFunctionResult("%u",result);
  TraceFunctionResultEnd();
  return result;
}


/* **************** Stipulation instrumentation ***************
 */

/* Insert a STMaxThreatLength slice before each defender slice
 * @param si identifies defender slice
 * @param st address of struct representing the traversal
 */
static void maxthreatlength_guard_inserter(slice_index si,
                                           stip_structure_traversal *st)
{
  stip_length_type const length = slices[si].u.branch.length;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParamListEnd();

  if (max_len_threat==0 || length>=get_n_max(length))
  {
    boolean * const inserted = st->param;
    slice_index const
        to_attacker = branch_find_slice(STSelfCheckGuardAttackerFilter,si);
    pipe_append(slices[si].prev,alloc_maxthreatlength_guard(length,to_attacker));
    *inserted = true;
  }

  stip_traverse_structure_children(si,st);

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

static stip_structure_visitor const maxthreatlength_guards_inserters[] =
{
  &stip_traverse_structure_children, /* STProxy */
  &stip_traverse_structure_children, /* STAttackMove */
  &maxthreatlength_guard_inserter,   /* STDefenseMove */
  &stip_traverse_structure_children, /* STHelpMove */
  &stip_traverse_structure_children, /* STHelpFork */
  &stip_traverse_structure_children, /* STSeriesMove */
  &stip_traverse_structure_children, /* STSeriesFork */
  &stip_traverse_structure_children, /* STLeafDirect */
  &stip_traverse_structure_children, /* STLeafHelp */
  &stip_traverse_structure_children, /* STLeafForced */
  &stip_traverse_structure_children, /* STReciprocal */
  &stip_traverse_structure_children, /* STQuodlibet */
  &stip_traverse_structure_children, /* STNot */
  &stip_traverse_structure_children, /* STMoveInverterRootSolvableFilter */
  &stip_traverse_structure_children, /* STMoveInverterSolvableFilter */
  &stip_traverse_structure_children, /* STMoveInverterSeriesFilter */
  &stip_traverse_structure_children, /* STAttackRoot */
  &stip_traverse_structure_children, /* STDefenseRoot */
  &stip_traverse_structure_children, /* STPostKeyPlaySuppressor */
  &stip_traverse_structure_children, /* STContinuationSolver */
  &stip_traverse_structure_children, /* STContinuationWriter */
  &stip_traverse_structure_children, /* STBattlePlaySolver */
  &stip_traverse_structure_children, /* STBattlePlaySolutionWriter */
  &stip_traverse_structure_children, /* STThreatSolver */
  &stip_traverse_structure_children, /* STZugzwangWriter */
  &stip_traverse_structure_children, /* STThreatEnforcer */
  &stip_traverse_structure_children, /* STThreatCollector */
  &stip_traverse_structure_children, /* STRefutationsCollector */
  &stip_traverse_structure_children, /* STVariationWriter */
  &stip_traverse_structure_children, /* STRefutingVariationWriter */
  &stip_traverse_structure_children, /* STNoShortVariations */
  &stip_traverse_structure_children, /* STAttackHashed */
  &stip_traverse_structure_children, /* STHelpRoot */
  &stip_traverse_structure_children, /* STHelpShortcut */
  &stip_traverse_structure_children, /* STHelpHashed */
  &stip_traverse_structure_children, /* STSeriesRoot */
  &stip_traverse_structure_children, /* STSeriesShortcut */
  &stip_traverse_structure_children, /* STParryFork */
  &stip_traverse_structure_children, /* STSeriesHashed */
  &stip_traverse_structure_children, /* STSelfCheckGuardRootSolvableFilter */
  &stip_traverse_structure_children, /* STSelfCheckGuardSolvableFilter */
  &stip_traverse_structure_children, /* STSelfCheckGuardAttackerFilter */
  &stip_traverse_structure_children, /* STSelfCheckGuardDefenderFilter */
  &stip_traverse_structure_children, /* STSelfCheckGuardHelpFilter */
  &stip_traverse_structure_children, /* STSelfCheckGuardSeriesFilter */
  &stip_traverse_structure_children, /* STDirectDefenderFilter */
  &stip_traverse_structure_children, /* STReflexRootFilter */
  &stip_traverse_structure_children, /* STReflexHelpFilter */
  &stip_traverse_structure_children, /* STReflexSeriesFilter */
  &stip_traverse_structure_children, /* STReflexAttackerFilter */
  &stip_traverse_structure_children, /* STReflexDefenderFilter */
  &stip_traverse_structure_children, /* STSelfDefense */
  &stip_traverse_structure_children, /* STRestartGuardRootDefenderFilter */
  &stip_traverse_structure_children, /* STRestartGuardHelpFilter */
  &stip_traverse_structure_children, /* STRestartGuardSeriesFilter */
  &stip_traverse_structure_children, /* STIntelligentHelpFilter */
  &stip_traverse_structure_children, /* STIntelligentSeriesFilter */
  &stip_traverse_structure_children, /* STGoalReachableGuardHelpFilter */
  &stip_traverse_structure_children, /* STGoalReachableGuardSeriesFilter */
  &stip_traverse_structure_children, /* STKeepMatingGuardAttackerFilter */
  &stip_traverse_structure_children, /* STKeepMatingGuardDefenderFilter */
  &stip_traverse_structure_children, /* STKeepMatingGuardHelpFilter */
  &stip_traverse_structure_children, /* STKeepMatingGuardSeriesFilter */
  &stip_traverse_structure_children, /* STMaxFlightsquares */
  &stip_traverse_structure_children, /* STDegenerateTree */
  &stip_traverse_structure_children, /* STMaxNrNonTrivial */
  &stip_traverse_structure_children, /* STMaxNrNonTrivialCounter */
  &stip_traverse_structure_children, /* STMaxThreatLength */
  &stip_traverse_structure_children, /* STMaxTimeRootDefenderFilter */
  &stip_traverse_structure_children, /* STMaxTimeDefenderFilter */
  &stip_traverse_structure_children, /* STMaxTimeHelpFilter */
  &stip_traverse_structure_children, /* STMaxTimeSeriesFilter */
  &stip_traverse_structure_children, /* STMaxSolutionsRootSolvableFilter */
  &stip_traverse_structure_children, /* STMaxSolutionsRootDefenderFilter */
  &stip_traverse_structure_children, /* STMaxSolutionsHelpFilter */
  &stip_traverse_structure_children, /* STMaxSolutionsSeriesFilter */
  &stip_traverse_structure_children, /* STStopOnShortSolutionsRootSolvableFilter */
  &stip_traverse_structure_children, /* STStopOnShortSolutionsHelpFilter */
  &stip_traverse_structure_children, /* STStopOnShortSolutionsSeriesFilter */
  &stip_traverse_structure_children, /* STEndOfPhaseWriter */
  &stip_traverse_structure_children, /* STEndOfSolutionWriter */
  &stip_traverse_structure_children, /* STRefutationWriter */
  &stip_traverse_structure_children, /* STOutputPlaintextTreeCheckDetectorAttackerFilter */
  &stip_traverse_structure_children  /* STOutputPlaintextTreeCheckDetectorDefenderFilter */
};

/* Instrument stipulation with STMaxThreatLength slices
 */
boolean stip_insert_maxthreatlength_guards(void)
{
  boolean result;
  stip_structure_traversal st;

  TraceFunctionEntry(__func__);
  TraceFunctionParamListEnd();

  TraceStipulation(root_slice);

  stip_structure_traversal_init(&st,&maxthreatlength_guards_inserters,&result);
  stip_traverse_structure(root_slice,&st);

  TraceFunctionExit(__func__);
  TraceFunctionResult("%u",result);
  TraceFunctionResultEnd();
  return result;
}
