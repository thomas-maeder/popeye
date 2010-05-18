#include "stipulation/battle_play/threat.h"
#include "pydata.h"
#include "pypipe.h"
#include "stipulation/branch.h"
#include "stipulation/proxy.h"
#include "stipulation/battle_play/branch.h"
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

/* Allocate a STThreatCollector slice.
 * @param length maximum number of half-moves of slice (+ slack)
 * @param min_length minimum number of half-moves of slice (+ slack)
 * @return index of allocated slice
 */
static slice_index alloc_threat_collector_slice(stip_length_type length,
                                                stip_length_type min_length)
{
  slice_index result;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",length);
  TraceFunctionParam("%u",min_length);
  TraceFunctionParamListEnd();

  result = alloc_branch(STThreatCollector,length,min_length);

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
 *            n_min-2 defense has turned out to be illegal
 *            n_min..n length of shortest solution found
 *            n+2 no solution found
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

/* Determine whether there is a short continuation after the defense
 * just played 
 * @param si identifies slice that just played the defense
 * @param n maximum number of half moves until end of branch
 * @param n_min minimal number of half moves to try
 */
static boolean has_short_continuation(slice_index si,
                                      stip_length_type n,
                                      stip_length_type n_min)
{
  boolean result;
  slice_index const next = slices[si].u.pipe.next;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParam("%u",n);
  TraceFunctionParamListEnd();

  if (n<slack_length_battle+3)
    /* remaining play in this slice is too short to allow short
     * continuations
     */
    result = false;
  else
    result = attack_has_solution_in_n(next,n-2,n_min)<=n-2;

  TraceFunctionExit(__func__);
  TraceFunctionResult("%u",result);
  TraceFunctionResultEnd();
  return result;
}

/* Solve a slice
 * @param si slice index
 * @param n maximum number of half moves until goal
 * @param n_min minimal number of half moves to try
 * @return length of solution found and written, i.e.:
 *            n_min-2 defense has turned out to be illegal
 *            n_min..n length of shortest solution found
 *            n+2 no solution found
 */
stip_length_type threat_enforcer_solve_in_n(slice_index si,
                                            stip_length_type n,
                                            stip_length_type n_min)
{
  stip_length_type result;
  slice_index const next = slices[si].u.pipe.next;
  ply const threats_ply = nbply+1;
  stip_length_type const len_threat = threat_lengths[threats_ply];

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParam("%u",n);
  TraceFunctionParam("%u",n_min);
  TraceFunctionParamListEnd();

  TraceValue("%u\n",len_threat);

  if (len_threat<=slack_length_battle)
    /* that attack has something stronger than threats (typically, it
     * delivers check)
     */
    result = attack_solve_in_n(next,n,n_min);
  else if (len_threat<=n)
  {
    /* there is a threat - don't report variations shorter than it */
    table const threats_table = threats[threats_ply];
    TraceValue("%u\n",threats_ply);
    if (has_short_continuation(si,len_threat,n_min))
      result = n_min-2;
    else if (attack_are_threats_refuted_in_n(threats_table,len_threat,next,n))
    {
      /* prevent threat_collector_defend_in_n() from adding the
       * reported continuations to the threats table
       */
      threats[threats_ply] = table_nil;
      result = attack_solve_in_n(next,n,n_min);
      threats[threats_ply] = threats_table;
    }
    else
      result = n_min-2;
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

/* Try to defend after an attempted key move at non-root level
 * When invoked with some n, the function assumes that the key doesn't
 * solve in less than n half moves.
 * @param si slice index
 * @param n maximum number of half moves until end state has to be reached
 * @param n_min minimum number of half-moves of interesting variations
 *              (slack_length_battle <= n_min <= slices[si].u.branch.length)
 * @return true iff the defender can defend
 */
stip_length_type threat_collector_defend_in_n(slice_index si,
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

  result = defense_defend_in_n(next,n,n_min);

  TraceValue("%u\n",nbply);
  if (threats[nbply]!=table_nil && result<=n)
    append_to_top_table();

  TraceFunctionExit(__func__);
  TraceValue("%u",result);
  TraceFunctionResultEnd();
  return result;
}

/* Determine whether there are refutations after an attempted key move
 * at non-root level
 * @param si slice index
 * @param n maximum number of half moves until end state has to be reached
 * @param n_min minimum number of half-moves of interesting variations
 *              (slack_length_battle <= n_min <= slices[si].u.branch.length)
 * @param max_nr_refutations how many refutations should we look for
 * @return <=n solved  - return value is maximum number of moves
                         (incl. defense) needed
           n+2 refuted - <=max_nr_refutations refutations found
           n+4 refuted - >max_nr_refutations refutations found
 */
stip_length_type
threat_collector_can_defend_in_n(slice_index si,
                                 stip_length_type n,
                                 stip_length_type n_min,
                                 unsigned int max_nr_refutations)
{
  stip_length_type result;
  slice_index const next = slices[si].u.pipe.next;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParam("%u",n);
  TraceFunctionParam("%u",max_nr_refutations);
  TraceFunctionParamListEnd();

  result = defense_can_defend_in_n(next,n,n_min,max_nr_refutations);

  TraceFunctionExit(__func__);
  TraceValue("%u",result);
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
  stip_length_type const n_min = battle_branch_calc_n_min(si,n);
  stip_length_type result;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParam("%u",n);
  TraceFunctionParamListEnd();

  nextply(nbply);
  active_slice[nbply] = si;

  output_start_threat_level();
  result = attack_solve_in_n(attack_side,n,n_min);

  {
    /* We don't signal "Zugzwang" after the last attacking move of a
     * self play variation
     */
    boolean const write_zugzwang = n>slack_length_battle && result==n+2;
    output_end_threat_level(si,write_zugzwang);
  }

  finply();

  TraceFunctionExit(__func__);
  TraceFunctionResult("%u",result);
  TraceFunctionResultEnd();
  return result;
}

/* Try to defend after an attempted key move at root level
 * @param si slice index
 * @param n maximum number of half moves until end state has to be reached
 * @param n_min minimum number of half-moves of interesting variations
 *              (slack_length_battle <= n_min <= slices[si].u.branch.length)
 * @param max_nr_refutations how many refutations should we look for
 * @return <=n solved  - return value is maximum number of moves
 *                       (incl. defense) needed
 *         n+2 refuted - <=max_nr_refutations refutations found
 *         n+4 refuted - >max_nr_refutations refutations found
 */
stip_length_type threat_writer_root_defend(slice_index si,
                                           stip_length_type n,
                                           stip_length_type n_min,
                                           unsigned int max_nr_refutations)
{
  stip_length_type result;
  slice_index const next = slices[si].u.pipe.next;
  ply const threats_ply = nbply+2;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParam("%u",n);
  TraceFunctionParam("%u",n_min);
  TraceFunctionParam("%u",max_nr_refutations);
  TraceFunctionParamListEnd();

  TraceValue("%u\n",threats_ply);
  threats[threats_ply] = allocate_table();
  threat_lengths[threats_ply] = solve_threats(threats[threats_ply],si,n-1);

  result = defense_root_defend(next,n,n_min,max_nr_refutations);

  assert(get_top_table()==threats[threats_ply]);
  threat_lengths[threats_ply] = no_threats_found;
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
 * @param n_min minimum number of half-moves of interesting variations
 *              (slack_length_battle <= n_min <= slices[si].u.branch.length)
 * @return true iff the defender can defend
 */
stip_length_type threat_writer_defend_in_n(slice_index si,
                                           stip_length_type n,
                                           stip_length_type n_min)
{
  stip_length_type result;
  slice_index const next = slices[si].u.pipe.next;
  ply const threats_ply = nbply+2;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParam("%u",n);
  TraceFunctionParam("%u",n_min);
  TraceFunctionParamListEnd();

  TraceValue("%u\n",threats_ply);
  threats[threats_ply] = allocate_table();
  threat_lengths[threats_ply] = solve_threats(threats[nbply+1],si,n-1);

  result = defense_defend_in_n(next,n,n_min);

  assert(get_top_table()==threats[threats_ply]);
  free_table();
  threat_lengths[threats_ply] = no_threats_found;
  threats[threats_ply] = table_nil;

  TraceFunctionExit(__func__);
  TraceValue("%u",result);
  TraceFunctionResultEnd();
  return result;
}

/* Determine whether there are refutations after an attempted key move
 * at non-root level
 * @param si slice index
 * @param n maximum number of half moves until end state has to be reached
 * @param n_min minimum number of half-moves of interesting variations
 *              (slack_length_battle <= n_min <= slices[si].u.branch.length)
 * @param max_nr_refutations how many refutations should we look for
 * @return <=n solved  - return value is maximum number of moves
 *                       (incl. defense) needed
 *         n+2 refuted - <=max_nr_refutations refutations found
 *         n+4 refuted - >max_nr_refutations refutations found
 */
stip_length_type threat_writer_can_defend_in_n(slice_index si,
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

  result = defense_can_defend_in_n(next,n,n_min,max_nr_refutations);

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

/* Substitute links to proxy slices by the proxy's target
 * @param si root of sub-tree where to resolve proxies
 * @param st address of structure representing the traversal
 */
void threat_writer_resolve_proxies(slice_index si,
                                   stip_structure_traversal *st)
{
  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParamListEnd();

  pipe_resolve_proxies(si,st);
  proxy_slice_resolve(&slices[si].u.threat_writer.attack_side);

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}


/* The following enumeration type represents the state of threat
 * handler insertion; this helps us avoiding unnecessary threat
 * handler slices (e.g. we only need a threat collector slice if we
 * have already inserted a threat writer and a threat enforcer slice).
 */
typedef enum
{
  threat_handler_inserted_none,
  threat_handler_inserted_writer,
  threat_handler_inserted_enforcer,
  threat_handler_inserted_collector
} threat_handler_insertion_state;

/* Prepend a threat writer slice to a defense move slice
 * @param si identifies slice around which to insert threat handlers
 * @param st address of structure defining traversal
 */
static void prepend_threat_writer(slice_index si, stip_structure_traversal *st)
{
  threat_handler_insertion_state * const state = st->param;
  threat_handler_insertion_state const save_state = *state;
  stip_length_type const length = slices[si].u.branch.length;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParamListEnd();

  if (length>slack_length_battle+1)
  {
    *state = threat_handler_inserted_writer;
    stip_traverse_structure_children(si,st);
    *state = save_state;

    {
      slice_index const prev = slices[si].prev;
      slice_index const attack_side = slices[si].u.pipe.next;
      stip_length_type const min_length = slices[si].u.branch.min_length;
      pipe_append(prev,alloc_threat_writer_slice(length,min_length,attack_side));
    }
  }
  else
    stip_traverse_structure_children(si,st);

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

/* Prepend a threat enforcer slice to a variation writer slice
 * @param si identifies slice around which to insert threat handlers
 * @param st address of structure defining traversal
 */
static void prepend_threat_enforcer(slice_index si,
                                    stip_structure_traversal *st)
{
  threat_handler_insertion_state * const state = st->param;
  stip_length_type const length = slices[si].u.branch.length;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParamListEnd();


  if (*state==threat_handler_inserted_writer && length>slack_length_battle)
  {
    *state = threat_handler_inserted_enforcer;
    stip_traverse_structure_children(si,st);
    *state = threat_handler_inserted_writer;

    {
      slice_index const prev = slices[si].prev;
      stip_length_type const min_length = slices[si].u.branch.min_length;
      pipe_append(prev,alloc_threat_enforcer_slice(length,min_length));
    }
  }
  else
    stip_traverse_structure_children(si,st);

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

/* Append a threat collector slice to an attack move slice
 * @param si identifies slice around which to insert threat handlers
 * @param st address of structure defining traversal
 */
static void append_threat_collector(slice_index si, stip_structure_traversal *st)
{
  threat_handler_insertion_state * const state = st->param;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParamListEnd();


  if (*state==threat_handler_inserted_enforcer)
  {
    *state = threat_handler_inserted_collector;
    stip_traverse_structure_children(si,st);
    *state = threat_handler_inserted_enforcer;

    {
      stip_length_type const length = slices[si].u.branch.length;
      stip_length_type const min_length = slices[si].u.branch.min_length;
      pipe_append(si,alloc_threat_collector_slice(length,min_length));
    }
  }
  else
    stip_traverse_structure_children(si,st);

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

/* Reset the threat handler insertaion state for the subsequent slices
 * @param si identifies slice around which to insert threat handlers
 * @param st address of structure defining traversal
 */
static void threat_handler_reset_insertion_state(slice_index si,
                                                 stip_structure_traversal *st)
{
  threat_handler_insertion_state * const state = st->param;
  threat_handler_insertion_state const save_state = *state;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParamListEnd();

  *state = threat_handler_inserted_none;
  stip_traverse_structure_children(si,st);
  *state = save_state;

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

static stip_structure_visitor const threat_handler_inserters[] =
{
  &stip_traverse_structure_children,     /* STProxy */
  &append_threat_collector,              /* STAttackMove */
  &prepend_threat_writer,                /* STDefenseMove */
  &stip_structure_visitor_noop,          /* STHelpMove */
  &stip_structure_visitor_noop,          /* STHelpFork */
  &stip_structure_visitor_noop,          /* STSeriesMove */
  &stip_structure_visitor_noop,          /* STSeriesFork */
  &stip_structure_visitor_noop,          /* STLeafDirect */
  &stip_structure_visitor_noop,          /* STLeafHelp */
  &stip_structure_visitor_noop,          /* STLeafForced */
  &stip_traverse_structure_children,     /* STReciprocal */
  &stip_traverse_structure_children,     /* STQuodlibet */
  &threat_handler_reset_insertion_state, /* STNot */
  &stip_traverse_structure_children,     /* STMoveInverterRootSolvableFilter */
  &stip_traverse_structure_children,     /* STMoveInverterSolvableFilter */
  &stip_traverse_structure_children,     /* STMoveInverterSeriesFilter */
  &stip_traverse_structure_children,     /* STAttackRoot */
  &stip_traverse_structure_children,     /* STBattlePlaySolutionWriter */
  &stip_traverse_structure_children,     /* STPostKeyPlaySolutionWriter */
  &stip_traverse_structure_children,     /* STPostKeyPlaySuppressor */
  &stip_traverse_structure_children,     /* STContinuationWriter */
  &stip_traverse_structure_children,     /* STRefutationsWriter */
  &stip_traverse_structure_children,     /* STThreatWriter */
  &stip_traverse_structure_children,     /* STThreatEnforcer */
  &stip_traverse_structure_children,     /* STThreatCollector */
  &stip_traverse_structure_children,     /* STRefutationsCollector */
  &prepend_threat_enforcer,              /* STVariationWriter */
  &stip_traverse_structure_children,     /* STRefutingVariationWriter */
  &stip_traverse_structure_children,     /* STNoShortVariations */
  &stip_traverse_structure_children,     /* STAttackHashed */
  &stip_structure_visitor_noop,          /* STHelpRoot */
  &stip_structure_visitor_noop,          /* STHelpShortcut */
  &stip_traverse_structure_children,     /* STHelpHashed */
  &stip_structure_visitor_noop,          /* STSeriesRoot */
  &stip_structure_visitor_noop,          /* STSeriesShortcut */
  &stip_traverse_structure_children,     /* STParryFork */
  &stip_traverse_structure_children,     /* STSeriesHashed */
  &stip_traverse_structure_children,     /* STSelfCheckGuardRootSolvableFilter */
  &stip_traverse_structure_children,     /* STSelfCheckGuardSolvableFilter */
  &stip_traverse_structure_children,     /* STSelfCheckGuardRootDefenderFilter */
  &stip_traverse_structure_children,     /* STSelfCheckGuardAttackerFilter */
  &stip_traverse_structure_children,     /* STSelfCheckGuardDefenderFilter */
  &stip_traverse_structure_children,     /* STSelfCheckGuardHelpFilter */
  &stip_traverse_structure_children,     /* STSelfCheckGuardSeriesFilter */
  &stip_traverse_structure_children,     /* STDirectDefenderFilter */
  &stip_traverse_structure_children,     /* STReflexHelpFilter */
  &stip_traverse_structure_children,     /* STReflexSeriesFilter */
  &stip_traverse_structure_children,     /* STReflexAttackerFilter */
  &stip_traverse_structure_children,     /* STReflexDefenderFilter */
  &stip_traverse_structure_children,     /* STSelfDefense */
  &stip_traverse_structure_children,     /* STRestartGuardRootDefenderFilter */
  &stip_traverse_structure_children,     /* STRestartGuardHelpFilter */
  &stip_traverse_structure_children,     /* STRestartGuardSeriesFilter */
  &stip_traverse_structure_children,     /* STIntelligentHelpFilter */
  &stip_traverse_structure_children,     /* STIntelligentSeriesFilter */
  &stip_traverse_structure_children,     /* STGoalReachableGuardHelpFilter */
  &stip_traverse_structure_children,     /* STGoalReachableGuardSeriesFilter */
  &stip_traverse_structure_children,     /* STKeepMatingGuardRootDefenderFilter */
  &stip_traverse_structure_children,     /* STKeepMatingGuardAttackerFilter */
  &stip_traverse_structure_children,     /* STKeepMatingGuardDefenderFilter */
  &stip_traverse_structure_children,     /* STKeepMatingGuardHelpFilter */
  &stip_traverse_structure_children,     /* STKeepMatingGuardSeriesFilter */
  &stip_traverse_structure_children,     /* STMaxFlightsquares */
  &stip_traverse_structure_children,     /* STDegenerateTree */
  &stip_traverse_structure_children,     /* STMaxNrNonTrivial */
  &stip_traverse_structure_children,     /* STMaxNrNonTrivialCounter */
  &stip_traverse_structure_children,     /* STMaxThreatLength */
  &stip_traverse_structure_children,     /* STMaxTimeRootDefenderFilter */
  &stip_traverse_structure_children,     /* STMaxTimeDefenderFilter */
  &stip_traverse_structure_children,     /* STMaxTimeHelpFilter */
  &stip_traverse_structure_children,     /* STMaxTimeSeriesFilter */
  &stip_traverse_structure_children,     /* STMaxSolutionsRootSolvableFilter */
  &stip_traverse_structure_children,     /* STMaxSolutionsRootDefenderFilter */
  &stip_traverse_structure_children,     /* STMaxSolutionsHelpFilter */
  &stip_traverse_structure_children,     /* STMaxSolutionsSeriesFilter */
  &stip_traverse_structure_children,     /* STStopOnShortSolutionsRootSolvableFilter */
  &stip_traverse_structure_children,     /* STStopOnShortSolutionsHelpFilter */
  &stip_traverse_structure_children      /* STStopOnShortSolutionsSeriesFilter */
};

/* Instrument the stipulation representation so that it can deal with
 * threats
 */
void stip_insert_threat_handlers(void)
{
  stip_structure_traversal st;
  threat_handler_insertion_state state = threat_handler_inserted_none;
  unsigned int i;

  TraceFunctionEntry(__func__);
  TraceFunctionParamListEnd();

  TraceStipulation(root_slice);

  stip_structure_traversal_init(&st,&threat_handler_inserters,&state);
  stip_traverse_structure(root_slice,&st);

  for (i = 0; i<=maxply; ++i)
  {
    threat_lengths[i] = no_threats_found;
    threats[i] = table_nil;
  }

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}
