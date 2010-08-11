#include "pyslice.h"
#include "pydata.h"
#include "trace.h"
#include "stipulation/leaf.h"
#include "stipulation/battle_play/attack_play.h"
#include "stipulation/battle_play/attack_root.h"
#include "stipulation/battle_play/defense_play.h"
#include "stipulation/battle_play/defense_root.h"
#include "stipulation/help_play/play.h"
#include "stipulation/help_play/root.h"
#include "stipulation/series_play/play.h"
#include "stipulation/series_play/root.h"
#include "stipulation/goal_reached_tester.h"
#include "pybrafrk.h"
#include "pyquodli.h"
#include "pyrecipr.h"
#include "pynot.h"
#include "pymovein.h"
#include "pyhash.h"
#include "pyreflxg.h"
#include "pydirctg.h"
#include "pyselfgd.h"
#include "pyselfcg.h"
#include "pykeepmt.h"
#include "pypipe.h"
#include "optimisations/maxsolutions/root_solvable_filter.h"
#include "optimisations/maxsolutions/solvable_filter.h"
#include "optimisations/stoponshortsolutions/root_solvable_filter.h"
#include "optimisations/intelligent/duplicate_avoider.h"
#include "output/plaintext/end_of_phase_writer.h"
#include "output/plaintext/tree/goal_writer.h"
#include "output/plaintext/tree/move_inversion_counter.h"
#include "output/plaintext/line/end_of_intro_series_marker.h"
#include "output/plaintext/line/move_inversion_counter.h"
#include "output/plaintext/line/line_writer.h"

#include <assert.h>
#include <stdlib.h>


#define ENUMERATION_TYPENAME has_solution_type
#define ENUMERATORS                             \
  ENUMERATOR(opponent_self_check),              \
    ENUMERATOR(has_no_solution),                \
    ENUMERATOR(has_solution)

#define ENUMERATION_MAKESTRINGS

#include "pyenum.h"


/* Solve a slice
 * @param si slice index
 * @return whether there is a solution and (to some extent) why not
 */
has_solution_type slice_solve(slice_index si)
{
  has_solution_type result;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParamListEnd();

  TraceEnumerator(SliceType,slices[si].type,"\n");
  switch (slices[si].type)
  {
    case STLeaf:
      result = leaf_solve(si);
      break;

    case STGoalReachedTester:
      result = goal_reached_tester_solve(si);
      break;

    case STAttackRoot:
    case STDefenseDealtWith:
    case STAttackFork:
    case STAttackHashed:
    case STSelfDefense:
    case STSelfCheckGuardAttackerFilter:
    case STRefutingVariationWriter:
    case STReflexAttackerFilter:
    case STDegenerateTree:
    case STVariationWriter:
    case STOutputPlaintextTreeCheckDetectorAttackerFilter:
    case STDoubleMateAttackerFilter:
    case STEnPassantAttackerFilter:
    case STCastlingAttackerFilter:
    case STCounterMateAttackerFilter:
      result = attack_solve(si);
      break;

    case STDefenseRoot:
      result = defense_root_solve(si);
      break;

    case STSelfCheckGuardDefenderFilter:
    case STContinuationSolver:
    case STBattlePlaySolver:
    case STOutputPlaintextTreeCheckDetectorDefenderFilter:
    case STDefenseMove:
    case STReflexDefenderFilter:
    case STThreatSolver:
    case STEnPassantDefenderFilter:
      if (defense_can_defend(si))
        result = has_no_solution;
      else
      {
        boolean const defend_result = defense_defend(si);
        assert(!defend_result);
        result = has_solution;
      }
      break;

    case STHelpRoot:
      result = help_root_solve(si);
      break;

    case STHelpMove:
    case STHelpMoveToGoal:
    case STHelpFork:
    case STHelpHashed:
    case STSelfCheckGuardHelpFilter:
    case STStopOnShortSolutionsHelpFilter:
    case STCounterMateHelpFilter:
    case STDoubleMateHelpFilter:
    case STEnPassantHelpFilter:
    case STCastlingHelpFilter:
      result = help_solve(si);
      break;

    case STSeriesRoot:
      result = series_root_solve(si);
      break;

    case STSelfCheckGuardSeriesFilter:
    case STSeriesMove:
    case STSeriesMoveToGoal:
    case STSeriesFork:
    case STSeriesHashed:
    case STStopOnShortSolutionsSeriesFilter:
    case STDoubleMateSeriesFilter:
    case STCastlingSeriesFilter:
      result = series_solve(si);
      break;

    case STQuodlibet:
      result = quodlibet_solve(si);
      break;

    case STReciprocal:
      result = reci_solve(si);
      break;

    case STNot:
      result = not_solve(si);
      break;

    case STReflexRootFilter:
      result = reflex_root_filter_solve(si);
      break;

    case STMoveInverterRootSolvableFilter:
      result = move_inverter_root_solve(si);
      break;

    case STMoveInverterSolvableFilter:
      result = move_inverter_solve(si);
      break;

    case STSelfCheckGuardRootSolvableFilter:
      result = selfcheck_guard_root_solve(si);
      break;

    case STSelfCheckGuardSolvableFilter:
      result = selfcheck_guard_solve(si);
      break;

    case STMaxSolutionsRootSolvableFilter:
      result = maxsolutions_root_solvable_filter_solve(si);
      break;

    case STMaxSolutionsSolvableFilter:
      result = maxsolutions_solvable_filter_solve(si);
      break;

    case STStopOnShortSolutionsRootSolvableFilter:
      result = stoponshortsolutions_root_solvable_filter_solve(si);
      break;

    case STEndOfPhaseWriter:
      result = end_of_phase_writer_solve(si);
      break;

    case STOutputPlaintextLineLineWriter:
      result = line_writer_solve(si);
      break;

    case STIntelligentDuplicateAvoider:
      result = intelligent_duplicate_avoider_solve(si);
      break;

    case STOutputPlaintextTreeGoalWriter:
      result = goal_writer_solve(si);
      break;

    case STOutputPlaintextTreeMoveInversionCounter:
      result = output_plaintext_tree_move_inversion_counter_solve(si);
      break;

    case STOutputPlaintextLineMoveInversionCounter:
      result = output_plaintext_line_move_inversion_counter_solve(si);
      break;

    case STOutputPlaintextLineEndOfIntroSeriesMarker:
      result = output_plaintext_line_end_of_intro_series_marker_solve(si);
      break;

    default:
      assert(0);
      result = has_no_solution;
      break;
  }

  TraceFunctionExit(__func__);
  TraceEnumerator(has_solution_type,result,"");
  TraceFunctionResultEnd();
  return result;
}

