#include "pynontrv.h"
#include "pydata.h"
#include "pypipe.h"
#include "pyoutput.h"
#include "stipulation/branch.h"
#include "stipulation/battle_play/attack_play.h"
#include "stipulation/battle_play/defense_play.h"
#include "trace.h"

#include <assert.h>
#include <stdlib.h>

static stip_length_type min_length_nontrivial;
unsigned int max_nr_nontrivial;

/* Lengths of threats of the various move levels 
 */
static unsigned int non_trivial_count[maxply+1];

/* Reset the non-trivial optimisation setting to off
 */
void reset_nontrivial_settings(void)
{
  max_nr_nontrivial = UINT_MAX;
  min_length_nontrivial = 2*maxply+slack_length_battle;
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
  TraceFunctionParam("%s",tok);
  TraceFunctionParamListEnd();

  TraceValue("%s\n",tok);

  if (tok!=end && requested_min_length_nontrivial<=UINT_MAX)
  {
    result = true;
    min_length_nontrivial = (2*(unsigned int)requested_min_length_nontrivial
                             +slack_length_battle);
    TraceValue("%u\n",min_length_nontrivial);
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
  return (min_length_nontrivial-slack_length_battle)/2;
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
  stip_length_type const parity = ((slices[si].u.branch.length
                                    -slack_length_battle)
                                   %2);
  unsigned int const nr_refutations_allowed = max_nr_nontrivial+1;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParamListEnd();

  if (min_length_nontrivial+parity==slack_length_battle)
  {
    /* TODO can this be moved between leaf and goal? */
    /* special case: just check for non-selfchecking moves
     */
    Side const attacker = slices[si].starter; 

    result = 0;

    move_generation_mode = move_generation_not_optimized;
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
  {
    non_trivial_count[nbply+1] = 0;
    defense_can_defend_in_n(next,
                            min_length_nontrivial+parity,
                            nr_refutations_allowed);
    result = non_trivial_count[nbply+1];
  }

  TraceFunctionExit(__func__);
  TraceFunctionResult("%u",result);
  TraceFunctionResultEnd();
  return result;
}

/* **************** Initialisation ***************
 */

/* Allocate a STMaxNrNonTrivial slice
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

/* Allocate a STMaxNrNonTrivialCounter slice
 * @param length maximum number of half-moves of slice (+ slack)
 * @param min_length minimum number of half-moves of slice (+ slack)
 * @return identifier of allocated slice
 */
static slice_index alloc_max_nr_nontrivial_counter(stip_length_type length,
                                                   stip_length_type min_length)
{
  slice_index result;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",length);
  TraceFunctionParam("%u",min_length);
  TraceFunctionParamListEnd();

  result = alloc_branch(STMaxNrNonTrivialCounter,length,min_length);

  TraceFunctionExit(__func__);
  TraceFunctionResult("%u",result);
  TraceFunctionResultEnd();
  return result;
}


/* **************** Implementation of interface DirectDefender **********
 */

/* Try to defend after an attempted key move at root level
 * @param si slice index
 * @param n maximum number of half moves until end state has to be reached
 * @param n_min minimum number of half-moves of interesting variations
 *              (slack_length_battle <= n_min <= slices[si].u.branch.length)
 * @param max_nr_refutations how many refutations should we look for
 * @return <slack_length_battle - stalemate
 *         <=n solved  - return value is maximum number of moves
 *                       (incl. defense) needed
 *         n+2 refuted - <=max_nr_refutations refutations found
 *         n+4 refuted - >max_nr_refutations refutations found
 */
stip_length_type
max_nr_nontrivial_guard_root_defend(slice_index si,
                                    stip_length_type n,
                                    stip_length_type n_min,
                                    unsigned int max_nr_refutations)
{
  stip_length_type result;
  slice_index const next = slices[si].u.pipe.next;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParam("%u",n);
  TraceFunctionParam("%u",n_min);
  TraceFunctionParam("%u",max_nr_refutations);
  TraceFunctionParamListEnd();

  if (n>min_length_nontrivial)
  {
    unsigned int const nr_nontrivial = count_nontrivial_defenses(si);
    if (max_nr_nontrivial+1>=nr_nontrivial)
    {
      ++max_nr_nontrivial;
      max_nr_nontrivial -= nr_nontrivial;
      result = defense_root_defend(next,n,n_min,max_nr_refutations);
      max_nr_nontrivial += nr_nontrivial;
      --max_nr_nontrivial;
    }
    else
      result = n+4;
  }
  else
    result = defense_root_defend(next,n,n_min,max_nr_refutations);

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
 * @param n_min minimum number of half-moves of interesting variations
 *              (slack_length_battle <= n_min <= slices[si].u.branch.length)
 * @return true iff the defender can defend
 */
boolean max_nr_nontrivial_guard_defend_in_n(slice_index si,
                                            stip_length_type n,
                                            stip_length_type n_min)
{
  slice_index const next = slices[si].u.pipe.next;
  boolean result;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParam("%u",n);
  TraceFunctionParam("%u",n_min);
  TraceFunctionParamListEnd();

  if (n>min_length_nontrivial)
  {
    unsigned int const nr_nontrivial = count_nontrivial_defenses(si);
    if (max_nr_nontrivial+1>=nr_nontrivial)
    {
      ++max_nr_nontrivial;
      max_nr_nontrivial -= nr_nontrivial;
      result = defense_defend_in_n(next,n,n_min);
      max_nr_nontrivial += nr_nontrivial;
      --max_nr_nontrivial;
    }
    else
      result = true;
  }
  else
    result = defense_defend_in_n(next,n,n_min);

  TraceFunctionExit(__func__);
  TraceFunctionResult("%u",result);
  TraceFunctionResultEnd();
  return result;
}

/* Determine whether there are refutations after an attempted key move
 * at non-root level
 * @param si slice index
 * @param n maximum number of half moves until end state has to be reached
 * @param max_nr_refutations how many refutations should we look for
 * @return <slack_length_battle - stalemate
           <=n solved  - return value is maximum number of moves
                         (incl. defense) needed
           n+2 refuted - <=max_nr_refutations refutations found
           n+4 refuted - >max_nr_refutations refutations found
 */
stip_length_type
max_nr_nontrivial_guard_can_defend_in_n(slice_index si,
                                        stip_length_type n,
                                        unsigned int max_nr_refutations)
{
  slice_index const next = slices[si].u.pipe.next;
  unsigned int result;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParam("%u",n);
  TraceFunctionParam("%u",max_nr_refutations);
  TraceFunctionParamListEnd();

  if (n>min_length_nontrivial)
  {
    unsigned int const nr_nontrivial = count_nontrivial_defenses(si);
    if (max_nr_nontrivial+1>=nr_nontrivial)
    {
      ++max_nr_nontrivial;
      max_nr_nontrivial -= nr_nontrivial;
      result = defense_can_defend_in_n(next,n,max_nr_refutations);
      max_nr_nontrivial += nr_nontrivial;
      --max_nr_nontrivial;
    }
    else
      result = n+4;
  }
  else
    result = defense_can_defend_in_n(next,n,max_nr_refutations);

  TraceFunctionExit(__func__);
  TraceFunctionResult("%u",result);
  TraceFunctionResultEnd();
  return result;
}

/* Determine whether there is a solution in n half moves.
 * @param si slice index of slice being solved
 * @param n maximum number of half moves until end state has to be reached
 * @param n_min minimal number of half moves to try
 * @return length of solution found, i.e.:
 *            <n_min defense put defender into self-check
 *            n_min..n length of shortest solution found
 *            >n no solution found
 *         (the second case includes the situation in self
 *         stipulations where the defense just played has reached the
 *         goal (in which case n_min<slack_length_battle and we return
 *         n_min)
 */
stip_length_type
max_nr_nontrivial_counter_has_solution_in_n(slice_index si,
                                            stip_length_type n,
                                            stip_length_type n_min)
{
  stip_length_type result = n+2;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParam("%u",n);
  TraceFunctionParam("%u",n_min);
  TraceFunctionParamListEnd();

  result = attack_has_solution_in_n(slices[si].u.pipe.next,n,n_min);

  if (result>=min_length_nontrivial)
    ++non_trivial_count[nbply];

  TraceFunctionExit(__func__);
  TraceFunctionResult("%u",result);
  TraceFunctionResultEnd();
  return result;
}

/* Solve a slice
 * @param si slice index
 * @param n maximum number of half moves until goal
 * @param n_min minimal number of half moves to try
 * @return number of half moves effectively used
 *         n+2 if no solution was found
 *         (n-slack_length_battle)%2 if the previous move led to a
 *            dead end (e.g. self-check)
 */
stip_length_type max_nr_nontrivial_counter_solve_in_n(slice_index si,
                                                      stip_length_type n,
                                                      stip_length_type n_min)
{
  stip_length_type result;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParam("%u",n);
  TraceFunctionParam("%u",n_min);
  TraceFunctionParamListEnd();

  result = attack_solve_in_n(slices[si].u.pipe.next,n,n_min);

  TraceFunctionExit(__func__);
  TraceFunctionResult("%u",result);
  TraceFunctionResultEnd();
  return result;
}

/* Determine and write the threats after the move that has just been
 * played.
 * @param threats table where to add threats
 * @param si slice index
 * @param n maximum number of half moves until goal
 * @param n_min minimal number of half moves to try
 * @return length of threats
 *         (n-slack_length_battle)%2 if the attacker has something
 *           stronger than threats (i.e. has delivered check)
 *         n+2 if there is no threat
 */
stip_length_type
max_nr_nontrivial_counter_solve_threats_in_n(table threats,
                                             slice_index si,
                                             stip_length_type n,
                                             stip_length_type n_min)
{
  stip_length_type result;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParam("%u",n);
  TraceFunctionParam("%u",n_min);
  TraceFunctionParamListEnd();

  result = attack_solve_threats_in_n(threats,slices[si].u.pipe.next,n,n_min);

  TraceFunctionExit(__func__);
  TraceFunctionResult("%u",result);
  TraceFunctionResultEnd();
  return result;
}

/* **************** Stipulation instrumentation ***************
 */

static void nontrivial_guard_inserter_attack_move(slice_index si,
                                                  stip_structure_traversal *st)
{
  stip_length_type const length = slices[si].u.branch.length;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParamListEnd();

  stip_traverse_structure_children(si,st);
  pipe_append(si,alloc_max_nr_nontrivial_guard(length));

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

static void append_nontrivial_counter(slice_index si,
                                      stip_structure_traversal *st)
{
  slice_index const next = slices[si].u.pipe.next;
  stip_length_type const length = slices[next].u.branch.length;
  stip_length_type const min_length = slices[next].u.branch.min_length;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParamListEnd();

  stip_traverse_structure_children(si,st);

  {
    slice_index const next = slices[si].u.pipe.next;
    slice_index const next_prev = slices[next].prev;
    if (next_prev==si)
      pipe_append(si,alloc_max_nr_nontrivial_counter(length,min_length));
    else
    {
      assert(slices[next_prev].type==STMaxNrNonTrivialCounter);
      pipe_set_successor(si,next_prev);
    }
  }

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

static stip_structure_visitor const max_nr_nontrivial_guards_inserters[] =
{
  &stip_traverse_structure_children,      /* STProxy */
  &stip_traverse_structure_children,      /* STAttackMove */
  &append_nontrivial_counter,             /* STDefenseMove */
  &stip_traverse_structure_children,      /* STHelpMove */
  &stip_traverse_structure_children,      /* STHelpFork */
  &stip_traverse_structure_children,      /* STSeriesMove */
  &stip_traverse_structure_children,      /* STSeriesFork */
  &stip_traverse_structure_children,      /* STLeafDirect */
  &stip_traverse_structure_children,      /* STLeafHelp */
  &stip_traverse_structure_children,      /* STLeafForced */
  &stip_traverse_structure_children,      /* STReciprocal */
  &stip_traverse_structure_children,      /* STQuodlibet */
  &stip_traverse_structure_children,      /* STNot */
  &stip_traverse_structure_children,      /* STMoveInverterRootSolvableFilter */
  &stip_traverse_structure_children,      /* STMoveInverterSolvableFilter */
  &stip_traverse_structure_children,      /* STMoveInverterSeriesFilter */
  &stip_traverse_structure_children,      /* STAttackRoot */
  &stip_traverse_structure_children,      /* STBattlePlaySolutionWriter */
  &stip_traverse_structure_children,      /* STPostKeyPlaySolutionWriter */
  &stip_traverse_structure_children,      /* STPostKeyPlaySuppressor */
  &stip_traverse_structure_children,      /* STContinuationWriter */
  &stip_traverse_structure_children,      /* STRefutationsWriter */
  &stip_traverse_structure_children,      /* STThreatWriter */
  &stip_traverse_structure_children,      /* STThreatEnforcer */
  &stip_traverse_structure_children,      /* STRefutationsCollector */
  &stip_traverse_structure_children,      /* STVariationWriter */
  &stip_traverse_structure_children,      /* STRefutingVariationWriter */
  &stip_traverse_structure_children,      /* STNoShortVariations */
  &stip_traverse_structure_children,      /* STAttackHashed */
  &stip_traverse_structure_children,      /* STHelpRoot */
  &stip_traverse_structure_children,      /* STHelpShortcut */
  &stip_traverse_structure_children,      /* STHelpHashed */
  &stip_traverse_structure_children,      /* STSeriesRoot */
  &stip_traverse_structure_children,      /* STSeriesShortcut */
  &stip_traverse_structure_children,      /* STParryFork */
  &stip_traverse_structure_children,      /* STSeriesHashed */
  &stip_traverse_structure_children,      /* STSelfCheckGuardRootSolvableFilter */
  &stip_traverse_structure_children,      /* STSelfCheckGuardSolvableFilter */
  &nontrivial_guard_inserter_attack_move, /* STSelfCheckGuardRootDefenderFilter */
  &stip_traverse_structure_children,      /* STSelfCheckGuardAttackerFilter */
  &nontrivial_guard_inserter_attack_move, /* STSelfCheckGuardDefenderFilter */
  &stip_traverse_structure_children,      /* STSelfCheckGuardHelpFilter */
  &stip_traverse_structure_children,      /* STSelfCheckGuardSeriesFilter */
  &stip_traverse_structure_children,      /* STDirectDefenseRootSolvableFilter */
  &stip_traverse_structure_children,      /* STDirectDefense */
  &stip_traverse_structure_children,      /* STReflexHelpFilter */
  &stip_traverse_structure_children,      /* STReflexSeriesFilter */
  &stip_traverse_structure_children,      /* STReflexRootSolvableFilter */
  &stip_traverse_structure_children,      /* STReflexAttackerFilter */
  &stip_traverse_structure_children,      /* STReflexDefenderFilter */
  &stip_traverse_structure_children,      /* STSelfDefense */
  &stip_traverse_structure_children,      /* STRestartGuardRootDefenderFilter */
  &stip_traverse_structure_children,      /* STRestartGuardHelpFilter */
  &stip_traverse_structure_children,      /* STRestartGuardSeriesFilter */
  &stip_traverse_structure_children,      /* STIntelligentHelpFilter */
  &stip_traverse_structure_children,      /* STIntelligentSeriesFilter */
  &stip_traverse_structure_children,      /* STGoalReachableGuardHelpFilter */
  &stip_traverse_structure_children,      /* STGoalReachableGuardSeriesFilter */
  &stip_traverse_structure_children,      /* STKeepMatingGuardRootDefenderFilter */
  &stip_traverse_structure_children,      /* STKeepMatingGuardAttackerFilter */
  &stip_traverse_structure_children,      /* STKeepMatingGuardDefenderFilter */
  &stip_traverse_structure_children,      /* STKeepMatingGuardHelpFilter */
  &stip_traverse_structure_children,      /* STKeepMatingGuardSeriesFilter */
  &stip_traverse_structure_children,      /* STMaxFlightsquares */
  &stip_traverse_structure_children,      /* STDegenerateTree */
  &stip_traverse_structure_children,      /* STMaxNrNonTrivial */
  &stip_traverse_structure_children,      /* STMaxNrNonTrivialCounter */
  &stip_traverse_structure_children,      /* STMaxThreatLength */
  &stip_traverse_structure_children,      /* STMaxTimeRootDefenderFilter */
  &stip_traverse_structure_children,      /* STMaxTimeDefenderFilter */
  &stip_traverse_structure_children,      /* STMaxTimeHelpFilter */
  &stip_traverse_structure_children,      /* STMaxTimeSeriesFilter */
  &stip_traverse_structure_children,      /* STMaxSolutionsRootSolvableFilter */
  &stip_traverse_structure_children,      /* STMaxSolutionsRootDefenderFilter */
  &stip_traverse_structure_children,      /* STMaxSolutionsHelpFilter */
  &stip_traverse_structure_children,      /* STMaxSolutionsSeriesFilter */
  &stip_traverse_structure_children,      /* STStopOnShortSolutionsRootSolvableFilter */
  &stip_traverse_structure_children,      /* STStopOnShortSolutionsHelpFilter */
  &stip_traverse_structure_children       /* STStopOnShortSolutionsSeriesFilter */
};

/* Instrument stipulation with STMaxNrNonTrivial slices
 */
void stip_insert_max_nr_nontrivial_guards(void)
{
  stip_structure_traversal st;

  TraceFunctionEntry(__func__);
  TraceFunctionParamListEnd();

  TraceStipulation(root_slice);

  stip_structure_traversal_init(&st,&max_nr_nontrivial_guards_inserters,0);
  stip_traverse_structure(root_slice,&st);

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}
