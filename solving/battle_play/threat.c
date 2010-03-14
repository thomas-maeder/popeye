#include "stipulation/battle_play/threat.h"
#include "pydata.h"
#include "pypipe.h"
#include "stipulation/branch.h"
#include "stipulation/battle_play/defense_play.h"
#include "stipulation/battle_play/attack_play.h"
#include "pyoutput.h"
#include "pymsg.h"
#include "trace.h"

#include <assert.h>

/* Table where threats of the various move levels are collected
 */
static table threats[maxply+1];

/* Lengths of threats of the various move levels 
 */
static stip_length_type threat_lengths[maxply+1];

/* Allocate a STThreatEnforcer slice.
 * @param length maximum number of half-moves of slice (+ slack)
 * @param min_length minimum number of half-moves of slice (+ slack)
 * @return index of allocated slice
 */
static slice_index alloc_threat_enforcer_slice(stip_length_type length,
                                               stip_length_type min_length)
{
  slice_index result;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",length);
  TraceFunctionParam("%u",min_length);
  TraceFunctionParamListEnd();

  result = alloc_branch(STThreatEnforcer,length,min_length);

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
stip_length_type threat_enforcer_has_solution_in_n(slice_index si,
                                                   stip_length_type n,
                                                   stip_length_type n_min)
{
  stip_length_type result;
  slice_index const next = slices[si].u.pipe.next;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParam("%u",n);
  TraceFunctionParamListEnd();

  result = attack_has_solution_in_n(next,n,n_min);

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
stip_length_type threat_enforcer_solve_threats_in_n(table threats,
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

/* Determine whether there is a short continuation after the defense
 * just played 
 * @param si identifies slice that just played the defense
 * @param n maximum number of half moves until end of branch
 */
static boolean has_short_continuation(slice_index si, stip_length_type n)
{
  boolean result;
  stip_length_type n_min;
  slice_index const next = slices[si].u.pipe.next;
  stip_length_type const parity = n%2;
  stip_length_type const length = slices[si].u.branch.length;
  stip_length_type const min_length = slices[si].u.branch.min_length;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParam("%u",n);
  TraceFunctionParamListEnd();

  if (n<slack_length_battle+2)
    /* remaining play in this slice is too short to allow short
     * continuations
     */
    result = false;
  else
  {
    n -= 2;

    if (n+min_length>slack_length_battle+length)
      n_min = n-(length-min_length);
    else
      n_min = slack_length_battle-parity;

    result = attack_has_solution_in_n(next,n,n_min)<=n;
  }

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
stip_length_type threat_enforcer_solve_in_n(slice_index si,
                                            stip_length_type n,
                                            stip_length_type n_min)
{
  stip_length_type result;
  slice_index const next = slices[si].u.pipe.next;
  stip_length_type const len_threat = threat_lengths[nbply];

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParam("%u",n);
  TraceFunctionParam("%u",n_min);
  TraceFunctionParamListEnd();

  if (len_threat<slack_length_battle)
    /* that attack has something stronger than threats (typically, it
     * delivers check)
     */
    result = attack_solve_in_n(next,n,n_min);
  else if (len_threat<=n)
  {
    /* there is a threat - don't report variations shorter than it */
    if (!has_short_continuation(si,len_threat)
        && attack_are_threats_refuted_in_n(threats[nbply],len_threat,next,n))
      result = attack_solve_in_n(next,n,n_min);
    else
      result = (n-slack_length_battle)%2;
  }
  else
    /* zugzwang */
    result = attack_solve_in_n(next,n,n_min);

  TraceFunctionExit(__func__);
  TraceFunctionResult("%u",result);
  TraceFunctionResultEnd();
  return result;
}

/* Allocate a STThreatWriter defender slice.
 * @param length maximum number of half-moves of slice (+ slack)
 * @param min_length minimum number of half-moves of slice (+ slack)
 * @param enforcer identifies the slice enforcing the found threats
 * @return index of allocated slice
 */
static slice_index alloc_threat_writer_slice(stip_length_type length,
                                             stip_length_type min_length,
                                             slice_index enforcer)
{
  slice_index result;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",length);
  TraceFunctionParam("%u",min_length);
  TraceFunctionParamListEnd();

  result = alloc_branch(STThreatWriter,length,min_length);
  slices[result].u.threat_writer.enforcer = enforcer;

  TraceFunctionExit(__func__);
  TraceFunctionResult("%u",result);
  TraceFunctionResultEnd();
  return result;
}

/* Solve threats after an attacker's move
 * @param threats table where to add threats
 * @param si slice index
 * @return length of threats
 *         <slack_length_battle if the attacker has something stronger
 *             than threats (i.e. has delivered check)
 *         n+2 if there is no threat
 */
static stip_length_type solve_threats(table threats,
                                      slice_index si,
                                      stip_length_type n)
{
  slice_index const enforcer = slices[si].u.threat_writer.enforcer;
  stip_length_type const length = slices[si].u.threat_writer.length;
  stip_length_type const min_length = slices[si].u.threat_writer.min_length;
  stip_length_type const parity = (n-slack_length_battle)%2;
  stip_length_type n_min = slack_length_battle-parity;
  stip_length_type result;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParam("%u",n);
  TraceFunctionParamListEnd();

  if (n+min_length>n_min+length)
    n_min = n-(length-min_length);

  output_start_threat_level();
  result = attack_solve_threats_in_n(threats,enforcer,n,n_min);
  output_end_threat_level();

  if (result==n+2)
    Message(Zugzwang);

  TraceFunctionExit(__func__);
  TraceFunctionResult("%u",result);
  TraceFunctionResultEnd();
  return result;
}

/* Try to defend after an attempted key move at root level
 * @param si slice index
 * @return true iff the defending side can successfully defend
 */
boolean threat_writer_root_defend(slice_index si)
{
  boolean result;
  slice_index const next = slices[si].u.pipe.next;
  stip_length_type const length = slices[si].u.threat_writer.length;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParamListEnd();

  threats[nbply+1] = allocate_table();
  threat_lengths[nbply+1] = solve_threats(threats[nbply+1],si,length-1);
  result = defense_root_defend(next);
  free_table();

  TraceFunctionExit(__func__);
  TraceValue("%u",result);
  TraceFunctionResultEnd();
  return result;
}

/* Try to defend after an attempted key move at non-root level
 * When invoked with some n, the function assumes that the key doesn't
 * solve in less than n half moves.
 * @param si slice index
 * @param n maximum number of half moves until end state has to be reached
 * @return true iff the defender can defend
 */
boolean threat_writer_defend_in_n(slice_index si, stip_length_type n)
{
  boolean result;
  slice_index const next = slices[si].u.pipe.next;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParamListEnd();

  threats[nbply+1] = allocate_table();
  threat_lengths[nbply+1] = solve_threats(threats[nbply+1],si,n-1);
  result = defense_defend_in_n(next,n);
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
unsigned int threat_writer_can_defend_in_n(slice_index si,
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

/* Find the first postkey slice and deallocate unused slices on the
 * way to it
 * @param si slice index
 * @param st address of structure capturing traversal state
 */
void threat_writer_reduce_to_postkey_play(slice_index si, slice_traversal *st)
{
  slice_index *postkey_slice = st->param;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParamListEnd();

  *postkey_slice = si;

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

/* Wrap a defense move slice with threat handler slices
 * @param si identifies slice around which to insert threat handlers
 * @param st address of structure defining traversal
 */
static void defense_move_wrap(slice_index si, slice_traversal *st)
{
  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParamListEnd();

  slice_traverse_children(si,st);

  {
    stip_length_type const length = slices[si].u.branch.length;
    stip_length_type const min_length = slices[si].u.branch.min_length;
    slice_index const prev = slices[si].prev;
    slice_index const enforcer = alloc_threat_enforcer_slice(length-1,
                                                             min_length-1);
    pipe_append(prev,alloc_threat_writer_slice(length,min_length,enforcer));
    pipe_append(si,enforcer);
  }

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

/* Avoid the parrying move when inserting threat handlers
 * @param si identifies slice around which to insert threat handlers
 * @param st address of structure defining traversal
 */
static void threat_writer_avoid_parrying(slice_index si, slice_traversal *st)
{
  slice_index const next = slices[si].u.pipe.next;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParamListEnd();

  traverse_slices(next,st);

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

static slice_operation const threat_handler_inserters[] =
{
  &slice_traverse_children,      /* STProxy */
  &slice_traverse_children,      /* STAttackMove */
  &defense_move_wrap,            /* STDefenseMove */
  &slice_traverse_children,      /* STHelpMove */
  &slice_traverse_children,      /* STHelpFork */
  &slice_traverse_children,      /* STSeriesMove */
  &slice_traverse_children,      /* STSeriesFork */
  &slice_operation_noop,         /* STLeafDirect */
  &slice_operation_noop,         /* STLeafHelp */
  &slice_operation_noop,         /* STLeafForced */
  &slice_traverse_children,      /* STReciprocal */
  &slice_traverse_children,      /* STQuodlibet */
  &slice_traverse_children,      /* STNot */
  &slice_traverse_children,      /* STMoveInverterRootSolvableFilter */
  &slice_traverse_children,      /* STMoveInverterSolvableFilter */
  &slice_traverse_children,      /* STMoveInverterSeriesFilter */
  &slice_traverse_children,      /* STAttackRoot */
  &slice_traverse_children,      /* STBattlePlaySolutionWriter */
  &slice_traverse_children,      /* STPostKeyPlaySolutionWriter */
  &slice_traverse_children,      /* STContinuationWriter */
  &slice_traverse_children,      /* STTryWriter */
  &slice_traverse_children,      /* STThreatWriter */
  &slice_traverse_children,      /* STThreatEnforcer */
  &slice_traverse_children,      /* STRefutationsCollector */
  &slice_traverse_children,      /* STVariationWriter */
  &slice_traverse_children,      /* STRefutingVariationWriter */
  &slice_traverse_children,      /* STNoShortVariations */
  &slice_traverse_children,      /* STAttackHashed */
  &slice_traverse_children,      /* STHelpRoot */
  &slice_traverse_children,      /* STHelpShortcut */
  &slice_traverse_children,      /* STHelpHashed */
  &slice_traverse_children,      /* STSeriesRoot */
  &slice_traverse_children,      /* STSeriesShortcut */
  &threat_writer_avoid_parrying, /* STParryFork */
  &slice_traverse_children,      /* STSeriesHashed */
  &slice_traverse_children,      /* STSelfCheckGuardRootSolvableFilter */
  &slice_traverse_children,      /* STSelfCheckGuardSolvableFilter */
  &slice_traverse_children,      /* STSelfCheckGuardRootDefenderFilter */
  &slice_traverse_children,      /* STSelfCheckGuardAttackerFilter */
  &slice_traverse_children,      /* STSelfCheckGuardDefenderFilter */
  &slice_traverse_children,      /* STSelfCheckGuardHelpFilter */
  &slice_traverse_children,      /* STSelfCheckGuardSeriesFilter */
  &slice_traverse_children,      /* STDirectDefenseRootSolvableFilter */
  &slice_traverse_children,      /* STDirectDefense */
  &slice_traverse_children,      /* STReflexHelpFilter */
  &slice_traverse_children,      /* STReflexSeriesFilter */
  &slice_traverse_children,      /* STReflexRootSolvableFilter */
  &slice_traverse_children,      /* STReflexAttackerFilter */
  &slice_traverse_children,      /* STReflexDefenderFilter */
  &slice_traverse_children,      /* STSelfAttack */
  &slice_traverse_children,      /* STSelfDefense */
  &slice_traverse_children,      /* STRestartGuardRootDefenderFilter */
  &slice_traverse_children,      /* STRestartGuardHelpFilter */
  &slice_traverse_children,      /* STRestartGuardSeriesFilter */
  &slice_traverse_children,      /* STIntelligentHelpFilter */
  &slice_traverse_children,      /* STIntelligentSeriesFilter */
  &slice_traverse_children,      /* STGoalReachableGuardHelpFilter */
  &slice_traverse_children,      /* STGoalReachableGuardSeriesFilter */
  &slice_traverse_children,      /* STKeepMatingGuardRootDefenderFilter */
  &slice_traverse_children,      /* STKeepMatingGuardAttackerFilter */
  &slice_traverse_children,      /* STKeepMatingGuardDefenderFilter */
  &slice_traverse_children,      /* STKeepMatingGuardHelpFilter */
  &slice_traverse_children,      /* STKeepMatingGuardSeriesFilter */
  &slice_traverse_children,      /* STMaxFlightsquares */
  &slice_traverse_children,      /* STDegenerateTree */
  &slice_traverse_children,      /* STMaxNrNonTrivial */
  &slice_traverse_children,      /* STMaxThreatLength */
  &slice_traverse_children,      /* STMaxTimeRootDefenderFilter */
  &slice_traverse_children,      /* STMaxTimeDefenderFilter */
  &slice_traverse_children,      /* STMaxTimeHelpFilter */
  &slice_traverse_children,      /* STMaxTimeSeriesFilter */
  &slice_traverse_children,      /* STMaxSolutionsRootSolvableFilter */
  &slice_traverse_children,      /* STMaxSolutionsRootDefenderFilter */
  &slice_traverse_children,      /* STMaxSolutionsHelpFilter */
  &slice_traverse_children,      /* STMaxSolutionsSeriesFilter */
  &slice_traverse_children,      /* STStopOnShortSolutionsRootSolvableFilter */
  &slice_traverse_children,      /* STStopOnShortSolutionsHelpFilter */
  &slice_traverse_children       /* STStopOnShortSolutionsSeriesFilter */
};

/* Instrument the stipulation representation so that it can deal with
 * threats
 */
void stip_insert_threat_handlers(void)
{
  slice_traversal st;

  TraceFunctionEntry(__func__);
  TraceFunctionParamListEnd();

  slice_traversal_init(&st,&threat_handler_inserters,0);
  traverse_slices(root_slice,&st);

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}