/* Determine whether a slice has a solution
 * @param si slice index
 * @return whether there is a solution and (to some extent) why not
 */
has_solution_type slice_has_solution(slice_index si)
{
  has_solution_type result = has_no_solution;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParamListEnd();

  TraceEnumerator(SliceType,slices[si].type,"\n");
  switch (slices[si].type)
  {
    case STLeaf:
      result = leaf_has_solution(si);
      break;

    case STGoalReachedTester:
      result = goal_reached_tester_has_solution(si);
      break;

    case STQuodlibet:
      result = quodlibet_has_solution(si);
      break;

    case STReciprocal:
      result = reci_has_solution(si);
      break;

    case STNot:
      result = not_has_solution(si);
      break;

    case STAttackRoot:
    case STAttackFork:
    case STDefenseDealtWith:
    case STSelfDefense:
    case STKeepMatingGuardAttackerFilter:
    case STKeepMatingGuardDefenderFilter:
    case STKeepMatingGuardHelpFilter:
    case STKeepMatingGuardSeriesFilter:
    case STDegenerateTree:
    case STAttackHashed:
    case STVariationWriter:
    case STOutputPlaintextTreeCheckDetectorAttackerFilter:
    case STDoubleMateAttackerFilter:
    case STEnPassantAttackerFilter:
    case STCastlingAttackerFilter:
    case STCounterMateAttackerFilter:
      result = attack_has_solution(si);
      break;

    case STContinuationSolver:
    case STBattlePlaySolver:
    case STDefenseMove:
    case STSelfCheckGuardDefenderFilter:
    case STEnPassantDefenderFilter:
      result = defense_can_defend(si) ? has_no_solution : has_solution;
      break;

    case STHelpRoot:
    case STHelpMove:
    case STHelpMoveToGoal:
    case STSelfCheckGuardHelpFilter:
    case STHelpHashed:
    case STCounterMateHelpFilter:
    case STDoubleMateHelpFilter:
    case STEnPassantHelpFilter:
    case STCastlingHelpFilter:
      result = help_has_solution(si);
      break;

    case STMoveInverterSolvableFilter:
      result = move_inverter_has_solution(si);
      break;

    case STSeriesMove:
    case STSeriesMoveToGoal:
    case STSeriesHashed:
    case STReflexSeriesFilter:
    case STDoubleMateSeriesFilter:
    case STCastlingSeriesFilter:
      result = series_has_solution(si);
      break;

    case STHelpFork:
    case STSeriesFork:
      result = branch_fork_has_solution(si);
      break;

    case STSelfCheckGuardRootSolvableFilter:
    case STSelfCheckGuardSolvableFilter:
      result = selfcheck_guard_has_solution(si);
      break;

    case STOutputPlaintextLineLineWriter:
      result = line_writer_has_solution(si);
      break;

    case STIntelligentDuplicateAvoider:
      result = intelligent_duplicate_avoider_has_solution(si);
      break;

    case STMaxSolutionsSolvableFilter:
      result = maxsolutions_solvable_filter_has_solution(si);
      break;

    case STOutputPlaintextTreeGoalWriter:
      result = goal_writer_has_solution(si);
      break;

    case STOutputPlaintextTreeMoveInversionCounter:
      result = output_plaintext_tree_move_inversion_counter_has_solution(si);
      break;

    case STOutputPlaintextLineMoveInversionCounter:
      result = output_plaintext_line_move_inversion_counter_has_solution(si);
      break;

    case STEndOfPhaseWriter:
      result = end_of_phase_writer_has_solution(si);
      break;

    case STOutputPlaintextLineEndOfIntroSeriesMarker:
      result = output_plaintext_line_end_of_intro_series_marker_has_solution(si);
      break;

    default:
      assert(0);
      break;
  }

  TraceFunctionExit(__func__);
  TraceEnumerator(has_solution_type,result,"");
  TraceFunctionResultEnd();
  return result;
}
