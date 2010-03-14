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
 */
static void continuation_writer_prepend(slice_index si, stip_structure_traversal *st)
{
  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParamListEnd();

  stip_traverse_structure_children(si,st);

  {
    stip_length_type const length = slices[si].u.branch.length;
    stip_length_type const min_length = slices[si].u.branch.min_length;
    pipe_append(slices[si].prev,
                alloc_continuation_writer_slice(length,min_length));
  }

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

static stip_structure_visitor const continuation_handler_inserters[] =
{
  &stip_traverse_structure_children,     /* STProxy */
  &stip_traverse_structure_children,     /* STAttackMove */
  &continuation_writer_prepend, /* STDefenseMove */
  &stip_traverse_structure_children,     /* STHelpMove */
  &stip_traverse_structure_children,     /* STHelpFork */
  &stip_traverse_structure_children,     /* STSeriesMove */
  &stip_traverse_structure_children,     /* STSeriesFork */
  &stip_structure_visitor_noop,        /* STLeafDirect */
  &stip_structure_visitor_noop,        /* STLeafHelp */
  &stip_structure_visitor_noop,        /* STLeafForced */
  &stip_traverse_structure_children,     /* STReciprocal */
  &stip_traverse_structure_children,     /* STQuodlibet */
  &stip_traverse_structure_children,     /* STNot */
  &stip_traverse_structure_children,     /* STMoveInverterRootSolvableFilter */
  &stip_traverse_structure_children,     /* STMoveInverterSolvableFilter */
  &stip_traverse_structure_children,     /* STMoveInverterSeriesFilter */
  &stip_traverse_structure_children,     /* STAttackRoot */
  &stip_traverse_structure_children,     /* STBattlePlaySolutionWriter */
  &stip_traverse_structure_children,     /* STPostKeyPlaySolutionWriter */
  &stip_traverse_structure_children,     /* STContinuationWriter */
  &stip_traverse_structure_children,     /* STTryWriter */
  &stip_traverse_structure_children,     /* STThreatWriter */
  &stip_traverse_structure_children,     /* STThreatEnforcer */
  &stip_traverse_structure_children,     /* STRefutationsCollector */
  &stip_traverse_structure_children,     /* STVariationWriter */
  &stip_traverse_structure_children,     /* STRefutingVariationWriter */
  &stip_traverse_structure_children,     /* STNoShortVariations */
  &stip_traverse_structure_children,     /* STAttackHashed */
  &stip_traverse_structure_children,     /* STHelpRoot */
  &stip_traverse_structure_children,     /* STHelpShortcut */
  &stip_traverse_structure_children,     /* STHelpHashed */
  &stip_traverse_structure_children,     /* STSeriesRoot */
  &stip_traverse_structure_children,     /* STSeriesShortcut */
  &stip_traverse_structure_children,     /* STParryFork */
  &stip_traverse_structure_children,     /* STSeriesHashed */
  &stip_traverse_structure_children,     /* STSelfCheckGuardRootSolvableFilter */
  &stip_traverse_structure_children,     /* STSelfCheckGuardSolvableFilter */
  &stip_traverse_structure_children,     /* STSelfCheckGuardRootDefenderFilter */
  &stip_traverse_structure_children,     /* STSelfCheckGuardAttackerFilter */
  &stip_traverse_structure_children,     /* STSelfCheckGuardDefenderFilter */
  &stip_traverse_structure_children,     /* STSelfCheckGuardHelpFilter */
  &stip_traverse_structure_children,     /* STSelfCheckGuardSeriesFilter */
  &stip_traverse_structure_children,     /* STDirectDefenseRootSolvableFilter */
  &stip_traverse_structure_children,     /* STDirectDefense */
  &stip_traverse_structure_children,     /* STReflexHelpFilter */
  &stip_traverse_structure_children,     /* STReflexSeriesFilter */
  &stip_traverse_structure_children,     /* STReflexRootSolvableFilter */
  &stip_traverse_structure_children,     /* STReflexAttackerFilter */
  &stip_traverse_structure_children,     /* STReflexDefenderFilter */
  &stip_traverse_structure_children,     /* STSelfAttack */
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
 * tries
 */
void stip_insert_continuation_handlers(void)
{
  stip_structure_traversal st;

  TraceFunctionEntry(__func__);
  TraceFunctionParamListEnd();

  stip_structure_traversal_init(&st,&continuation_handler_inserters,0);
  stip_traverse_structure(root_slice,&st);

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}
