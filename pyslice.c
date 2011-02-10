#include "pyslice.h"
#include "pydata.h"
#include "trace.h"
#include "stipulation/leaf.h"
#include "stipulation/setplay_fork.h"
#include "stipulation/reflex_attack_solver.h"
#include "stipulation/battle_play/attack_adapter.h"
#include "stipulation/battle_play/defense_adapter.h"
#include "stipulation/battle_play/ready_for_defense.h"
#include "stipulation/help_play/play.h"
#include "stipulation/help_play/root.h"
#include "stipulation/series_play/play.h"
#include "stipulation/series_play/root.h"
#include "stipulation/goals/target/reached_tester.h"
#include "stipulation/goals/check/reached_tester.h"
#include "stipulation/goals/capture/reached_tester.h"
#include "stipulation/goals/steingewinn/reached_tester.h"
#include "stipulation/goals/enpassant/reached_tester.h"
#include "stipulation/goals/doublemate/reached_tester.h"
#include "stipulation/goals/castling/reached_tester.h"
#include "stipulation/goals/circuit/reached_tester.h"
#include "stipulation/goals/exchange/reached_tester.h"
#include "stipulation/goals/circuit_by_rebirth/reached_tester.h"
#include "stipulation/goals/exchange_by_rebirth/reached_tester.h"
#include "stipulation/goals/proofgame/reached_tester.h"
#include "stipulation/goals/immobile/reached_tester.h"
#include "stipulation/goals/notcheck/reached_tester.h"
#include "stipulation/goals/any/reached_tester.h"
#include "pybrafrk.h"
#include "pyquodli.h"
#include "pyrecipr.h"
#include "pynot.h"
#include "pymovein.h"
#include "pyhash.h"
#include "pyreflxg.h"
#include "pyselfgd.h"
#include "pyselfcg.h"
#include "pykeepmt.h"
#include "pypipe.h"
#include "pieces/attributes/paralysing/mate_filter.h"
#include "pieces/attributes/paralysing/stalemate_special.h"
#include "conditions/amu/mate_filter.h"
#include "conditions/circe/steingewinn_filter.h"
#include "conditions/circe/circuit_by_rebirth_special.h"
#include "conditions/circe/exchange_by_rebirth_special.h"
#include "conditions/anticirce/target_square_filter.h"
#include "conditions/anticirce/circuit_special.h"
#include "conditions/anticirce/exchange_special.h"
#include "conditions/anticirce/exchange_filter.h"
#include "conditions/ultraschachzwang/goal_filter.h"
#include "optimisations/maxsolutions/root_solvable_filter.h"
#include "optimisations/maxsolutions/solvable_filter.h"
#include "optimisations/stoponshortsolutions/initialiser.h"
#include "optimisations/intelligent/duplicate_avoider.h"
#include "output/plaintext/illegal_selfcheck_writer.h"
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

    case STSetplayFork:
      result = setplay_fork_solve(si);
      break;

    case STGoalTargetReachedTester:
      result = goal_target_reached_tester_solve(si);
      break;

    case STGoalCheckReachedTester:
      result = goal_check_reached_tester_solve(si);
      break;

    case STGoalCaptureReachedTester:
      result = goal_capture_reached_tester_solve(si);
      break;

    case STGoalSteingewinnReachedTester:
      result = goal_steingewinn_reached_tester_solve(si);
      break;

    case STGoalEnpassantReachedTester:
      result = goal_enpassant_reached_tester_solve(si);
      break;

    case STGoalDoubleMateReachedTester:
    case STGoalCounterMateReachedTester:
      result = goal_doublemate_reached_tester_solve(si);
      break;

    case STGoalCastlingReachedTester:
      result = goal_castling_reached_tester_solve(si);
      break;

    case STGoalCircuitReachedTester:
      result = goal_circuit_reached_tester_solve(si);
      break;

    case STGoalExchangeReachedTester:
      result = goal_exchange_reached_tester_solve(si);
      break;

    case STGoalCircuitByRebirthReachedTester:
      result = goal_circuit_by_rebirth_reached_tester_solve(si);
      break;

    case STGoalExchangeByRebirthReachedTester:
      result = goal_exchange_by_rebirth_reached_tester_solve(si);
      break;

    case STGoalProofgameReachedTester:
    case STGoalAToBReachedTester:
      result = goal_proofgame_reached_tester_solve(si);
      break;

    case STGoalImmobileReachedTester:
      result = goal_immobile_reached_tester_solve(si);
      break;

    case STGoalNotCheckReachedTester:
      result = goal_notcheck_reached_tester_solve(si);
      break;

    case STGoalAnyReachedTester:
      result = goal_any_reached_tester_solve(si);
      break;

    case STAttackAdapter:
      result = attack_adapter_solve(si);
      break;

    case STDefenseAdapter:
      result = defense_adapter_solve(si);
      break;

    case STHelpRoot:
      result = help_root_solve(si);
      break;

    case STHelpMove:
    case STHelpMoveToGoal:
    case STHelpFork:
    case STHelpHashed:
    case STEnPassantHelpFilter:
    case STCastlingHelpFilter:
      result = help_solve(si);
      break;

    case STSeriesRoot:
      result = series_root_solve(si);
      break;

    case STSeriesMove:
    case STSeriesMoveToGoal:
    case STSeriesFork:
    case STSeriesHashed:
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

    case STStipulationReflexAttackSolver:
      result = reflex_attack_solver_solve(si);
      break;

    case STMoveInverter:
      result = move_inverter_solve(si);
      break;

    case STSelfCheckGuard:
      result = selfcheck_guard_solve(si);
      break;

    case STMaxSolutionsRootSolvableFilter:
      result = maxsolutions_root_solvable_filter_solve(si);
      break;

    case STMaxSolutionsSolvableFilter:
      result = maxsolutions_solvable_filter_solve(si);
      break;

    case STStopOnShortSolutionsInitialiser:
      result = stoponshortsolutions_initialiser_solve(si);
      break;

    case STIllegalSelfcheckWriter:
      result = illegal_selfcheck_writer_solve(si);
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

    case STPiecesParalysingMateFilter:
      result = paralysing_mate_filter_solve(si);
      break;

    case STPiecesParalysingStalemateSpecial:
      result = paralysing_stalemate_special_solve(si);
      break;

    case STAmuMateFilter:
      result = amu_mate_filter_solve(si);
      break;

    case STUltraschachzwangGoalFilter:
      result = ultraschachzwang_goal_filter_solve(si);
      break;

    case STCirceSteingewinnFilter:
      result = circe_steingewinn_filter_solve(si);
      break;

    case STCirceCircuitSpecial:
      result = circe_circuit_special_solve(si);
      break;

    case STCirceExchangeSpecial:
      result = circe_exchange_special_solve(si);
      break;

    case STAnticirceTargetSquareFilter:
      result = anticirce_target_square_filter_solve(si);
      break;

    case STAnticirceCircuitSpecial:
      result = anticirce_circuit_special_solve(si);
      break;

    case STAnticirceExchangeSpecial:
      result = anticirce_exchange_special_solve(si);
      break;

    case STAnticirceExchangeFilter:
      result = anticirce_exchange_filter_solve(si);
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

    case STSetplayFork:
      result = setplay_fork_has_solution(si);
      break;

    case STGoalTargetReachedTester:
      result = goal_target_reached_tester_has_solution(si);
      break;

    case STGoalCheckReachedTester:
      result = goal_check_reached_tester_has_solution(si);
      break;

    case STGoalCaptureReachedTester:
      result = goal_capture_reached_tester_has_solution(si);
      break;

    case STGoalSteingewinnReachedTester:
      result = goal_steingewinn_reached_tester_has_solution(si);
      break;

    case STGoalEnpassantReachedTester:
      result = goal_enpassant_reached_tester_has_solution(si);
      break;

    case STGoalDoubleMateReachedTester:
    case STGoalCounterMateReachedTester:
      result = goal_doublemate_reached_tester_has_solution(si);
      break;

    case STGoalCastlingReachedTester:
      result = goal_castling_reached_tester_has_solution(si);
      break;

    case STGoalCircuitReachedTester:
      result = goal_circuit_reached_tester_has_solution(si);
      break;

    case STGoalExchangeReachedTester:
      result = goal_exchange_reached_tester_has_solution(si);
      break;

    case STGoalCircuitByRebirthReachedTester:
      result = goal_circuit_by_rebirth_reached_tester_has_solution(si);
      break;

    case STGoalExchangeByRebirthReachedTester:
      result = goal_exchange_by_rebirth_reached_tester_has_solution(si);
      break;

    case STGoalProofgameReachedTester:
    case STGoalAToBReachedTester:
      result = goal_proofgame_reached_tester_has_solution(si);
      break;

    case STGoalImmobileReachedTester:
      result = goal_immobile_reached_tester_has_solution(si);
      break;

    case STGoalNotCheckReachedTester:
      result = goal_notcheck_reached_tester_has_solution(si);
      break;

    case STGoalAnyReachedTester:
      result = goal_any_reached_tester_has_solution(si);
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

    case STStipulationReflexAttackSolver:
      result = reflex_attack_solver_has_solution(si);
      break;

    case STAttackAdapter:
      result = attack_adapter_has_solution(si);
      break;

    case STDefenseAdapter:
      result = defense_adapter_has_solution(si);
      break;

    case STHelpRoot:
    case STHelpMove:
    case STHelpMoveToGoal:
    case STHelpHashed:
    case STEnPassantHelpFilter:
    case STCastlingHelpFilter:
      result = help_has_solution(si);
      break;

    case STMoveInverter:
      result = move_inverter_has_solution(si);
      break;

    case STSeriesMove:
    case STSeriesMoveToGoal:
    case STSeriesHashed:
    case STReflexSeriesFilter:
    case STCastlingSeriesFilter:
      result = series_has_solution(si);
      break;

    case STHelpFork:
    case STSeriesFork:
      result = branch_fork_has_solution(si);
      break;

    case STSelfCheckGuard:
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

    case STIllegalSelfcheckWriter:
      result = illegal_selfcheck_writer_has_solution(si);
      break;

    case STEndOfPhaseWriter:
      result = end_of_phase_writer_has_solution(si);
      break;

    case STOutputPlaintextLineEndOfIntroSeriesMarker:
      result = output_plaintext_line_end_of_intro_series_marker_has_solution(si);
      break;

    case STPiecesParalysingMateFilter:
      result = paralysing_mate_filter_has_solution(si);
      break;

    case STPiecesParalysingStalemateSpecial:
      result = paralysing_stalemate_special_has_solution(si);
      break;

    case STAmuMateFilter:
      result = amu_mate_filter_has_solution(si);
      break;

    case STUltraschachzwangGoalFilter:
      result = ultraschachzwang_goal_filter_has_solution(si);
      break;

    case STCirceSteingewinnFilter:
      result = circe_steingewinn_filter_has_solution(si);
      break;

    case STCirceCircuitSpecial:
      result = circe_circuit_special_has_solution(si);
      break;

    case STCirceExchangeSpecial:
      result = circe_exchange_special_has_solution(si);
      break;

    case STAnticirceTargetSquareFilter:
      result = anticirce_target_square_filter_has_solution(si);
      break;

    case STAnticirceCircuitSpecial:
      result = anticirce_circuit_special_has_solution(si);
      break;

    case STAnticirceExchangeSpecial:
      result = anticirce_exchange_special_has_solution(si);
      break;

    case STAnticirceExchangeFilter:
      result = anticirce_exchange_filter_has_solution(si);
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
