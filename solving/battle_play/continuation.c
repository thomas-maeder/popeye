#include "stipulation/battle_play/continuation.h"
#include "pydata.h"
#include "pypipe.h"
#include "stipulation/branch.h"
#include "stipulation/battle_play/defense_play.h"
#include "pyoutput.h"
#include "trace.h"

#include <assert.h>

/* Allocate a STContinuationWriter defender slice.
 * @param length maximum number of half-moves of slice (+ slack)
 * @param min_length minimum number of half-moves of slice (+ slack)
 * @return index of allocated slice
 */
static slice_index alloc_continuation_writer_slice(stip_length_type length,
                                                   stip_length_type min_length)
{
  slice_index result;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",length);
  TraceFunctionParam("%u",min_length);
  TraceFunctionParamListEnd();

  result = alloc_branch(STContinuationWriter,length,min_length);

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
 * @return true iff the defender can defend
 */
boolean continuation_writer_defend_in_n(slice_index si, stip_length_type n)
{
  boolean result;
  slice_index const next = slices[si].u.pipe.next;
  unsigned int nr_refutations;
  unsigned int const max_nr_allowed_refutations = 0;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParam("%u",n);
  TraceFunctionParamListEnd();

  nr_refutations = defense_can_defend_in_n(next,n,max_nr_allowed_refutations);
  if (nr_refutations==0)
  {
    result = false;
    write_attack(attack_regular);
    {
      boolean const defend_result = defense_defend_in_n(next,n);
      assert(!defend_result);
    }
  }
  else
    result = true;

  TraceFunctionExit(__func__);
  TraceFunctionResult("%u",result);
  TraceFunctionResultEnd();
  return result;
}

/* Solve postkey play play after the move that has just
 * been played in the current ply.
 * @param si slice index
 * @return true iff >=1 variation or a threat was found
 */
boolean continuation_writer_solve_postkey(slice_index si)
{
  boolean result;
  slice_index const next = slices[si].u.pipe.next;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParamListEnd();

  output_start_postkey_level();

  if (OptFlag[solvariantes])
    result = !defense_root_defend(next);
  else
    result = false;

  output_end_postkey_level();

  TraceFunctionExit(__func__);
  TraceFunctionResult("%u",result);
  TraceFunctionResultEnd();
  return result;
}

/* Try to defend after an attempted key move at root level
 * @param si slice index
 * @return true iff the defending side can successfully defend
 */
boolean continuation_writer_root_defend(slice_index si)
{
  boolean result;
  slice_index const next = slices[si].u.pipe.next;
  stip_length_type const length = slices[si].u.branch.length;
  unsigned int nr_refutations;
  unsigned int const max_nr_allowed_refutations = 0;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParamListEnd();

  nr_refutations = defense_can_defend_in_n(next,
                                           length,
                                           max_nr_allowed_refutations);
  if (nr_refutations==0)
  {
    result = false;
    write_attack(attack_key);
    continuation_writer_solve_postkey(si);
    write_end_of_solution();
  }
  else
    result = true;

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
unsigned int continuation_writer_can_defend_in_n(slice_index si,
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

/* Prepend a continuation writer
 * @param si identifies slice around which to insert try handlers
 * @param st address of structure defining traversal
 * @return true
 */
static boolean continuation_writer_prepend(slice_index si, slice_traversal *st)
{
  boolean const result = true;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParamListEnd();

  slice_traverse_children(si,st);

  {
    stip_length_type const length = slices[si].u.branch.length;
    stip_length_type const min_length = slices[si].u.branch.min_length;
    pipe_append(slices[si].prev,
                alloc_continuation_writer_slice(length,min_length));
  }

  TraceFunctionExit(__func__);
  TraceFunctionResult("%u",result);
  TraceFunctionResultEnd();
  return result;
}

static slice_operation const continuation_handler_inserters[] =
{
  &slice_traverse_children,     /* STProxy */
  &slice_traverse_children,     /* STAttackMove */
  &continuation_writer_prepend, /* STDefenseMove */
  &slice_traverse_children,     /* STHelpMove */
  &slice_traverse_children,     /* STHelpFork */
  &slice_traverse_children,     /* STSeriesMove */
  &slice_traverse_children,     /* STSeriesFork */
  &slice_operation_noop,        /* STLeafDirect */
  &slice_operation_noop,        /* STLeafHelp */
  &slice_operation_noop,        /* STLeafForced */
  &slice_traverse_children,     /* STReciprocal */
  &slice_traverse_children,     /* STQuodlibet */
  &slice_traverse_children,     /* STNot */
  &slice_traverse_children,     /* STMoveInverterRootSolvableFilter */
  &slice_traverse_children,     /* STMoveInverterSolvableFilter */
  &slice_traverse_children,     /* STMoveInverterSeriesFilter */
  &slice_traverse_children,     /* STAttackRoot */
  &slice_traverse_children,     /* STBattlePlaySolutionWriter */
  &slice_traverse_children,     /* STPostKeyPlaySolutionWriter */
  &slice_traverse_children,     /* STContinuationWriter */
  &slice_traverse_children,     /* STTryWriter */
  &slice_traverse_children,     /* STThreatWriter */
  &slice_traverse_children,     /* STThreatEnforcer */
  &slice_traverse_children,     /* STRefutationsCollector */
  &slice_traverse_children,     /* STVariationWriter */
  &slice_traverse_children,     /* STRefutingVariationWriter */
  &slice_traverse_children,     /* STNoShortVariations */
  &slice_traverse_children,     /* STAttackHashed */
  &slice_traverse_children,     /* STHelpRoot */
  &slice_traverse_children,     /* STHelpShortcut */
  &slice_traverse_children,     /* STHelpHashed */
  &slice_traverse_children,     /* STSeriesRoot */
  &slice_traverse_children,     /* STSeriesShortcut */
  &slice_traverse_children,     /* STParryFork */
  &slice_traverse_children,     /* STSeriesHashed */
  &slice_traverse_children,     /* STSelfCheckGuardRootSolvableFilter */
  &slice_traverse_children,     /* STSelfCheckGuardSolvableFilter */
  &slice_traverse_children,     /* STSelfCheckGuardRootDefenderFilter */
  &slice_traverse_children,     /* STSelfCheckGuardAttackerFilter */
  &slice_traverse_children,     /* STSelfCheckGuardDefenderFilter */
  &slice_traverse_children,     /* STSelfCheckGuardHelpFilter */
  &slice_traverse_children,     /* STSelfCheckGuardSeriesFilter */
  &slice_traverse_children,     /* STDirectDefenseRootSolvableFilter */
  &slice_traverse_children,     /* STDirectDefense */
  &slice_traverse_children,     /* STReflexHelpFilter */
  &slice_traverse_children,     /* STReflexSeriesFilter */
  &slice_traverse_children,     /* STReflexRootSolvableFilter */
  &slice_traverse_children,     /* STReflexAttackerFilter */
  &slice_traverse_children,     /* STReflexDefenderFilter */
  &slice_traverse_children,     /* STSelfAttack */
  &slice_traverse_children,     /* STSelfDefense */
  &slice_traverse_children,     /* STRestartGuardRootDefenderFilter */
  &slice_traverse_children,     /* STRestartGuardHelpFilter */
  &slice_traverse_children,     /* STRestartGuardSeriesFilter */
  &slice_traverse_children,     /* STIntelligentHelpFilter */
  &slice_traverse_children,     /* STIntelligentSeriesFilter */
  &slice_traverse_children,     /* STGoalReachableGuardHelpFilter */
  &slice_traverse_children,     /* STGoalReachableGuardSeriesFilter */
  &slice_traverse_children,     /* STKeepMatingGuardRootDefenderFilter */
  &slice_traverse_children,     /* STKeepMatingGuardAttackerFilter */
  &slice_traverse_children,     /* STKeepMatingGuardDefenderFilter */
  &slice_traverse_children,     /* STKeepMatingGuardHelpFilter */
  &slice_traverse_children,     /* STKeepMatingGuardSeriesFilter */
  &slice_traverse_children,     /* STMaxFlightsquares */
  &slice_traverse_children,     /* STDegenerateTree */
  &slice_traverse_children,     /* STMaxNrNonTrivial */
  &slice_traverse_children,     /* STMaxThreatLength */
  &slice_traverse_children,     /* STMaxTimeRootDefenderFilter */
  &slice_traverse_children,     /* STMaxTimeDefenderFilter */
  &slice_traverse_children,     /* STMaxTimeHelpFilter */
  &slice_traverse_children,     /* STMaxTimeSeriesFilter */
  &slice_traverse_children,     /* STMaxSolutionsRootSolvableFilter */
  &slice_traverse_children,     /* STMaxSolutionsRootDefenderFilter */
  &slice_traverse_children,     /* STMaxSolutionsHelpFilter */
  &slice_traverse_children,     /* STMaxSolutionsSeriesFilter */
  &slice_traverse_children,     /* STStopOnShortSolutionsRootSolvableFilter */
  &slice_traverse_children,     /* STStopOnShortSolutionsHelpFilter */
  &slice_traverse_children      /* STStopOnShortSolutionsSeriesFilter */
};

/* Instrument the stipulation representation so that it can deal with
 * tries
 */
void stip_insert_continuation_handlers(void)
{
  slice_traversal st;

  TraceFunctionEntry(__func__);
  TraceFunctionParamListEnd();

  slice_traversal_init(&st,&continuation_handler_inserters,0);
  traverse_slices(root_slice,&st);

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}
