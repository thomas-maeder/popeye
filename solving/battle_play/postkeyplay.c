#include "stipulation/battle_play/postkeyplay.h"
#include "pyoutput.h"
#include "pypipe.h"
#include "stipulation/branch.h"
#include "stipulation/battle_play/continuation.h"
#include "trace.h"

#include <assert.h>

/* Allocate a STPostKeyPlaySolutionWriter defender slice.
 * @param length maximum number of half-moves of slice (+ slack)
 * @param min_length minimum number of half-moves of slice (+ slack)
 * @return index of allocated slice
 */
static
slice_index alloc_postkey_solution_writer_slice(stip_length_type length,
                                                stip_length_type min_length)
{
  slice_index result;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",length);
  TraceFunctionParam("%u",min_length);
  TraceFunctionParamListEnd();

  result = alloc_branch(STPostKeyPlaySolutionWriter,length,min_length);

  TraceFunctionExit(__func__);
  TraceFunctionResult("%u",result);
  TraceFunctionResultEnd();
  return result;
}

/* Solve at root level.
 * @param si slice index
 * @return true iff >=1 solution was found
 */
boolean postkey_solution_writer_root_solve(slice_index si)
{
  boolean result;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParamListEnd();

  init_output(si);

  if (continuation_writer_solve_postkey(si))
  {
    write_end_of_solution();
    result = true;
  }
  else
    result = false;

  write_end_of_solution_phase();

  TraceFunctionExit(__func__);
  TraceFunctionResult("%u",result);
  TraceFunctionResultEnd();
  return result;
}

/* Allocate a STRefutingVariationWriter slice.
 * @param length maximum number of half-moves of slice (+ slack)
 * @param min_length minimum number of half-moves of slice (+ slack)
 * @return index of allocated slice
 */
