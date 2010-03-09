#include "stipulation/battle_play/try.h"
#include "pydata.h"
#include "pypipe.h"
#include "stipulation/branch.h"
#include "stipulation/battle_play/defense_play.h"
#include "stipulation/battle_play/attack_play.h"
#include "stipulation/battle_play/solution.h"
#include "pyoutput.h"
#include "trace.h"

#include <assert.h>

/* Table where refutations are collected
 */
static table refutations;

/* Allocate a STTryWriter defender slice.
 * @param length maximum number of half-moves of slice (+ slack)
 * @param min_length minimum number of half-moves of slice (+ slack)
 * @return index of allocated slice
 */
static slice_index alloc_try_writer_slice(stip_length_type length,
                                          stip_length_type min_length)
{
  slice_index result;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",length);
  TraceFunctionParam("%u",min_length);
  TraceFunctionParamListEnd();

  result = alloc_branch(STTryWriter,length,min_length);

  TraceFunctionExit(__func__);
  TraceFunctionResult("%u",result);
  TraceFunctionResultEnd();
  return result;
}

/* Try to defend after an attempted key move at root level
 * @param si slice index
 * @return true iff the defending side can successfully defend
 */
boolean try_writer_root_defend(slice_index si)
{
  unsigned int nr_refutations;
  stip_length_type const length = slices[si].u.branch.length;
  boolean result;
  slice_index const next = slices[si].u.pipe.next;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParamListEnd();

  refutations = allocate_table();

  nr_refutations = defense_can_defend_in_n(next,length,max_nr_refutations);
  if (nr_refutations==0)
  {
    result = false;
    write_attack(attack_key);
    solution_writer_solve_postkey(si);
    write_end_of_solution();
  }
  else
  {
    result = true;

    if (nr_refutations<=max_nr_refutations)
    {
      write_attack(attack_try);
      solution_writer_solve_postkey(si);
      write_refutations(refutations);
      write_end_of_solution();
    }
  }

  free_table();

  TraceFunctionExit(__func__);
  TraceValue("%u",result);
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
unsigned int try_writer_can_defend_in_n(slice_index si,
                                        stip_length_type n,
                                        unsigned int max_result)
{
  unsigned int result;
  slice_index const next = slices[si].u.pipe.next;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParamListEnd();

  result = defense_can_defend_in_n(next,n,max_result);

  TraceFunctionExit(__func__);
  TraceValue("%u",result);
  TraceFunctionResultEnd();
  return result;
}

/* Allocate a STRefutationsCollector slice.
 * @param length maximum number of half-moves of slice (+ slack)
 * @param min_length minimum number of half-moves of slice (+ slack)
 * @return index of allocated slice
 */
static
slice_index alloc_refutations_collector_slice(stip_length_type length,
                                              stip_length_type min_length)
{
  slice_index result;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",length);
  TraceFunctionParam("%u",min_length);
  TraceFunctionParamListEnd();

  result = alloc_branch(STRefutationsCollector,length,min_length);

  TraceFunctionExit(__func__);
  TraceFunctionResult("%u",result);
  TraceFunctionResultEnd();
  return result;
}

/* Determine whether the defense just played defends against the threats.
 * @param threats table containing the threats
 * @param len_threat length of threat(s) in table threats
 * @param si slice index
 * @param n maximum number of moves until goal (after the defense)
 * @return true iff the defense defends against at least one of the
 *         threats
 */
boolean
refutations_collector_are_threats_refuted_in_n(table threats,
                                               stip_length_type len_threat,
                                               slice_index si,
                                               stip_length_type n)
{
  slice_index const next = slices[si].u.pipe.next;
  boolean result = true;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",threats);
  TraceFunctionParam("%u",table_length(threats));
  TraceFunctionParam("%u",len_threat);
  TraceFunctionParam("%u",si);
  TraceFunctionParam("%u",n);
  TraceFunctionParamListEnd();

  if (is_current_move_in_table(refutations))
    result = false;
  else
    result = attack_are_threats_refuted_in_n(threats,len_threat,next,n);

  TraceFunctionExit(__func__);
  TraceFunctionResult("%u",result);
  TraceFunctionResultEnd();
  return result;
}

/* Determine whether attacker can end in n half moves.
 * @param si slice index
 * @param n maximum number of half moves until goal
 * @param n_min minimal number of half moves to try
 * @return length of solution found, i.e.:
 *            0 defense put defender into self-check
 *            n_min..n length of shortest solution found
 *            >n no solution found
 *         (the second case includes the situation in self
 *         stipulations where the defense just played has reached the
 *         goal (in which case n_min<slack_length_battle and we return
 *         n_min)
 */
stip_length_type
refutations_collector_has_solution_in_n(slice_index si,
                                        stip_length_type n,
                                        stip_length_type n_min)
{
  stip_length_type result;
  slice_index const next = slices[si].u.pipe.next;
  stip_length_type const length = slices[si].u.branch.length;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParam("%u",n);
  TraceFunctionParamListEnd();

  if (n==length && table_length(refutations)>max_nr_refutations)
    result = n+2;
  else
  {
    result = attack_has_solution_in_n(next,n,n_min);

    if (result>length)
    {
      assert(get_top_table()==refutations);
      append_to_top_table();
      coupfort();
    }
  }

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
refutations_collector_solve_threats_in_n(table threats,
                                         slice_index si,
                                         stip_length_type n,
                                         stip_length_type n_min)
{
  stip_length_type result;
  slice_index const next = slices[si].u.pipe.next;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",threats);
  TraceFunctionParam("%u",table_length(threats));
  TraceFunctionParam("%u",si);
  TraceFunctionParam("%u",n);
  TraceFunctionParam("%u",n_min);
  TraceFunctionParamListEnd();

  result = attack_solve_threats_in_n(threats,next,n,n_min);

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
stip_length_type refutations_collector_solve_in_n(slice_index si,
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

  if (is_current_move_in_table(refutations))
    result = n+2;
  else
    result = attack_solve_in_n(next,n,n_min);

  TraceFunctionExit(__func__);
  TraceFunctionResult("%u",result);
  TraceFunctionResultEnd();
  return result;
}

/* Append refutations collector
 * @param si identifies slice around which to insert try handlers
 * @param st address of structure defining traversal
 * @return true
 */
static boolean defense_root_wrap(slice_index si, slice_traversal *st)
{
  boolean const result = true;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParamListEnd();

  {
    stip_length_type const length = slices[si].u.branch.length;
    stip_length_type const min_length = slices[si].u.branch.min_length;
    slice_index const prev = slices[si].prev;
    slice_index const next = slices[si].u.pipe.next;
    slice_index const next_prev = slices[next].prev;
    slice_index const writer = alloc_try_writer_slice(length,min_length);
    slice_index const
        collector = alloc_refutations_collector_slice(length-1,min_length-1);
    pipe_link(prev,writer);
    pipe_link(writer,si);
    pipe_link(si,collector);

    if (slices[next_prev].u.pipe.next==next)
      pipe_set_successor(collector,next);
    else
      pipe_link(collector,next);
  }

  TraceFunctionExit(__func__);
  TraceFunctionResult("%u",result);
  TraceFunctionResultEnd();
  return result;
}

static slice_operation const try_handler_inserters[] =
{
  &slice_traverse_children, /* STProxy */
  &slice_traverse_children, /* STAttackMove */
  &slice_traverse_children, /* STDefenseMove */
  &slice_traverse_children, /* STHelpMove */
  &slice_traverse_children, /* STHelpFork */
  &slice_traverse_children, /* STSeriesMove */
  &slice_traverse_children, /* STSeriesFork */
  &slice_operation_noop,    /* STLeafDirect */
  &slice_operation_noop,    /* STLeafHelp */
  &slice_operation_noop,    /* STLeafForced */
  &slice_traverse_children, /* STReciprocal */
  &slice_traverse_children, /* STQuodlibet */
  &slice_traverse_children, /* STNot */
  &slice_traverse_children, /* STMoveInverterRootSolvableFilter */
  &slice_traverse_children, /* STMoveInverterSolvableFilter */
  &slice_traverse_children, /* STMoveInverterSeriesFilter */
  &slice_traverse_children, /* STAttackRoot */
  &slice_traverse_children, /* STBattlePlaySolutionWriter */
  &slice_traverse_children, /* STPostKeyPlaySolutionWriter */
  &slice_traverse_children, /* STContinuationWriter */
  &slice_traverse_children, /* STTryWriter */
  &slice_traverse_children, /* STThreatWriter */
  &defense_root_wrap,       /* STDefenseRoot */
  &slice_traverse_children, /* STThreatEnforcer */
  &slice_traverse_children, /* STRefutationsCollector */
  &slice_traverse_children, /* STVariationWriter */
  &slice_traverse_children, /* STRefutingVariationWriter */
  &slice_traverse_children, /* STNoShortVariations */
  &slice_traverse_children, /* STAttackHashed */
  &slice_traverse_children, /* STHelpRoot */
  &slice_traverse_children, /* STHelpShortcut */
  &slice_traverse_children, /* STHelpHashed */
  &slice_traverse_children, /* STSeriesRoot */
  &slice_traverse_children, /* STSeriesShortcut */
  &slice_traverse_children, /* STParryFork */
  &slice_traverse_children, /* STSeriesHashed */
  &slice_traverse_children, /* STSelfCheckGuardRootSolvableFilter */
  &slice_traverse_children, /* STSelfCheckGuardSolvableFilter */
  &slice_traverse_children, /* STSelfCheckGuardRootDefenderFilter */
  &slice_traverse_children, /* STSelfCheckGuardAttackerFilter */
  &slice_traverse_children, /* STSelfCheckGuardDefenderFilter */
  &slice_traverse_children, /* STSelfCheckGuardHelpFilter */
  &slice_traverse_children, /* STSelfCheckGuardSeriesFilter */
  &slice_traverse_children, /* STDirectDefenseRootSolvableFilter */
  &slice_traverse_children, /* STDirectDefense */
  &slice_traverse_children, /* STReflexHelpFilter */
  &slice_traverse_children, /* STReflexSeriesFilter */
  &slice_traverse_children, /* STReflexRootSolvableFilter */
  &slice_traverse_children, /* STReflexAttackerFilter */
  &slice_traverse_children, /* STReflexDefenderFilter */
  &slice_traverse_children, /* STSelfAttack */
  &slice_traverse_children, /* STSelfDefense */
  &slice_traverse_children, /* STRestartGuardRootDefenderFilter */
  &slice_traverse_children, /* STRestartGuardHelpFilter */
  &slice_traverse_children, /* STRestartGuardSeriesFilter */
  &slice_traverse_children, /* STIntelligentHelpFilter */
  &slice_traverse_children, /* STIntelligentSeriesFilter */
  &slice_traverse_children, /* STGoalReachableGuardHelpFilter */
  &slice_traverse_children, /* STGoalReachableGuardSeriesFilter */
  &slice_traverse_children, /* STKeepMatingGuardRootDefenderFilter */
  &slice_traverse_children, /* STKeepMatingGuardAttackerFilter */
  &slice_traverse_children, /* STKeepMatingGuardDefenderFilter */
  &slice_traverse_children, /* STKeepMatingGuardHelpFilter */
  &slice_traverse_children, /* STKeepMatingGuardSeriesFilter */
  &slice_traverse_children, /* STMaxFlightsquares */
  &slice_traverse_children, /* STDegenerateTree */
  &slice_traverse_children, /* STMaxNrNonTrivial */
  &slice_traverse_children, /* STMaxThreatLength */
  &slice_traverse_children, /* STMaxTimeRootDefenderFilter */
  &slice_traverse_children, /* STMaxTimeDefenderFilter */
  &slice_traverse_children, /* STMaxTimeHelpFilter */
  &slice_traverse_children, /* STMaxTimeSeriesFilter */
  &slice_traverse_children, /* STMaxSolutionsRootSolvableFilter */
  &slice_traverse_children, /* STMaxSolutionsRootDefenderFilter */
  &slice_traverse_children, /* STMaxSolutionsHelpFilter */
  &slice_traverse_children, /* STMaxSolutionsSeriesFilter */
  &slice_traverse_children, /* STStopOnShortSolutionsRootSolvableFilter */
  &slice_traverse_children, /* STStopOnShortSolutionsHelpFilter */
  &slice_traverse_children  /* STStopOnShortSolutionsSeriesFilter */
};

/* Instrument the stipulation representation so that it can deal with
 * tries
 */
void stip_insert_try_handlers(void)
{
  slice_traversal st;

  TraceFunctionEntry(__func__);
  TraceFunctionParamListEnd();

  slice_traversal_init(&st,&try_handler_inserters,0);
  traverse_slices(root_slice,&st);

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}
