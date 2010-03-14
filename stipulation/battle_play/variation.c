#include "stipulation/battle_play/variation.h"
#include "pyoutput.h"
#include "pypipe.h"
#include "stipulation/branch.h"
#include "stipulation/battle_play/attack_play.h"
#include "trace.h"

#include <assert.h>

/* Allocate a STVariationWriter slice.
 * @param length maximum number of half-moves of slice (+ slack)
 * @param min_length minimum number of half-moves of slice (+ slack)
 * @return index of allocated slice
 */
static slice_index alloc_variation_writer_slice(stip_length_type length,
                                                stip_length_type min_length)
{
  slice_index result;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",length);
  TraceFunctionParam("%u",min_length);
  TraceFunctionParamListEnd();

  if (min_length<slack_length_battle)
    min_length += 2;
  result = alloc_branch(STVariationWriter,length,min_length);

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
boolean variation_writer_are_threats_refuted_in_n(table threats,
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
stip_length_type variation_writer_has_solution_in_n(slice_index si,
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
stip_length_type variation_writer_solve_threats_in_n(table threats,
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
stip_length_type variation_writer_solve_in_n(slice_index si,
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

  write_defense();
  result = attack_solve_in_n(next,n,n_min);

  TraceFunctionExit(__func__);
  TraceFunctionResult("%u",result);
  TraceFunctionResultEnd();
  return result;
}

/* Solve a slice - adapter for direct slices
 * @param si slice index
 * @return true iff >=1 solution was found
 */
boolean variation_writer_solve(slice_index si)
{
  boolean result;
  slice_index const next = slices[si].u.pipe.next;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParamListEnd();

  if (attack_has_solution(next)==has_solution)
  {
    write_defense();
    result = attack_solve(next);
  }
  else
    result = false;

  TraceFunctionExit(__func__);
  TraceFunctionResult("%u",result);
  TraceFunctionResultEnd();
  return result;
}

/* Prepend a variation writer to a slice
 * @param si identifies slice around which to insert try handlers
 */
static void variation_writer_prepend(slice_index si)
{
  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParamListEnd();

  {
    stip_length_type const length = slices[si].u.branch.length;
    stip_length_type const min_length = slices[si].u.branch.min_length;
    pipe_append(slices[si].prev,alloc_variation_writer_slice(length,min_length));
  }

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

/* Append a variation writer to a slice
 * @param si identifies slice around which to insert try handlers
 */
static void variation_writer_append(slice_index si)
{
  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParamListEnd();

  {
    stip_length_type const length = slices[si].u.branch.length;
    stip_length_type const min_length = slices[si].u.branch.min_length;
    pipe_append(si,alloc_variation_writer_slice(length,min_length));
  }

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

/* Inserting variation handlers in both parts of a binary
 * @param si identifies slice around which to insert try handlers
 * @param st address of structure defining traversal
 */
static void variation_handler_insert_binary(slice_index si, slice_traversal *st)
{
  boolean * const inserted = st->param;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParamListEnd();

  {
    boolean const save_inserted = *inserted;
    traverse_slices(slices[si].u.binary.op1,st);
    *inserted = save_inserted;
    traverse_slices(slices[si].u.binary.op2,st);
  }

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

/* Continue inserting variation handlers in this branch; start
 * inserting them in the subsequent play 
 * @param si identifies slice around which to insert try handlers
 * @param st address of structure defining traversal
 */
static void variation_handler_insert_fork(slice_index si, slice_traversal *st)
{
  boolean * const inserted = st->param;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParamListEnd();

  {
    boolean const save_inserted = *inserted;
    *inserted = false;
    traverse_slices(slices[si].u.branch_fork.towards_goal,st);
    *inserted = save_inserted;
  }

  traverse_slices(slices[si].u.branch_fork.next,st);

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

/* Prepend a variation writer, then mark subsequent slices visited to
 * avoid double insertion 
 * @param si identifies slice around which to insert try handlers
 * @param st address of structure defining traversal
 */
static void variation_writer_fork_prepend(slice_index si, slice_traversal *st)
{
  boolean * const inserted = st->param;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParamListEnd();

  if (!*inserted)
  {
    variation_writer_prepend(si);
    *inserted = true;
  }

  variation_handler_insert_fork(si,st);

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

/* Append a variation writer, then mark subsequent slices visited to
 * avoid double insertion 
 * @param si identifies slice around which to insert try handlers
 * @param st address of structure defining traversal
 */
static void variation_writer_fork_append(slice_index si, slice_traversal *st)
{
  boolean * const inserted = st->param;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParamListEnd();

  if (!*inserted)
  {
    variation_writer_append(si);
    *inserted = true;
  }

  variation_handler_insert_fork(si,st);

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

/* Append a variation writer if none has been inserted before
 * @param si identifies slice around which to insert try handlers
 * @param st address of structure defining traversal
 */
static void variation_writer_branch_append(slice_index si, slice_traversal *st)
{
  boolean * const inserted = st->param;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParamListEnd();

  slice_traverse_children(si,st);

  if (!*inserted)
  {
    variation_writer_append(si);
    *inserted = true;
  }

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

/* Avoid the parrying move when inserting variation writers
 * @param si identifies slice around which to insert try handlers
 * @param st address of structure defining traversal
 */
static void variation_handler_avoid_parrying(slice_index si, slice_traversal *st)
{
  slice_index const next = slices[si].u.pipe.next;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParamListEnd();

  traverse_slices(next,st);

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

static slice_operation const variation_handler_inserters[] =
{
  &slice_traverse_children,          /* STProxy */
  &slice_traverse_children,          /* STAttackMove */
  &slice_traverse_children,          /* STDefenseMove */
  &slice_traverse_children,          /* STHelpMove */
  &slice_traverse_children,          /* STHelpFork */
  &slice_traverse_children,          /* STSeriesMove */
  &slice_traverse_children,          /* STSeriesFork */
  &slice_operation_noop,             /* STLeafDirect */
  &slice_operation_noop,             /* STLeafHelp */
  &slice_operation_noop,             /* STLeafForced */
  &variation_handler_insert_binary,  /* STReciprocal */
  &variation_handler_insert_binary,  /* STQuodlibet */
  &slice_traverse_children,          /* STNot */
  &slice_traverse_children,          /* STMoveInverterRootSolvableFilter */
  &slice_traverse_children,          /* STMoveInverterSolvableFilter */
  &slice_traverse_children,          /* STMoveInverterSeriesFilter */
  &slice_traverse_children,          /* STAttackRoot */
  &slice_traverse_children,          /* STBattlePlaySolutionWriter */
  &slice_traverse_children,          /* STPostKeyPlaySolutionWriter */
  &slice_traverse_children,          /* STContinuationWriter */
  &slice_traverse_children,          /* STTryWriter */
  &slice_traverse_children,          /* STThreatWriter */
  &slice_traverse_children,          /* STThreatEnforcer */
  &slice_traverse_children,          /* STRefutationsCollector */
  &slice_traverse_children,          /* STVariationWriter */
  &slice_traverse_children,          /* STRefutingVariationWriter */
  &slice_traverse_children,          /* STNoShortVariations */
  &slice_traverse_children,          /* STAttackHashed */
  &slice_traverse_children,          /* STHelpRoot */
  &slice_traverse_children,          /* STHelpShortcut */
  &slice_traverse_children,          /* STHelpHashed */
  &slice_traverse_children,          /* STSeriesRoot */
  &slice_traverse_children,          /* STSeriesShortcut */
  &variation_handler_avoid_parrying, /* STParryFork */
  &slice_traverse_children,          /* STSeriesHashed */
  &slice_traverse_children,          /* STSelfCheckGuardRootSolvableFilter */
  &slice_traverse_children,          /* STSelfCheckGuardSolvableFilter */
  &slice_traverse_children,          /* STSelfCheckGuardRootDefenderFilter */
  &variation_writer_branch_append,   /* STSelfCheckGuardAttackerFilter */
  &slice_traverse_children,          /* STSelfCheckGuardDefenderFilter */
  &slice_traverse_children,          /* STSelfCheckGuardHelpFilter */
  &slice_traverse_children,          /* STSelfCheckGuardSeriesFilter */
  &slice_traverse_children,          /* STDirectDefenseRootSolvableFilter */
  &variation_writer_fork_prepend,    /* STDirectDefense */
  &slice_traverse_children,          /* STReflexHelpFilter */
  &slice_traverse_children,          /* STReflexSeriesFilter */
  &slice_traverse_children,          /* STReflexRootSolvableFilter */
  &variation_writer_fork_append,     /* STReflexAttackerFilter */
  &slice_traverse_children,          /* STReflexDefenderFilter */
  &slice_traverse_children,          /* STSelfAttack */
  &variation_writer_fork_append,     /* STSelfDefense */
  &slice_traverse_children,          /* STRestartGuardRootDefenderFilter */
  &slice_traverse_children,          /* STRestartGuardHelpFilter */
  &slice_traverse_children,          /* STRestartGuardSeriesFilter */
  &slice_traverse_children,          /* STIntelligentHelpFilter */
  &slice_traverse_children,          /* STIntelligentSeriesFilter */
  &slice_traverse_children,          /* STGoalReachableGuardHelpFilter */
  &slice_traverse_children,          /* STGoalReachableGuardSeriesFilter */
  &slice_traverse_children,          /* STKeepMatingGuardRootDefenderFilter */
  &slice_traverse_children,          /* STKeepMatingGuardAttackerFilter */
  &slice_traverse_children,          /* STKeepMatingGuardDefenderFilter */
  &slice_traverse_children,          /* STKeepMatingGuardHelpFilter */
  &slice_traverse_children,          /* STKeepMatingGuardSeriesFilter */
  &slice_traverse_children,          /* STMaxFlightsquares */
  &slice_traverse_children,          /* STDegenerateTree */
  &slice_traverse_children,          /* STMaxNrNonTrivial */
  &slice_traverse_children,          /* STMaxThreatLength */
  &slice_traverse_children,          /* STMaxTimeRootDefenderFilter */
  &slice_traverse_children,          /* STMaxTimeDefenderFilter */
  &slice_traverse_children,          /* STMaxTimeHelpFilter */
  &slice_traverse_children,          /* STMaxTimeSeriesFilter */
  &slice_traverse_children,          /* STMaxSolutionsRootSolvableFilter */
  &slice_traverse_children,          /* STMaxSolutionsRootDefenderFilter */
  &slice_traverse_children,          /* STMaxSolutionsHelpFilter */
  &slice_traverse_children,          /* STMaxSolutionsSeriesFilter */
  &slice_traverse_children,          /* STStopOnShortSolutionsRootSolvableFilter */
  &slice_traverse_children,          /* STStopOnShortSolutionsHelpFilter */
  &slice_traverse_children           /* STStopOnShortSolutionsSeriesFilter */
};

/* Instrument the stipulation representation so that it can deal with
 * variations
 */
void stip_insert_variation_handlers(void)
{
  slice_traversal st;
  boolean inserted = false;

  TraceFunctionEntry(__func__);
  TraceFunctionParamListEnd();

  TraceStipulation(root_slice);

  slice_traversal_init(&st,&variation_handler_inserters,&inserted);
  traverse_slices(root_slice,&st);

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}