static
slice_index alloc_refuting_variation_writer_slice(stip_length_type length,
                                                  stip_length_type min_length)
{
  slice_index result;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",length);
  TraceFunctionParam("%u",min_length);
  TraceFunctionParamListEnd();

  if (min_length<slack_length_battle)
    min_length += 2;
  result = alloc_branch(STRefutingVariationWriter,length,min_length);

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
refuting_variation_writer_are_threats_refuted_in_n(table threats,
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
stip_length_type
refuting_variation_writer_has_solution_in_n(slice_index si,
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
stip_length_type
refuting_variation_writer_solve_threats_in_n(table threats,
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
stip_length_type refuting_variation_writer_solve_in_n(slice_index si,
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

  result = attack_solve_in_n(next,n,n_min);
  if (result>n)
    write_refutation_mark();

  TraceFunctionExit(__func__);
  TraceFunctionResult("%u",result);
  TraceFunctionResultEnd();
  return result;
}

/* Append a variation writer
 * @param si identifies slice around which to insert try handlers
 * @param st address of structure defining traversal
 */
static void prepend_refutes_writer(slice_index si, stip_structure_traversal *st)
{
  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParamListEnd();

  {
    stip_length_type const length = slices[si].u.branch.length;
    stip_length_type const min_length = slices[si].u.branch.min_length;
    slice_index const prev = slices[si].prev;
    pipe_append(prev,alloc_refuting_variation_writer_slice(length,min_length));
  }

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

/* Append a variation writer
 * @param si identifies slice around which to insert try handlers
 * @param st address of structure defining traversal
 */
static void substitute_solution_writer(slice_index si, stip_structure_traversal *st)
{
  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParamListEnd();

  stip_traverse_structure_children(si,st);

  {
    stip_length_type const length = slices[si].u.branch.length;
    stip_length_type const min_length = slices[si].u.branch.min_length;
    pipe_replace(si,alloc_postkey_solution_writer_slice(length,min_length));
  }

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

static stip_structure_visitor const postkey_handler_inserters[] =
{
  &stip_traverse_structure_children,           /* STProxy */
  &stip_traverse_structure_children,           /* STAttackMove */
  &stip_traverse_structure_children,           /* STDefenseMove */
  &stip_structure_visitor_noop,              /* STHelpMove */
  &stip_structure_visitor_noop,              /* STHelpFork */
  &stip_structure_visitor_noop,              /* STSeriesMove */
  &stip_structure_visitor_noop,              /* STSeriesFork */
  &stip_structure_visitor_noop,              /* STLeafDirect */
  &stip_structure_visitor_noop,              /* STLeafHelp */
  &stip_structure_visitor_noop,              /* STLeafForced */
  &stip_traverse_structure_children,           /* STReciprocal */
  &stip_traverse_structure_children,           /* STQuodlibet */
  &stip_traverse_structure_children,           /* STNot */
  &stip_traverse_structure_children,           /* STMoveInverterRootSolvableFilter */
  &stip_traverse_structure_children,           /* STMoveInverterSolvableFilter */
  &stip_structure_visitor_noop,              /* STMoveInverterSeriesFilter */
  &stip_traverse_structure_children,           /* STAttackRoot */
  &stip_traverse_structure_children,           /* STBattlePlaySolutionWriter */
  &stip_traverse_structure_children,           /* STPostKeyPlaySolutionWriter */
  &substitute_solution_writer,        /* STContinuationWriter */
  &stip_traverse_structure_children,           /* STTryWriter */
  &stip_traverse_structure_children,           /* STThreatWriter */
  &stip_traverse_structure_children,           /* STThreatEnforcer */
  &stip_traverse_structure_children,           /* STRefutationsCollector */
  &prepend_refutes_writer,            /* STVariationWriter */
  &stip_traverse_structure_children,           /* STRefutingVariationWriter */
  &stip_traverse_structure_children,           /* STNoShortVariations */
  &stip_traverse_structure_children,           /* STAttackHashed */
  &stip_structure_visitor_noop,              /* STHelpRoot */
  &stip_structure_visitor_noop,              /* STHelpShortcut */
  &stip_structure_visitor_noop,              /* STHelpHashed */
  &stip_structure_visitor_noop,              /* STSeriesRoot */
  &stip_structure_visitor_noop,              /* STSeriesShortcut */
  &stip_structure_visitor_noop,              /* STParryFork */
  &stip_structure_visitor_noop,              /* STSeriesHashed */
  &stip_traverse_structure_children,           /* STSelfCheckGuardRootSolvableFilter */
  &stip_traverse_structure_children,           /* STSelfCheckGuardSolvableFilter */
  &stip_traverse_structure_children,           /* STSelfCheckGuardRootDefenderFilter */
  &stip_traverse_structure_children,           /* STSelfCheckGuardAttackerFilter */
  &stip_traverse_structure_children,           /* STSelfCheckGuardDefenderFilter */
  &stip_structure_visitor_noop,              /* STSelfCheckGuardHelpFilter */
  &stip_structure_visitor_noop,              /* STSelfCheckGuardSeriesFilter */
  &stip_traverse_structure_children,           /* STDirectDefenseRootSolvableFilter */
  &stip_traverse_structure_children,           /* STDirectDefense */
  &stip_structure_visitor_noop,              /* STReflexHelpFilter */
  &stip_structure_visitor_noop,              /* STReflexSeriesFilter */
  &stip_traverse_structure_children,           /* STReflexRootSolvableFilter */
  &stip_traverse_structure_children,           /* STReflexAttackerFilter */
  &stip_traverse_structure_children,           /* STReflexDefenderFilter */
  &stip_traverse_structure_children,           /* STSelfAttack */
  &stip_traverse_structure_children,           /* STSelfDefense */
  &stip_traverse_structure_children,           /* STRestartGuardRootDefenderFilter */
  &stip_structure_visitor_noop,              /* STRestartGuardHelpFilter */
  &stip_structure_visitor_noop,              /* STRestartGuardSeriesFilter */
  &stip_structure_visitor_noop,              /* STIntelligentHelpFilter */
  &stip_structure_visitor_noop,              /* STIntelligentSeriesFilter */
  &stip_structure_visitor_noop,              /* STGoalReachableGuardHelpFilter */
  &stip_structure_visitor_noop,              /* STGoalReachableGuardSeriesFilter */
  &stip_traverse_structure_children,           /* STKeepMatingGuardRootDefenderFilter */
  &stip_traverse_structure_children,           /* STKeepMatingGuardAttackerFilter */
  &stip_traverse_structure_children,           /* STKeepMatingGuardDefenderFilter */
  &stip_structure_visitor_noop,              /* STKeepMatingGuardHelpFilter */
  &stip_structure_visitor_noop,              /* STKeepMatingGuardSeriesFilter */
  &stip_traverse_structure_children,           /* STMaxFlightsquares */
  &stip_traverse_structure_children,           /* STDegenerateTree */
  &stip_traverse_structure_children,           /* STMaxNrNonTrivial */
  &stip_traverse_structure_children,           /* STMaxThreatLength */
  &stip_traverse_structure_children,           /* STMaxTimeRootDefenderFilter */
  &stip_traverse_structure_children,           /* STMaxTimeDefenderFilter */
  &stip_structure_visitor_noop,              /* STMaxTimeHelpFilter */
  &stip_structure_visitor_noop,              /* STMaxTimeSeriesFilter */
  &stip_traverse_structure_children,           /* STMaxSolutionsRootSolvableFilter */
  &stip_traverse_structure_children,           /* STMaxSolutionsRootDefenderFilter */
  &stip_structure_visitor_noop,              /* STMaxSolutionsHelpFilter */
  &stip_structure_visitor_noop,              /* STMaxSolutionsSeriesFilter */
  &stip_traverse_structure_children,           /* STStopOnShortSolutionsRootSolvableFilter */
  &stip_structure_visitor_noop,              /* STStopOnShortSolutionsHelpFilter */
  &stip_structure_visitor_noop               /* STStopOnShortSolutionsSeriesFilter */
};

/* Instrument the stipulation representation so that it can write
 * variations
 */
void stip_insert_postkey_handlers(void)
{
  stip_structure_traversal st;

  TraceFunctionEntry(__func__);
  TraceFunctionParamListEnd();

  stip_structure_traversal_init(&st,&postkey_handler_inserters,0);
  stip_traverse_structure(root_slice,&st);

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}
