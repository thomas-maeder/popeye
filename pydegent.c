#include "pydegent.h"
#include "pydata.h"
#include "pypipe.h"
#include "pypipe.h"
#include "pyoutput.h"
#include "stipulation/branch.h"
#include "stipulation/battle_play/attack_play.h"
#include "trace.h"

#include <assert.h>
#include <stdlib.h>

static stip_length_type max_length_short_solutions;

/* Reset the max threats setting to off
 */
void reset_max_nr_nontrivial_length(void)
{
  max_length_short_solutions = no_stip_length;
}

/* Read the requested max threat length setting from a text token
 * entered by the user
 * @param textToken text token from which to read
 * @return true iff max threat setting was successfully read
 */
void init_degenerate_tree(stip_length_type max_length_short)
{
  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",max_length_short);
  TraceFunctionParamListEnd();

  max_length_short_solutions = max_length_short+1;

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

/* **************** Initialisation ***************
 */

/* Allocate a STDegenerateTree slice
 * @return allocated slice
 */
static slice_index alloc_degenerate_tree_guard_slice(void)
{
  slice_index result;

  TraceFunctionEntry(__func__);
  TraceFunctionParamListEnd();

  result = alloc_pipe(STDegenerateTree);

  TraceFunctionExit(__func__);
  TraceFunctionResult("%u",result);
  TraceFunctionResultEnd();
  return result;
}


/* **************** Implementation of interface Direct **********
 */

/* Determine whether there is a solution in n half moves, by trying
 * n_min, n_min+2 ... n half-moves.
 * @param si slice index of slice being solved
 * @param n maximum number of half moves until end state has to be reached
 * @param n_min minimal number of half moves to try
 * @param n_max_unsolvable maximum number of half-moves that we
 *                         know have no solution
 * @return length of solution found, i.e.:
 *            slack_length_battle-2 defense has turned out to be illegal
 *            <=n length of shortest solution found
 *            n+2 no solution found
 */
stip_length_type
degenerate_tree_direct_has_solution_in_n(slice_index si,
                                         stip_length_type n,
                                         stip_length_type n_min,
                                         stip_length_type n_max_unsolvable)
{
  stip_length_type result = n+2;
  stip_length_type const parity = n%2;
  slice_index const next = slices[si].u.pipe.next;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParam("%u",n);
  TraceFunctionParam("%u",n_min);
  TraceFunctionParam("%u",n_max_unsolvable);
  TraceFunctionParamListEnd();

  if (n>max_length_short_solutions+parity
      && max_length_short_solutions>=slack_length_battle+3)
  {
    stip_length_type const n_interm = max_length_short_solutions-2+parity;
    result = attack_has_solution_in_n(next,n_interm,n_min,n_max_unsolvable);
    if (result>n_interm)
      result = attack_has_solution_in_n(next,n,n,n_interm);
  }
  else
    result = attack_has_solution_in_n(next,n,n_min,n_max_unsolvable);

  TraceFunctionExit(__func__);
  TraceFunctionResult("%u",result);
  TraceFunctionResultEnd();
  return result;
}

/* Solve a slice, by trying n_min, n_min+2 ... n half-moves.
 * @param si slice index
 * @param n maximum number of half moves until goal
 * @param n_max_unsolvable maximum number of half-moves that we
 *                         know have no solution
 * @return length of solution found and written, i.e.:
 *            slack_length_battle-2 defense has turned out to be illegal
 *            <=n length of shortest solution found
 *            n+2 no solution found
 */
stip_length_type
degenerate_tree_direct_solve_in_n(slice_index si,
                                  stip_length_type n,
                                  stip_length_type n_max_unsolvable)
{
  stip_length_type result;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParam("%u",n);
  TraceFunctionParam("%u",n_max_unsolvable);
  TraceFunctionParamListEnd();

  result = attack_solve_in_n(slices[si].u.pipe.next,n,n_max_unsolvable);

  TraceFunctionExit(__func__);
  TraceFunctionResult("%u",result);
  TraceFunctionResultEnd();
  return result;
}

/* **************** Stipulation instrumentation ***************
 */

static void degenerate_tree_inserter_attack_move(slice_index si,
                                                 stip_structure_traversal *st)
{
  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParamListEnd();

  if (slices[si].u.branch.length>=slack_length_battle+2)
    pipe_append(slices[si].prev,alloc_degenerate_tree_guard_slice());

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

static stip_structure_visitor const degenerate_tree_guards_inserters[] =
{
  &stip_traverse_structure_children,     /* STProxy */
  &degenerate_tree_inserter_attack_move, /* STAttackMove */
  &stip_traverse_structure_children,     /* STDefenseMove */
  &stip_traverse_structure_children,     /* STDefenseMoveAgainstGoal */
  &stip_traverse_structure_children,     /* STHelpMove */
  &stip_traverse_structure_children,     /* STHelpMoveToGoal */
  &stip_traverse_structure_children,     /* STHelpFork */
  &stip_traverse_structure_children,     /* STSeriesMove */
  &stip_traverse_structure_children,     /* STSeriesMoveToGoal */
  &stip_traverse_structure_children,     /* STSeriesFork */
  &stip_traverse_structure_children,     /* STGoalReachedTester */
  &stip_traverse_structure_children,     /* STLeaf */
  &stip_traverse_structure_children,     /* STReciprocal */
  &stip_traverse_structure_children,     /* STQuodlibet */
  &stip_traverse_structure_children,     /* STNot */
  &stip_traverse_structure_children,     /* STMoveInverterRootSolvableFilter */
  &stip_traverse_structure_children,     /* STMoveInverterSolvableFilter */
  &stip_traverse_structure_children,     /* STMoveInverterSeriesFilter */
  &stip_traverse_structure_children,     /* STAttackRoot */
  &stip_traverse_structure_children,     /* STDefenseRoot */
  &stip_traverse_structure_children,     /* STPostKeyPlaySuppressor */
  &stip_traverse_structure_children,     /* STContinuationSolver */
  &stip_traverse_structure_children,     /* STContinuationWriter */
  &stip_traverse_structure_children,     /* STBattlePlaySolver */
  &stip_traverse_structure_children,     /* STBattlePlaySolutionWriter */
  &stip_traverse_structure_children,     /* STThreatSolver */
  &stip_traverse_structure_children,     /* STZugzwangWriter */
  &stip_traverse_structure_children,     /* STThreatEnforcer */
  &stip_traverse_structure_children,     /* STThreatCollector */
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
  &stip_traverse_structure_children,     /* STSelfCheckGuardAttackerFilter */
  &stip_traverse_structure_children,     /* STSelfCheckGuardDefenderFilter */
  &stip_traverse_structure_children,     /* STSelfCheckGuardHelpFilter */
  &stip_traverse_structure_children,     /* STSelfCheckGuardSeriesFilter */
  &stip_traverse_structure_children,     /* STDirectDefenderFilter */
  &stip_traverse_structure_children,     /* STReflexRootFilter */
  &stip_traverse_structure_children,     /* STReflexHelpFilter */
  &stip_traverse_structure_children,     /* STReflexSeriesFilter */
  &stip_traverse_structure_children,     /* STReflexAttackerFilter */
  &stip_traverse_structure_children,     /* STReflexDefenderFilter */
  &stip_traverse_structure_children,     /* STSelfDefense */
  &stip_traverse_structure_children,     /* STDefenseFork */
  &stip_traverse_structure_children,     /* STRestartGuardRootDefenderFilter */
  &stip_traverse_structure_children,     /* STRestartGuardHelpFilter */
  &stip_traverse_structure_children,     /* STRestartGuardSeriesFilter */
  &stip_traverse_structure_children,     /* STIntelligentHelpFilter */
  &stip_traverse_structure_children,     /* STIntelligentSeriesFilter */
  &stip_traverse_structure_children,     /* STGoalReachableGuardHelpFilter */
  &stip_traverse_structure_children,     /* STGoalReachableGuardSeriesFilter */
  &stip_traverse_structure_children,     /* STIntelligentDuplicateAvoider */
  &stip_traverse_structure_children,     /* STKeepMatingGuardAttackerFilter */
  &stip_traverse_structure_children,     /* STKeepMatingGuardDefenderFilter */
  &stip_traverse_structure_children,     /* STKeepMatingGuardHelpFilter */
  &stip_traverse_structure_children,     /* STKeepMatingGuardSeriesFilter */
  &stip_traverse_structure_children,     /* STMaxFlightsquares */
  &stip_traverse_structure_children,     /* STDegenerateTree */
  &stip_traverse_structure_children,     /* STMaxNrNonTrivial */
  &stip_traverse_structure_children,     /* STMaxNrNonChecks */
  &stip_traverse_structure_children,     /* STMaxNrNonTrivialCounter */
  &stip_traverse_structure_children,     /* STMaxThreatLength */
  &stip_traverse_structure_children,     /* STMaxTimeRootDefenderFilter */
  &stip_traverse_structure_children,     /* STMaxTimeDefenderFilter */
  &stip_traverse_structure_children,     /* STMaxTimeHelpFilter */
  &stip_traverse_structure_children,     /* STMaxTimeSeriesFilter */
  &stip_traverse_structure_children,     /* STMaxSolutionsRootSolvableFilter */
  &stip_traverse_structure_children,     /* STMaxSolutionsSolvableFilter */
  &stip_traverse_structure_children,     /* STMaxSolutionsRootDefenderFilter */
  &stip_traverse_structure_children,     /* STMaxSolutionsHelpFilter */
  &stip_traverse_structure_children,     /* STMaxSolutionsSeriesFilter */
  &stip_traverse_structure_children,     /* STStopOnShortSolutionsRootSolvableFilter */
  &stip_traverse_structure_children,     /* STStopOnShortSolutionsHelpFilter */
  &stip_traverse_structure_children,     /* STStopOnShortSolutionsSeriesFilter */
  &stip_traverse_structure_children,     /* STEndOfPhaseWriter */
  &stip_traverse_structure_children,     /* STEndOfSolutionWriter */
  &stip_traverse_structure_children,     /* STRefutationWriter */
  &stip_traverse_structure_children,     /* STOutputPlaintextTreeCheckDetectorAttackerFilter */
  &stip_traverse_structure_children,     /* STOutputPlaintextTreeCheckDetectorDefenderFilter */
  &stip_traverse_structure_children,     /* STOutputPlaintextLineLineWriter */
  &stip_traverse_structure_children,     /* STOutputPlaintextTreeGoalWriter */
  &stip_traverse_structure_children,     /* STOutputPlaintextTreeMoveInversionCounter */
  &stip_traverse_structure_children,     /* STOutputPlaintextLineMoveInversionCounter */
  &stip_traverse_structure_children      /* STOutputPlaintextLineEndOfIntroSeriesMarker */
};

/* Instrument stipulation with STDegenerateTree slices
 */
void stip_insert_degenerate_tree_guards(void)
{
  stip_structure_traversal st;

  TraceFunctionEntry(__func__);
  TraceFunctionParamListEnd();

  stip_structure_traversal_init(&st,&degenerate_tree_guards_inserters,0);
  stip_traverse_structure(root_slice,&st);

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}
