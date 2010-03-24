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

/* value of threat_lengths[N] that signals that no threats were found
 * in ply N (and that threats[N] may not even refer to a table)
 */
static stip_length_type const no_threats_found = UINT_MAX;

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
    /* zugzwang, or we haven't even looked for threats */
    result = attack_solve_in_n(next,n,n_min);

  TraceFunctionExit(__func__);
  TraceFunctionResult("%u",result);
  TraceFunctionResultEnd();
  return result;
}

/* Allocate a STThreatWriter defender slice.
 * @param length maximum number of half-moves of slice (+ slack)
 * @param min_length minimum number of half-moves of slice (+ slack)
 * @param attack_side identifies the slice where attack play starts
 *                    after this slice
 * @return index of allocated slice
 */
static slice_index alloc_threat_writer_slice(stip_length_type length,
                                             stip_length_type min_length,
                                             slice_index attack_side)
{
  slice_index result;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",length);
  TraceFunctionParam("%u",min_length);
  TraceFunctionParamListEnd();

  result = alloc_branch(STThreatWriter,length,min_length);
  slices[result].u.threat_writer.attack_side = attack_side;

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
  slice_index const attack_side = slices[si].u.threat_writer.attack_side;
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
  result = attack_solve_threats_in_n(threats,attack_side,n,n_min);
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

  assert(get_top_table()==threats[nbply+1]);
  threat_lengths[nbply+1] = no_threats_found;
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

  assert(get_top_table()==threats[nbply+1]);
  threat_lengths[nbply+1] = no_threats_found;
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
void threat_writer_reduce_to_postkey_play(slice_index si,
                                          stip_structure_traversal *st)
{
  slice_index *postkey_slice = st->param;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParamListEnd();

  *postkey_slice = si;

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

/* Prepend a threat writer slice to a defense move slice
 * @param si identifies slice around which to insert threat handlers
 * @param st address of structure defining traversal
 */
static void prepend_threat_writer(slice_index si, stip_structure_traversal *st)
{
  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParamListEnd();

  stip_traverse_structure_children(si,st);

  {
    slice_index const prev = slices[si].prev;
    slice_index const attack_side = slices[si].u.pipe.next;
    stip_length_type const length = slices[si].u.branch.length;
    stip_length_type const min_length = slices[si].u.branch.min_length;
    pipe_append(prev,alloc_threat_writer_slice(length,min_length,attack_side));
  }

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

/* Prepend a threat enforcer slice to a variation writer slice
 * @param si identifies slice around which to insert threat handlers
 * @param st address of structure defining traversal
 */
static void prepend_threat_enforcer(slice_index si, stip_structure_traversal *st)
{
  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParamListEnd();

  stip_traverse_structure_children(si,st);

  {
    slice_index const prev = slices[si].prev;
    stip_length_type const length = slices[si].u.branch.length;
    stip_length_type const min_length = slices[si].u.branch.min_length;
    pipe_append(prev,alloc_threat_enforcer_slice(length,min_length));
  }

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

/* Avoid the parrying move when inserting threat handlers
 * @param si identifies slice around which to insert threat handlers
 * @param st address of structure defining traversal
 */
static void threat_writer_avoid_parrying(slice_index si,
                                         stip_structure_traversal *st)
{
  slice_index const next = slices[si].u.pipe.next;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParamListEnd();

  stip_traverse_structure(next,st);

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

static stip_structure_visitor const threat_handler_inserters[] =
{
  &stip_traverse_structure_children, /* STProxy */
  &stip_traverse_structure_children, /* STAttackMove */
  &prepend_threat_writer,            /* STDefenseMove */
  &stip_traverse_structure_children, /* STHelpMove */
  &stip_traverse_structure_children, /* STHelpFork */
  &stip_traverse_structure_children, /* STSeriesMove */
  &stip_traverse_structure_children, /* STSeriesFork */
  &stip_structure_visitor_noop,      /* STLeafDirect */
  &stip_structure_visitor_noop,      /* STLeafHelp */
  &stip_structure_visitor_noop,      /* STLeafForced */
  &stip_traverse_structure_children, /* STReciprocal */
  &stip_traverse_structure_children, /* STQuodlibet */
  &stip_traverse_structure_children, /* STNot */
  &stip_traverse_structure_children, /* STMoveInverterRootSolvableFilter */
  &stip_traverse_structure_children, /* STMoveInverterSolvableFilter */
  &stip_traverse_structure_children, /* STMoveInverterSeriesFilter */
  &stip_traverse_structure_children, /* STAttackRoot */
  &stip_traverse_structure_children, /* STBattlePlaySolutionWriter */
  &stip_traverse_structure_children, /* STPostKeyPlaySolutionWriter */
  &stip_traverse_structure_children, /* STContinuationWriter */
  &stip_traverse_structure_children, /* STTryWriter */
  &stip_traverse_structure_children, /* STThreatWriter */
  &stip_traverse_structure_children, /* STThreatEnforcer */
  &stip_traverse_structure_children, /* STRefutationsCollector */
  &prepend_threat_enforcer,          /* STVariationWriter */
  &stip_traverse_structure_children, /* STRefutingVariationWriter */
  &stip_traverse_structure_children, /* STNoShortVariations */
  &stip_traverse_structure_children, /* STAttackHashed */
  &stip_traverse_structure_children, /* STHelpRoot */
  &stip_traverse_structure_children, /* STHelpShortcut */
  &stip_traverse_structure_children, /* STHelpHashed */
  &stip_traverse_structure_children, /* STSeriesRoot */
  &stip_traverse_structure_children, /* STSeriesShortcut */
  &threat_writer_avoid_parrying,     /* STParryFork */
  &stip_traverse_structure_children, /* STSeriesHashed */
  &stip_traverse_structure_children, /* STSelfCheckGuardRootSolvableFilter */
  &stip_traverse_structure_children, /* STSelfCheckGuardSolvableFilter */
  &stip_traverse_structure_children, /* STSelfCheckGuardRootDefenderFilter */
  &stip_traverse_structure_children, /* STSelfCheckGuardAttackerFilter */
  &stip_traverse_structure_children, /* STSelfCheckGuardDefenderFilter */
  &stip_traverse_structure_children, /* STSelfCheckGuardHelpFilter */
  &stip_traverse_structure_children, /* STSelfCheckGuardSeriesFilter */
  &stip_traverse_structure_children, /* STDirectDefenseRootSolvableFilter */
  &stip_traverse_structure_children, /* STDirectDefense */
  &stip_traverse_structure_children, /* STReflexHelpFilter */
  &stip_traverse_structure_children, /* STReflexSeriesFilter */
  &stip_traverse_structure_children, /* STReflexRootSolvableFilter */
  &stip_traverse_structure_children, /* STReflexAttackerFilter */
  &stip_traverse_structure_children, /* STReflexDefenderFilter */
  &stip_traverse_structure_children, /* STSelfAttack */
  &stip_traverse_structure_children, /* STSelfDefense */
  &stip_traverse_structure_children, /* STRestartGuardRootDefenderFilter */
  &stip_traverse_structure_children, /* STRestartGuardHelpFilter */
  &stip_traverse_structure_children, /* STRestartGuardSeriesFilter */
  &stip_traverse_structure_children, /* STIntelligentHelpFilter */
  &stip_traverse_structure_children, /* STIntelligentSeriesFilter */
  &stip_traverse_structure_children, /* STGoalReachableGuardHelpFilter */
  &stip_traverse_structure_children, /* STGoalReachableGuardSeriesFilter */
  &stip_traverse_structure_children, /* STKeepMatingGuardRootDefenderFilter */
  &stip_traverse_structure_children, /* STKeepMatingGuardAttackerFilter */
  &stip_traverse_structure_children, /* STKeepMatingGuardDefenderFilter */
  &stip_traverse_structure_children, /* STKeepMatingGuardHelpFilter */
  &stip_traverse_structure_children, /* STKeepMatingGuardSeriesFilter */
  &stip_traverse_structure_children, /* STMaxFlightsquares */
  &stip_traverse_structure_children, /* STDegenerateTree */
  &stip_traverse_structure_children, /* STMaxNrNonTrivial */
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
  &stip_traverse_structure_children  /* STStopOnShortSolutionsSeriesFilter */
};

/* Instrument the stipulation representation so that it can deal with
 * threats
 */
void stip_insert_threat_handlers(void)
{
  stip_structure_traversal st;
  unsigned int i;

  TraceFunctionEntry(__func__);
  TraceFunctionParamListEnd();

  TraceStipulation(root_slice);

  stip_structure_traversal_init(&st,&threat_handler_inserters,0);
  stip_traverse_structure(root_slice,&st);

  for (i = 0; i<=maxply; ++i)
    threat_lengths[i] = no_threats_found;

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}
